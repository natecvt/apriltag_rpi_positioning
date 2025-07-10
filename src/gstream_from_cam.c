#include <gstream_from_cam.h>

int setup(CustomData *cd, u_int8_t es, u_int8_t sy, u_int16_t width, u_int16_t height, u_int8_t fr) {

    // create the pipeline and elements to add to it
    cd->pipeline = gst_pipeline_new("pipeline");

    cd->source = gst_element_factory_make("libcamerasrc", "source");
    cd->convert = gst_element_factory_make("videoconvert", "convert");
    cd->caps1 = gst_element_factory_make("capsfilter", "caps1");
    cd->sink = gst_element_factory_make("appsink", "sink");

    // add properties to data structure
    cd->iw = width;
    cd->ih = height;
    cd->fr = fr;
    cd->np = width * height;
    cd->stride = 2;

    // check if things were created correctly
    if (!cd->pipeline || !cd->source || !cd->caps1 || !cd->convert || !cd->sink) {
        g_printerr("Not all elements could be created.\n");
        return 1;
    }

    // set data to objects, customize filters
    g_object_set(cd->sink,
        "emit-signals", es, 
        "sync", sy, 
        NULL);
    
    // caps filter for setting the output image parameters 
    // #TODO: add format options
    GstCaps *caps = gst_caps_new_simple(
        "video/x-raw",
        "format", G_TYPE_STRING, "GRAY16_LE",
        "width", G_TYPE_INT, cd->iw,
        "height", G_TYPE_INT, cd->ih,
        "framerate", GST_TYPE_FRACTION, cd->fr, 1,
        NULL);

    g_object_set(cd->caps1, "caps", caps, NULL);
    gst_caps_unref(caps); // have to unref objects

    // add and link everything to the pipeline
    gst_bin_add_many(GST_BIN (cd->pipeline), cd->source, cd->convert, cd->caps1, cd->sink, NULL);
    if (!gst_element_link_many(cd->source, cd->convert, cd->caps1, cd->sink, NULL)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(cd->pipeline);
        return 2;
    }

    // set the state to playing once everything is properly set up
    if (cd->pipeline->current_state != GST_STATE_PLAYING) gst_element_set_state(cd->pipeline, GST_STATE_PLAYING);

    return 0;
}

int print_bus_message(GstBus *bus, CustomData *cd) {
    // get newest message
    GstMessage *msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    // parse message, given it exists
    if (msg != NULL) {
        GError *err;
        gchar *debug_info;

        // cases based on message type, 64 case is broken for some reason
        int errtype = GST_MESSAGE_TYPE (msg);

        switch (errtype) {
            case GST_MESSAGE_EOS:
                // for end of stream, should never reach here unless reading from a file or premade stream
                g_print ("End-Of-Stream reached.\n");
                return 1;
            case GST_MESSAGE_ERROR:
                // for errors
                gst_message_parse_error (msg, &err, &debug_info);
                g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
                g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error (&err);
                g_free (debug_info);
                return 2;
            case GST_MESSAGE_STATE_CHANGED:
                // reading state changes from the pipeline
                if (GST_MESSAGE_SRC (msg) == GST_OBJECT (cd->pipeline)) {
                    GstState old_state, new_state, pending_state;
                    gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
                    g_print ("Pipeline state changed from %s to %s:\n",
                        gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
                }
                break;
            default:
                // default, should never happen
                g_printerr ("Unexpected message received of type: %d\n", errtype);
                return 3; // uncomment when this never happens anymore
        }

        // unref the message object
        gst_message_unref (msg);
    }

    return 0;
}

int cleanup(GstBus *bus, CustomData *cd) {
    // unrefs objects passed by reference

    if (bus != NULL) {
        gst_object_unref(bus);
    }
    if (cd != NULL) {
        gst_element_set_state(cd->pipeline, GST_STATE_NULL);
        gst_object_unref(cd->pipeline);
    }
    return 0;
}

int pull_sample(CustomData *cd, u_int16_t data[]) {
    // pull the sample
    GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(cd->sink));
    
    // don't continue if sample is not found
    if (!sample) return 1;

    // load a buffer and a map for sample
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;

    // if the buffer can be read, copy the map data to an external array
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        // printf("Frame: %d bytes\n", map.size);
        memcpy(data, map.data, map.size);

        gst_buffer_unmap(buffer, &map);
    }

    gst_sample_unref(sample);

    return 0;
}

int main(int argc, char *argv[]) {
    // initialization, #TODO add method to read debug mode
    setenv("GST_DEBUG", "3", 1);
    gst_init(&argc, &argv);

    // declaring the data used for the rest of this file
    CustomData cd;
    u_int8_t ec;

    // perform setup, check error output
    // #TODO add method to read in these inputs from config
    // #TODO add way to modulate framerate based on max framerate per image size
    ec = setup(&cd, FALSE, FALSE, 640, 480, 30);
    if (ec) {
        g_printerr("Setup returned error code: %d", ec);
        exit(1);
    }

    // set up bus
    GstBus *bus = gst_element_get_bus(cd.pipeline);

    // #TODO: add a way to account for different formats
    u_int16_t *data;

    data = (u_int16_t*)malloc(cd.np * cd.stride);
    if (data == NULL) {
        perror("Image data allocation failed");
        exit(2);
    }

    // pulling sample from camera and print bus error message, the only functions used in a loop
    ec = pull_sample(&cd, data);
    if (ec) {
        g_printerr("Sampling returned error code: %d", ec);
        // do not exit, run something to fix the break in timing
    }

    ec = print_bus_message(bus, &cd);
    if (ec) {
        g_printerr("Bus returned error code: %d", ec);
        exit(3);
    }

    // write image to file, for debugging
    // FILE *f;
    // f = fopen("output/img.csv", "w");
    // for (int j = cd.np - 1; j > 0; j--) {
    //     fprintf(f, "%d,", data[j]);
    // }

    ec = cleanup(bus, &cd);
    if (ec) {
        g_printerr("Cleanup returned error code: %d", ec);
        exit(4);
    }
    
    // free dynamically allocated image data array
    free(data);

    exit(0);
}
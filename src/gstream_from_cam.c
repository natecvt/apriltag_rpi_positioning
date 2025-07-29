#include <gstream_from_cam.h>

int gstream_setup(StreamSet *ss, Settings *settings, uint8_t emit_signals, uint8_t sync) {

    // create the pipeline and elements to add to it
    ss->pipeline = gst_pipeline_new("pipeline");

    ss->source = gst_element_factory_make("libcamerasrc", "source");
    ss->caps = gst_element_factory_make("capsfilter", "caps");
    ss->queue = gst_element_factory_make("queue", "queue");
    ss->convert = gst_element_factory_make("videoconvert", "convert");
    ss->scale = gst_element_factory_make("videoscale", "scale");
    ss->sink = gst_element_factory_make("appsink", "sink");

    // check if things were created correctly
    if (!ss->pipeline || !ss->queue || !ss->source || !ss->caps || !ss->convert || !ss->scale || !ss->sink) {
        g_printerr("Not all elements could be created.\n");
        return 1;
    }

    // set data to objects, customize filters
    g_object_set(ss->sink,
        "emit-signals", emit_signals, 
        "sync", sync, 
        NULL);
    
    // caps filter for setting the output image parameters 
    // #TODO: add format options
    GstCaps *capssrc = gst_caps_new_simple(
        "video/x-raw",
        "format", G_TYPE_STRING, "BGRx",
        "width", G_TYPE_INT, 1920,
        "height", G_TYPE_INT, 1080,
        NULL);

    GstCaps *capssink = gst_caps_new_simple(
        "video/x-raw",
        "format", G_TYPE_STRING, "GRAY8",
        "width", G_TYPE_INT, settings->width,
        "height", G_TYPE_INT, settings->height,
        "framerate", GST_TYPE_FRACTION, settings->framerate, 1,
        NULL);

    g_object_set(G_OBJECT(ss->caps), "caps", capssrc, NULL);
    gst_app_sink_set_caps(GST_APP_SINK(ss->sink), capssink);
    gst_caps_unref(capssrc);
    gst_caps_unref(capssink); // have to unref objects

    // add and link everything to the pipeline
    gst_bin_add_many(GST_BIN (ss->pipeline), ss->source, ss->caps, ss->queue, ss->convert, ss->scale, ss->sink, NULL);
    if (!gst_element_link_many(ss->source, ss->caps, ss->queue, ss->convert, ss->scale, ss->sink, NULL)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(ss->pipeline);
        return 2;
    }

    // set the state to playing once everything is properly set up
    if (ss->pipeline->current_state != GST_STATE_PLAYING) gst_element_set_state(ss->pipeline, GST_STATE_PLAYING);

    return 0;
}

int gstream_pull_sample(StreamSet *ss, uint8_t *data, Settings *settings) {
    // pull the sample
    GstSample *sample = gst_app_sink_try_pull_sample(GST_APP_SINK(ss->sink), GST_SECOND / settings->framerate);
    
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

// int print_bus_message(GstBus *bus, StreamSet *ss) {
//     // get newest message
//     GstMessage *msg = gst_bus_timed_pop_filtered (bus, GST_SECOND / 10,
//         GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

//     // parse message, given it exists
//     if (msg != NULL) {
//         GError *err;
//         gchar *debug_info;

//         // cases based on message type, 64 case is broken for some reason
//         int errtype = GST_MESSAGE_TYPE (msg);

//         switch (errtype) {
//             case GST_MESSAGE_EOS:
//                 // for end of stream, should never reach here unless reading from a file or premade stream
//                 g_print ("End-Of-Stream reached.\n");
//                 return 1;
//             case GST_MESSAGE_ERROR:
//                 // for errors
//                 gst_message_parse_error (msg, &err, &debug_info);
//                 g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
//                 g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
//                 g_clear_error (&err);
//                 g_free (debug_info);
//                 return 2;
//             case GST_MESSAGE_STATE_CHANGED:
//                 // reading state changes from the pipeline
//                 if (GST_MESSAGE_SRC (msg) == GST_OBJECT (ss->pipeline)) {
//                     GstState old_state, new_state, pending_state;
//                     gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
//                     g_print ("Pipeline state changed from %s to %s:\n",
//                         gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
//                 }
//                 break;
//             default:
//                 // default, should never happen
//                 g_printerr ("Unexpected message received of type: %d\n", errtype);
//                 return 3; // uncomment when this never happens anymore
//         }

//         // unref the message object
//         gst_message_unref (msg);
//     }

//     return 0;
// }

int gstream_cleanup(GstBus *bus, StreamSet *ss) {
    // unrefs objects passed by reference

    if (bus != NULL) {
        gst_object_unref(bus);
    }
    if (ss != NULL) {
        gst_element_set_state(ss->pipeline, GST_STATE_NULL);
        gst_object_unref(ss->pipeline);
    }
    return 0;
}

#include <gstream_from_cam.h>

typedef struct _CustomData {
    GstElement *pipeline;
    GstElement *source;
    GstElement *caps1;
    GstElement *convert;
    //GstElement *caps2;
    GstElement *sink;
} CustomData;

// ,width=640,height=480,format=NV12,framerate=30/1
// ,format=GRAY16_LE

int setup(CustomData *cd, u_int8_t es, u_int8_t sy, u_int16_t width, u_int16_t height, u_int8_t fr) {
    // cd.source = gst_element_factory_make("libcamerasrc", "source");
    // cd.convert = gst_element_factory_make("videoconvert", "convert");
    // cd.caps1 = gst_element_factory_make("capsfilter", "caps1");
    // cd.caps2 = gst_element_factory_make("capsfilter", "caps2");
    // cd.sink = gst_element_factory_make("appsink", "sink");

    cd->pipeline = gst_pipeline_new("test-pipeline");

    cd->source = gst_element_factory_make("libcamerasrc", "source");
    cd->convert = gst_element_factory_make("videoconvert", "convert");
    cd->caps1 = gst_element_factory_make("capsfilter", "caps1");
    cd->sink = gst_element_factory_make("appsink", "sink");

    if (!cd->pipeline || !cd->source || !cd->caps1 || !cd->convert || !cd->sink) {
        g_printerr("Not all elements could be created.\n");
        return 1;
    }

    g_object_set(cd->sink,
        "emit-signals", es, 
        "sync", sy, 
        NULL);

    GstCaps *caps = gst_caps_new_simple(
        "video/x-raw",
        "format", G_TYPE_STRING, "GRAY16_LE",
        "width", G_TYPE_INT, width,
        "height", G_TYPE_INT, height,
        "framerate", GST_TYPE_FRACTION, fr, 1,
        NULL);

    g_object_set(cd->caps1, "caps", caps, NULL);
    gst_caps_unref(caps);

    gst_bin_add_many(GST_BIN (cd->pipeline), cd->source, cd->convert, cd->caps1, cd->sink, NULL);
    if (!gst_element_link_many(cd->source, cd->convert, cd->caps1, cd->sink, NULL)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(cd->pipeline);
        return 2;
    }

    return 0;
}

int print_bus(GstBus *bus, CustomData cd) {
    GstMessage *msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    /* Parse message */
    if (msg != NULL) {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE (msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error (msg, &err, &debug_info);
                g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
                g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error (&err);
                g_free (debug_info);
            case GST_MESSAGE_EOS:
                g_print ("End-Of-Stream reached.\n");
            case GST_MESSAGE_STATE_CHANGED:
                /* We are only interested in state-changed messages from the pipeline */
                if (GST_MESSAGE_SRC (msg) == GST_OBJECT (cd.pipeline)) {
                    GstState old_state, new_state, pending_state;
                    gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
                    g_print ("Pipeline state changed from %s to %s:\n",
                        gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
                }
            default:
            /* We should not reach here */
            g_printerr ("Unexpected message received.\n");
        }
      gst_message_unref (msg);
    }
}

int cleanup(GstBus *bus, CustomData *cd) {
    gst_object_unref(bus);
    gst_element_set_state(cd->pipeline, GST_STATE_NULL);
    gst_object_unref(cd->pipeline);
    return 0;
}

int main(int argc, char *argv[]) {
    setenv("GST_DEBUG", "3", 1);

    gst_init(&argc, &argv);

    CustomData cd;

    // #TODO add method to read in these inputs from config
    if (setup(&cd, FALSE, FALSE, 640, 480, 100)) {
        return 0;
    }

    gst_element_set_state(cd.pipeline, GST_STATE_PLAYING);

    for (int i = 0; i < 1; i++) {

        GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(cd.sink));

        if (!sample) break;

        GstBuffer *buffer = gst_sample_get_buffer(sample);
        GstMapInfo map;

        if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            printf("Frame: %d bytes ", map.size);
            FILE *f;
            f = fopen("output/img.csv", "w");

            for (int j = 2 * 640 * 480 - 2; j > 0; j = j - 2) {
                int data = map.data[j] + (map.data[j+1] << 8);

                fprintf(f, "%d,", data);
            }

            gst_buffer_unmap(buffer, &map);
        }

        gst_sample_unref(sample);
    }

    gst_element_set_state(cd.pipeline, GST_STATE_NULL);
    gst_object_unref(cd.pipeline);

    return 0;
}
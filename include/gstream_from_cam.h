#ifndef GSTREAM_FROM_CAM_H
#define GSTREAM_FROM_CAM_H

#include <settings.h>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

// a type to contain all relevant gstreamer and image parameters
typedef struct _StreamSet {
    GstElement *pipeline;
    GstElement *source;
    GstElement *convert;
    GstElement *caps1;
    GstElement *sink;
} StreamSet;

// function declarations, #TODO: document these
int gstream_setup(StreamSet *cd, Settings *settings, uint8_t emit_signals, uint8_t sync);

int gstream_pull_sample(StreamSet *ss, u_int16_t data[]);

int print_bus_message(GstBus *bus, StreamSet *ss);

int gstream_cleanup(GstBus *bus, StreamSet *ss);

#endif
#ifndef GSTREAM_FROM_CAM_H
#define GSTREAM_FROM_CAM_H

#include <settings.h>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <stdint.h>

// a type to contain all relevant gstreamer and image parameters
typedef struct _StreamSet {
    GstElement *pipeline;
    GstElement *source;
    GstElement *caps;
    GstElement *queue;
    GstElement *convert;
    GstElement *scale;
    GstElement *sink;
} StreamSet;

// function declarations, #TODO: document these
int gstream_setup(StreamSet *cd, Settings *settings, uint8_t emit_signals, uint8_t sync);

int gstream_pull_sample(StreamSet *ss, uint8_t *data, Settings *settings);

int print_bus_message(GstBus *bus, StreamSet *ss);

int gstream_cleanup(GstBus *bus, StreamSet *ss);

#endif
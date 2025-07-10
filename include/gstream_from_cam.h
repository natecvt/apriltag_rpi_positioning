#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <stdio.h>

// a type to contain all relevant gstreamer and image parameters
typedef struct _CustomData {
    GstElement *pipeline;
    GstElement *source;
    GstElement *convert;
    GstElement *caps1;
    GstElement *sink;

    // #TODO: implement aspect ratio control, should always be 16x9
    u_int16_t iw; // output image width
    u_int16_t ih; // output image height
    u_int8_t fr; // capture framerate
    u_int32_t np; // number of pixels in output image
    u_int8_t stride; // number of bytes per pixel, 1 or 2 for grayscale
} CustomData;
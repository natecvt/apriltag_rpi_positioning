#include <json-c/json.h>
// #TODO: find specific .h files to include, not one for everything
#include <stdio.h>

#define PATH "settings/"

typedef struct _Settings {
    // images
    uint16_t iw; // output image width
    uint16_t ih; // output image height
    uint8_t ar[2]; // aspect ratio of output image, make [16, 9] to minimize image clipping, make [1, 1] for square image
    uint8_t fr; // capture framerate
    uint32_t np; // number of pixels in output image
    uint8_t stride; // number of bytes per pixel, 1 or 2 for grayscale

    // apriltags
    uint8_t quiet; // quiet debugging
    uint8_t iters; // number of iterations to run on detection

    uint8_t hamming; // number of bit errors per detection
    uint8_t threads; // number of threads to use, make 1 usually
    float dec; // decimation factor on images, make 1.5, 2.0, 3.0, 4.0, etc.
    float blur; // blurring factor, 0.0 does nothing, >0.0 blurs, <0.0 sharpens
    uint8_t refine; // boolean for if refining

    uint8_t tagfam; // tag family, refer to tagTypes enum in detect_apriltags.h
} Settings;
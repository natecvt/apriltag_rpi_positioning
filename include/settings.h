#ifndef SETTINGS_H
#define SETTINGS_H

#include <json-c/json.h>
// #TODO: find specific .h files to include, not one for everything
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// #TODO: implement auto-pathing, when path not given
#define PATH "settings/"
#define PLEN 100
#define FLEN 100

typedef struct _Settings {
    // images
    uint16_t width; // output image width
    uint16_t height; // output image height
    uint8_t is_height_from_ar; // whether height is computed from AR or just given
    uint8_t aspectratio[2]; // aspect ratio of output image, make [16, 9] to minimize image clipping, make [1, 1] for square image
    uint8_t framerate; // capture framerate
    uint32_t np; // number of pixels in output image
    uint8_t stride; // number of bytes per pixel, 1 or 2 for grayscale

    // apriltags
    uint8_t debug; // do debugging

    uint8_t quiet; // quiet debugging
    uint8_t iterations; // number of iterations to run on detection

    int hamming; // number of bit errors per detection
    uint8_t threads; // number of threads to use, make 1 usually
    float dec; // decimation factor on images, make 1.5, 2.0, 3.0, 4.0, etc.
    float blur; // blurring factor, 0.0 does nothing, >0.0 blurs, <0.0 sharpens
    uint8_t refine; // boolean for if refining

    uint8_t tag_family; // tag family, refer to tagTypes enum

    // calibration
    uint8_t use_preset_camera_calibration; // whether to use .cal file (false) or .json file (true)
    char* cal_file_path; // the path to the calibration file
    float fx, fy, cx, cy; // each of the intrinsic camera matrix coefficients, in pixels

    char* output_directory; // the folder where debug output will be created
} Settings;

enum tagTypes {
    TAG36H10 = 0,
    TAG36H11 = 1,
    TAG25H9 = 2,
    TAG16H5 = 3,
    TAG27H7R = 4,
    TAG49H12R = 5,
    TAG41H12S = 6,
    TAG52H13S = 7,
    TAG48H12C = 8
};

int load_settings_from_path(const char* path, Settings *settings);

#endif
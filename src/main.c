#include <settings.h>
#include <gstream_from_cam.h>
#include <detect_apriltags.h>
//#include <transmit_pose.h>

int main(int argc, char *argv[]) {
    setenv("GST_DEBUG", "3", 1);
    gst_init(&argc, &argv);

    Settings settings;
    StreamSet streams;
    int ec;
    GstBus *bus;
    GstState *state1, *state2;
    uint8_t *data;

    zarray_t *det;
    apriltag_detector_t *td;
    apriltag_family_t *tf;

    // read in settings from json file, #TODO: make the path an arg (using stropts?)
    ec = load_settings_from_path("/home/natec/apriltag_rpi_positioning/settings/settings.json", &settings);
    if(ec) {
        printf("Settings failed to load with error code: %d\n", ec);
        exit(1);
    }
    settings.np = settings.width * settings.height;

    // perform setup, check error output
    ec = gstream_setup(&streams, &settings, FALSE, FALSE);
    if (ec) {
        g_printerr("Gstream setup returned error code: %d\n", ec);
        exit(1);
    }

    bus = gst_element_get_bus(streams.pipeline);

    // allocating data 
    data = (uint8_t *)malloc(settings.np * settings.stride);
    if (data == NULL) {
        perror("Image data allocation failed\n");
        exit(2);
    }

    // perform apriltag setup
    ec = apriltag_setup(&td, &tf, &settings);
    if (ec) {
        printf("Setup returned error code: %d\n", ec);
    }

    // #TODO: create a proper g_loop and create a bus watch
    while(TRUE) {
        // pulling sample from camera and print bus error message, the only gstream functions used in a loop
        ec = gstream_pull_sample(&streams, data, &settings);
        if (ec) {
            continue;
            // do not exit, run something to fix the break in timing
        }

        // put image processing and data transmission functions below \/
        ec = apriltag_detect(&td, &tf, &det, data, &settings);
        if (ec) {
            printf("Apriltag detection returned error code: %d\n", ec);
            // do not exit, perform error handling based on what happened
        }
    }

    // gstreamer cleanup
    ec = gstream_cleanup(bus, &streams);
    if (ec) {
        g_printerr("Cleanup returned error code: %d\n", ec);
        exit(4);
    }
    // apriltag cleanup
    apriltag_cleanup(&td, &tf, &det);
    
    // free dynamically allocated image data array
    free(data);

    exit(0);
}
#include <settings.h>
#include <gstream_from_cam.h>
//#include <detect_apriltags.h>
//#include <transmit_pose.h>

int main(int argc, char *argv[]) {
    setenv("GST_DEBUG", "3", 1);
    gst_init(&argc, &argv);

    Settings settings;
    StreamSet streams;
    u_int8_t ec;
    GstBus *bus;
    u_int16_t *data;

    // read in settings from json file, #TODO: make the path an arg (using stropts?)
    ec = load_settings_from_path("/home/natec/apriltag_rpi_positioning/settings/settings.json", &settings);
    if(ec) {
        printf("Settings failed to load with error code: %d", ec);
        exit(1);
    }
    settings.np = settings.width * settings.height;

    // perform setup, check error output
    ec = gstream_setup(&streams, &settings, FALSE, FALSE);
    if (ec) {
        g_printerr("Setup returned error code: %d", ec);
        exit(1);
    }

    bus = gst_element_get_bus(streams.pipeline);

    // allocating data 
    data = malloc(settings.np * settings.stride);
    if (data == NULL) {
        perror("Image data allocation failed");
        exit(2);
    }

    // while(TRUE) {
    // pulling sample from camera and print bus error message, the only gstream functions used in a loop
    ec = gstream_pull_sample(&streams, data);
    if (ec) {
        g_printerr("Sampling returned error code: %d", ec);
        // do not exit, run something to fix the break in timing
    }
    // print messages tied to the bus, after every loop
    ec = print_bus_message(bus, &streams);
    if (ec) {
        g_printerr("Bus returned error code: %d", ec);
        exit(3);
    }

    // put image processing and data transmission functions below \/

    // }

    ec = gstream_cleanup(bus, &streams);
    if (ec) {
        g_printerr("Cleanup returned error code: %d", ec);
        exit(4);
    }
    
    // free dynamically allocated image data array
    free(data);

    exit(0);
}
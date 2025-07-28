#include <gstream_from_cam.h>
#include <settings.h>
#include <apriltag/apriltag.h>

int main(int argc, char *argv[]) {
    setenv("GST_DEBUG", "3", 1);
    gst_init(&argc, &argv);

    Settings settings; // global settings structure

    // gstreamer setup stuffs
    StreamSet streams;
    int ec;
    GstBus *bus;
    GstState *state1, *state2;

    uint8_t *data; // image data

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
    ec = apriltag_setup(&td, &tf, &info, &cd, &settings);
    if (ec) {
        printf("Setup returned error code: %d\n", ec);
    }

    printf("Press <Enter> once:");
    int key_ent = getchar();
    int i = 1;
    image_u8_t im = image_u8_create(settings->width, settings->height);

    // #TODO: create a proper g_loop and create a bus watch
    while(TRUE) {
        printf("Press <Enter> to capture image:");
        if (getchar() != key_ent) continue;

        // pulling sample from camera and print bus error message, the only gstream functions used in a loop
        ec = gstream_pull_sample(&streams, data, &settings);
        if (ec) {
            printf("Sample not taken");
            continue;
        }
        
        // copy captured image to the buffer, this accomodates extra row space in im
        for (int i = 0; i < settings.height; i++) {
            uint8_t* row_d = im->buf + i * im.stride;
            uint8_t* row_s = data + i * settings.width;
            memcpy(row_d, row_s, settings.width);
        }

        // write to specific location
        char path[100];
        strcpy(path, settings.images_directory);
        char number[5];
        sprintf(number, "%d", i);
        strcat(path, number);
        strcat(path, ".pnm");
        printf(path);
        printf("\n");

        ec = image_u8_write_pnm(im, path);
        if (ec) {
            printf("Image failed to write, error code %d", ec);
            continue;
        }

        i++;
    }

    // gstreamer cleanup
    ec = gstream_cleanup(bus, &streams);
    if (ec) {
        g_printerr("Cleanup returned error code: %d\n", ec);
        exit(4);
    }
    // apriltag cleanup
    apriltag_cleanup(&td, &tf, &info, &det);
    
    // free dynamically allocated image data array
    free(data);

    exit(0);
}
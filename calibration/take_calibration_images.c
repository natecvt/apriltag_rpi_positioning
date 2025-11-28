#include <gstream_from_cam.h>
#include <settings.h>
#include <apriltag/apriltag.h>

#define SKIP_FRAMES 4

int main(int argc, char *argv[]) {
    setenv("GST_DEBUG", "0", 1); // gets in the way of the first print
    gst_init(&argc, &argv);

    Settings settings; // global settings structure

    // gstreamer setup stuffs
    StreamSet streams;
    int ec;
    GstBus *bus;

    uint8_t *data; // image data

    ec = load_settings_from_path(argv[1], &settings);
    if (ec) {
        printf("Settings failed to load with error code: %d\n", ec);
        exit(1);
    }
    settings.np = settings.width * settings.height;

    // perform setup, check error output
    ec = gstream_setup(&streams, &settings, FALSE, TRUE);
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

    printf("\nPress <Enter> once:\n");
    int key_ent = getchar();
    int i = 1 - SKIP_FRAMES;
    image_u8_t *im = image_u8_create(settings.width, settings.height);

    printf("Press <Enter> to capture image:");

    // #TODO: create a proper g_loop and create a bus watch
    while(TRUE) {
        
        // pulling sample from camera and print bus error message, the only gstream functions used in a loop
        ec = gstream_pull_sample(&streams, data, &settings);
        if (ec) {
            printf("Sample not taken\n");
            continue;
        }

        if (i < 1) {
            i++;
            printf("Skipped frame\n");
            continue;
        }
        
        if (getchar() != key_ent) continue;
        // copy captured image to the buffer, this accomodates extra row space in im
        for (int i = 0; i < settings.height; i++) {
            uint8_t* row_d = im->buf + i * im->stride;
            uint8_t* row_s = data + i * settings.width;
            memcpy(row_d, row_s, settings.width);
        }

        // write to specific location
        char path[100];
        
        snprintf(path, 100, "%s%d.pnm", settings.images_directory, i);
        printf("%s\n", path);

        ec = image_u8_write_pnm(im, path);
        if (ec) {
            printf("Image failed to write, error code %d\n", ec);
            continue;
        }

        if (settings.n_cal_imgs == i) break;

        i++;
    }

    // gstreamer cleanup
    ec = gstream_cleanup(bus, &streams);
    if (ec) {
        g_printerr("Cleanup returned error code: %d\n", ec);
        exit(4);
    }
    
    // free dynamically allocated image data array
    free(data);

    exit(0);
}
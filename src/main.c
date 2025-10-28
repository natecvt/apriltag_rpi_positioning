#include <settings.h>
#include <gstream_from_cam.h>
#include <detect_apriltags.h>
#include <transmit_pose.h>
#include <logger.h>

#include <stdlib.h>

volatile sig_atomic_t stop;

void handle_sigint(int sig) {
    stop = 1;
}

int main(int argc, char *argv[]) {
    setenv("GST_DEBUG", "3", 1);
    gst_init(&argc, &argv);

    signal(SIGINT, handle_sigint);

    Settings settings; // global settings structure

    // gstreamer setup stuffs
    StreamSet streams;
    int ec;
    GstBus *bus;
    GstState *state1, *state2;

    uint8_t *data; // image data

    // apriltag items
    apriltag_detector_t *td;
    apriltag_family_t *tf;
    apriltag_detection_info_t info;
    apriltag_pose_t pose;
    int ids[MAX_DETECTIONS];

    // pose array and transmission
    CoordDefs cd;
    matd_t *p = matd_create(3, 1); // position vector
    matd_t *q = matd_create(4, 1); // quaternion vector

    UARTInfo uart_info;

    // read in settings from json file, #TODO: make the path an arg (using stropts?)
    ec = load_settings_from_path(argv[1], &settings);
    if(ec) {
        printf("Settings failed to load with error code: %d\n", ec);
        exit(1);
    }
    settings.np = settings.width * settings.height;

    // perform setup, check error output
    ec = gstream_setup(&streams, &settings, TRUE, FALSE);
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
    ec = apriltag_setup(&td, &tf, &info, &settings);
    if (ec) {
        printf("Setup returned error code: %d\n", ec);
    }

    printf("X %f\n", cd.center_x);
    printf("Z %f\n", cd.center_z);

    // perform UART setup
    ec = init_transmit_pose(&uart_info, &settings, &cd);
    if (ec) {
        printf("UART initialization failed with error code: %d\n", ec);
        exit(3);
    }

    // #TODO: create a proper g_loop and create a bus watch
    while(!stop) {
        // pulling sample from camera and print bus error message, the only gstream functions used in a loop
        ec = gstream_pull_sample(&streams, data, &settings);
        if (ec) {
            continue;
            // do not exit, run something to fix the break in timing
        }

        // detect apriltags and update the pose and ids array
        ec = apriltag_detect(td, data, &info, &pose, &settings, ids);
        if (ec) {
            printf("Apriltag detection returned error code: %d\n", ec);
            // do not exit, perform error handling based on what happened
            if (ec == 3) {
                // no detections, continue
                continue;
            }
            else {
                ec = pose_transform(p, q, &pose);
                if (ec) {
                    printf("Pose transformation returned error code: %d\n", ec);
                }

                break;
            }
        }
    }

    printf("Exiting main loop...\n");

    // gstreamer cleanup
    ec = gstream_cleanup(bus, &streams);
    if (ec) {
        g_printerr("Cleanup returned error code: %d\n", ec);
        exit(4);
    }
    // apriltag cleanup
    apriltag_cleanup(&td, &tf, &info);
    
    // free dynamically allocated image data array
    free(data);

    exit(0);
}
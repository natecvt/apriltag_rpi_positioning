#include <detect_apriltags.h>

// #TODO: create macros that help with different image bit depths/strides

int apriltag_setup(apriltag_detector_t **td, apriltag_family_t **tf, Settings *settings) 
{
    *td = apriltag_detector_create();

    switch(settings->tag_family) {
        case TAG36H10:
            *tf = tag36h10_create();
            break;
        case TAG36H11:
            *tf = tag36h11_create();
            break;
        case TAG25H9:
            *tf = tag25h9_create();
            break;
        case TAG16H5:
            *tf = tag16h5_create();
            break;
        case TAG27H7R:
            *tf = tagCircle21h7_create();
            break;
        case TAG49H12R:
            *tf = tagCircle49h12_create();
            break;
        case TAG41H12S:
            *tf = tagStandard41h12_create();
            break;
        case TAG52H13S:
            *tf = tagStandard52h13_create();
            break;
        case TAG48H12C:
            *tf = tagCustom48h12_create();
            break;
        default:
            printf("Unknown tag type entered: %d", settings->tag_family);
            return 1;
    }

    apriltag_detector_add_family_bits(*td, *tf, settings->hamming);

    switch(errno){
        case EINVAL:
            printf("\"hamming\" parameter is out-of-range.\n");
            exit(-1);
        case ENOMEM:
            printf("Unable to add family to detector due to insufficient memory to allocate the tag-family decoder.\n");
            exit(-1);
    }

    (*td)->debug = settings->debug;
    (*td)->nthreads = settings->threads;
    (*td)->quad_decimate = settings->dec;
    (*td)->quad_sigma = settings->blur;
    (*td)->refine_edges = settings->refine;

    return 0;
}

int apriltag_detect(apriltag_detector_t **td, apriltag_family_t **tf, zarray_t **det, uint8_t *imdata, Settings *settings) {
    // loop through iterations
    image_u8_t *im = NULL;

    for (uint8_t i = 0; i < settings->iterations; i++) {
        int total_quads = 0;
        double total_time = 0;

        im = image_u8_create(settings->width, settings->height);
        
        for (int i = 0; i < settings->height; i++) {
            uint8_t* row_d = im->buf + i * im->stride;
            uint8_t* row_s = imdata + i * settings->width;
            memcpy(row_d, row_s, settings->width);
        }

        if ((*td)->debug) {
            image_u8_write_pnm(im, "/home/natec/apriltag_rpi_positioning/output/debug.pgm");
        }

        *det = apriltag_detector_detect(*td, im);

        if (errno == EAGAIN) {
            printf("Unable to create the %d threads requested.\n",(*td)->nthreads);
            return 2;
        }

        for (int i = 0; i < zarray_size(*det); i++) {
            apriltag_detection_t *d;
            zarray_get(*det, i, &d);

            if (!settings->quiet)
                printf("detection %3d: id (%2dx%2d)-%-4d, hamming %d, margin %8.3f\n",
                        i, d->family->nbits, d->family->h, d->id, d->hamming, d->decision_margin);

            
        }

        if (!settings->quiet) {
                timeprofile_display((*td)->tp);
        }

        double t = timeprofile_total_utime((*td)->tp) / 1.0E3;
        total_time += t;
        printf("Time: %12.3f \n", t);

        image_u8_destroy(im);
    }

    return 0;
}

int apriltag_cleanup(apriltag_detector_t **td, apriltag_family_t **tf, zarray_t **det) {
    apriltag_detections_destroy(*det);
    tagStandard41h12_destroy(*tf);
    apriltag_detector_destroy(*td);

    return 0;
}
#include <detect_apriltags.h>

int dsetup(apriltag_detector_t *td, apriltag_family_t *tf, apriltag_settings *ts,
    uint8_t tag, 
    bool debug,
    bool quiet,
    uint8_t hamming, 
    uint8_t iters, 
    uint8_t threads, 
    float dec, 
    float blur,
    bool refine) 
{
    td = apriltag_detector_create();

    switch(tag) {
        case TAG36H10:
            tf = tag36h10_create();
            break;
        case TAG36H11:
            tf = tag36h11_create();
            break;
        case TAG25H9:
            tf = tag25h9_create();
            break;
        case TAG16H5:
            tf = tag16h5_create();
            break;
        case TAG27H7R:
            tf = tagCircle21h7_create();
            break;
        case TAG49H12R:
            tf = tagCircle49h12_create();
            break;
        case TAG41H12S:
            tf = tagStandard41h12_create();
            break;
        case TAG52H13S:
            tf = tagStandard52h13_create();
            break;
        case TAG48H12C:
            tf = tagCustom48h12_create();
            break;
        default:
            printf("Unknown tag type entered: %d", tag);
            return 1;
    }

    apriltag_detector_add_family_bits(td, tf, hamming);

    switch(errno){
        case EINVAL:
            printf("\"hamming\" parameter is out-of-range.\n");
            exit(-1);
        case ENOMEM:
            printf("Unable to add family to detector due to insufficient memory to allocate the tag-family decoder.\n");
            exit(-1);
    }

    td->debug = debug;
    td->nthreads = threads;
    td->quad_decimate = dec;
    td->quad_sigma = blur;
    td->refine_edges = refine;

    ts->quiet = quiet;
    ts->iters = iters;

    return 0;
}

int detect_from_image(apriltag_detector_t *td, apriltag_family_t *tf, apriltag_settings *ts, zarray_t *det, uint16_t *imdata, CustomData *cd) {
    // loop through iterations
    image_u8_t *im = NULL;

    for (int i = 0; i < ts->iters; i++) {
        
        int total_quads = 0;
        int total_hamm_hist[HAMM_HIST_MAX];
        memset(total_hamm_hist, 0, sizeof(int) * HAMM_HIST_MAX);

        int hamm_hist[HAMM_HIST_MAX];
        memset(hamm_hist, 0, sizeof(hamm_hist));

        double t_time = 0;

        im = image_u8_create(cd->iw, cd->ih);

        if (cd->stride == 2) {
            for (int j = 0; j < cd->np; j = j++) {
                im->buf[j] = imdata[j * cd->stride] >> 8;
            }
        }
        else {
            memcpy(im->buf, imdata, cd->np);
        }

        if (td->debug) {
            image_u8_write_pnm(im, "output/debug_invariant.pnm");
        }

        zarray_t *det = apriltag_detector_detect(td, im);

        if (errno == EAGAIN) {
            printf("Unable to create the %d threads requested.\n",td->nthreads);
            exit(-1);
        }

        for (int i = 0; i < zarray_size(det); i++) {
            apriltag_detection_t *d;
            zarray_get(det, i, &d);

            if (!ts->quiet)
                printf("detection %3d: id (%2dx%2d)-%-4d, hamming %d, margin %8.3f\n",
                        i, d->family->nbits, d->family->h, d->id, d->hamming, d->decision_margin);

            hamm_hist[d->hamming]++;
            total_hamm_hist[d->hamming]++;

            
        }
    }
}

int main(int argc, char *argv[]) {
    zarray_t *za;

    apriltag_detector_t *td;
    apriltag_family_t *tf;
    apriltag_settings *ts;
    uint8_t ec;

    // #TODO: add method to read these in from file
    ec = dsetup(td, tf, ts, 1, true, true, 2, 8, 2, 2.0f, 0.0f, true);
    if (ec) {
        printf("Setup returned error code: %d", ec);
    }

    ec = detect_from_image(td, tf, ts, za, NULL, NULL);
    if (ec) {
        printf("Loop returned error code: %d", ec);
    }
}
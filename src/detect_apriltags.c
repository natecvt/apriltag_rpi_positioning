#include <detect_apriltags.h>

// #TODO: create macros that help with different image bit depths/strides

int apriltag_setup(apriltag_detector_t **td, 
        apriltag_family_t **tf, 
        apriltag_detection_info_t *info,
        CoordDefs *cd,
        Settings *settings) {
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

    (*info).tagsize = settings->tag_size;
    (*info).fx = settings->fx;
    (*info).fy = settings->fy;
    (*info).cx = settings->cx;
    (*info).cy = settings->cy;

    (*cd).center_x = settings->grid_unit_length * (float)(settings->center_id % settings->grid_units_x);
    (*cd).center_y = settings->grid_unit_width * (float)(settings->center_id / settings->grid_units_x);
    (*cd).center_z = settings->grid_elevation;
    (*cd).ulength_x = settings->grid_unit_length;
    (*cd).uwidth_y = settings->grid_unit_width;
    (*cd).nx = settings->grid_units_x;
    (*cd).ny = settings->grid_units_y;

    return 0;
}

void pose_global_transform(float **global_pose, matd_t *centers, int **ids, CoordDefs *cd) {
    if (centers == NULL) {
        printf("Invalid matrix pointer entered\n");
        return;
    }

    if (centers->nrows == 0 || centers->ncols == 0) {
        printf("No tag found\n");
        return;
    }
    float x = 0.0, y = 0.0, z = 0.0;

    for (int j = 0; j < centers->ncols; j++) {
        int id = (*ids)[j];

        float tag_dx = cd->ulength_x * (id % cd->nx) - cd->center_x,
              tag_dy = cd->uwidth_y * (id / cd->nx) - cd->center_y;

        x += (tag_dx - centers->data[j])                            / centers->ncols;
        y += (tag_dy - centers->data[centers->ncols + j])           / centers->ncols;
        z += (cd->center_z + centers->data[2 * centers->ncols + j]) / centers->ncols;
    }

    (*global_pose)[0] = x;
    (*global_pose)[1] = y;
    (*global_pose)[2] = z;

}

int apriltag_detect(apriltag_detector_t **td,
        apriltag_family_t **tf, 
        zarray_t **det, 
        uint8_t *imdata, 
        apriltag_detection_info_t *info, 
        Settings *settings, 
        CoordDefs *cd,
        float *global_pose) {
    // loop through iterations
    image_u8_t *im = NULL;

    for (uint8_t i = 0; i < settings->iterations; i++) {
        int total_quads = 0;
        double total_time = 0;

        im = image_u8_create(settings->width, settings->height);
        
        // copy captured image to the buffer, this accomodates extra row space in im
        for (int i = 0; i < settings->height; i++) {
            uint8_t* row_d = im->buf + i * im->stride;
            uint8_t* row_s = imdata + i * settings->width;
            memcpy(row_d, row_s, settings->width);
        }

        // write the current image buffer to a file
        if ((*td)->debug) {
            char path[100];
            strcpy(path, settings->output_directory);
            strcat(path, "debug.pnm");
            printf(path);
            printf("\n");

            image_u8_write_pnm(im, path);
        }

        // get detections
        *det = apriltag_detector_detect(*td, im);

        if (errno == EAGAIN) {
            printf("Unable to create the %d threads requested.\n",(*td)->nthreads);
            return 2;
        }

        // loop through detections, every d* is a tag detected in the image
        int ndets = zarray_size(*det);
        int *ids = malloc(sizeof(int) * ndets);
        matd_t *centers = matd_create(4, ndets);

        for (int j = 0; j < ndets; j++) {
            apriltag_detection_t *d;
            zarray_get(*det, j, &d);

            if (!settings->quiet)
                printf("detection %3d: id (%2dx%2d)-%-4d, hamming %d, margin %8.3f\n",
                        j, d->family->nbits, d->family->h, d->id, d->hamming, d->decision_margin);
            
            // TODO: add pose estimation
            (*info).det = d;

            ids[j] = d->id;

            apriltag_pose_t pose;

            // get the pose (vector is cetered at cam center and points toward the tag center)
            double err = estimate_tag_pose(info, &pose);

            printf("Rotation matrix R = \n{%2.2f, %2.2f, %2.2f\n %2.2f, %2.2f, %2.2f\n %2.2f, %2.2f, %2.2f\n",
                pose.R->data[0], pose.R->data[1], pose.R->data[2],
                pose.R->data[3], pose.R->data[4], pose.R->data[5],
                pose.R->data[6], pose.R->data[7], pose.R->data[8]
            );

            printf("Position vector t = \n{%2.2f, %2.2f, %2.2f}\n", pose.t->data[0], pose.t->data[1], pose.t->data[2]);
            
            // copy the pose data to one row of centers
            // #TODO: add R-matrix as well
            centers->data[j]                      = pose.t->data[0];
            centers->data[centers->ncols + j]     = pose.t->data[1];
            centers->data[2 * centers->ncols + j] = pose.t->data[2];
            centers->data[3 * centers->ncols + j] = err;
        }

        pose_global_transform(&global_pose, centers, &ids, cd);

        matd_destroy(centers);

        // display total time
        if (!settings->quiet) {
                timeprofile_display((*td)->tp);
        }

        // calculate time and free the image
        double t = timeprofile_total_utime((*td)->tp) / 1.0E3;
        total_time += t;
        printf("Time: %12.3f \n", t);

        image_u8_destroy(im);
    }

    return 0;
}

int apriltag_cleanup(apriltag_detector_t **td, 
        apriltag_family_t **tf, 
        apriltag_detection_info_t *info, 
        zarray_t **det) {
    apriltag_detections_destroy(*det);
    tagStandard41h12_destroy(*tf);
    apriltag_detector_destroy(*td);

    return 0;
}
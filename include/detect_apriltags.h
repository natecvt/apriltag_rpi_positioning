#ifndef DETECT_APRILTAGS_H
#define DETECT_APRILTAGS_H

// external functionality
#include <settings.h>
#include <gstream_from_cam.h>

// apriltag functionality
#include <apriltag/apriltag.h>
#include <apriltag/apriltag_pose.h>
#include <apriltag/apriltag_math.h>

// apriltag types
#include <apriltag/tag16h5.h>
#include <apriltag/tag25h9.h>
#include <apriltag/tag36h10.h>
#include <apriltag/tag36h11.h>
#include "apriltag/tagCircle21h7.h"
#include "apriltag/tagCircle49h12.h"
#include "apriltag/tagCustom48h12.h"
#include "apriltag/tagStandard41h12.h"
#include "apriltag/tagStandard52h13.h"

#include <math.h>
#include <errno.h>

#define  HAMM_HIST_MAX 10

typedef struct CoordDefs {
    // center x and y from where tag id 0 is placed, z from ground level
    float center_x, center_y, center_z;
    // unit length and width of tiling in x and y, can be different
    float ulength_x, uwidth_y;
    // number of tags in the grid
    uint8_t nx, ny;
} CoordDefs;

int apriltag_setup(apriltag_detector_t **td, apriltag_family_t **tf, apriltag_detection_info_t *info, CoordDefs *cd, Settings *settings);

int apriltag_detect(apriltag_detector_t **td, apriltag_family_t **tf, zarray_t **det, uint8_t *imdata, apriltag_detection_info_t *info, Settings *settings, CoordDefs *cd, float *global_pose);

int apriltag_cleanup(apriltag_detector_t **td, apriltag_family_t **tf, apriltag_detection_info_t *info, zarray_t **det);

#endif
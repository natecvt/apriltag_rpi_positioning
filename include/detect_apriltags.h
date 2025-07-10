#pragma once

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

#include <gstream_from_cam.h>

#include <errno.h>

#define  HAMM_HIST_MAX 10

typedef struct _apriltag_settings {
    bool quiet;
    uint8_t iters;
} apriltag_settings;

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


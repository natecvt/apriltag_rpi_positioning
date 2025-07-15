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




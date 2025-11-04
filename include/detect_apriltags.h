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

#include <errno.h>

#define  HAMM_HIST_MAX 10

#define MAX_DETECTIONS 16

int apriltag_setup(apriltag_detector_t **td, apriltag_family_t **tf, apriltag_detection_info_t *info, Settings *settings);

int apriltag_detect(apriltag_detector_t *td, uint8_t *imdata, apriltag_detection_info_t *info, apriltag_pose_t *pose, Settings *settings, int *ids, uint8_t *nids);

int apriltag_cleanup(apriltag_detector_t **td, apriltag_family_t **tf, apriltag_detection_info_t *info);

#endif
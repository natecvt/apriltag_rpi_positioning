#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRANSMIT_POSE_H
#define TRANSMIT_POSE_H

#include <settings.h>

#include <apriltag/apriltag_pose.h>

#include <uart.h>
#include <math.h>

#define SMALL_NUM 0.0001

#define APRILTAG_START_BYTE1 0x00               ///< Start byte 1
#define APRILTAG_START_BYTE2 0xFF               ///< Start byte 2

typedef struct CoordDefs {
    // center x and y from where tag id 0 is placed, z from ground level
    float center_x, center_y, center_z;
    // unit length and width of tiling in x and y, can be different
    float ulength_x, uwidth_y;
    // number of tags in the grid
    uint8_t nx, ny;
} CoordDefs;

int init_transmit_pose(UARTInfo *uart_info, Settings *settings, CoordDefs *cd);

int pose_transform(matd_t *p, matd_t *q, apriltag_pose_t *pose);

int transmit_pose(UARTInfo *uart_info, matd_t *p, matd_t *q);

#endif

#ifdef __cplusplus
}
#endif
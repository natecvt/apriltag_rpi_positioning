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

#define APRILTAG_START_BYTE1 0x0A               ///< Start byte 1, newline character so each transmission is readable in output
#define APRILTAG_START_BYTE2 0x0A               ///< Start byte 2

typedef struct CoordDefs {
    // center x and y from where tag id 0 is placed, z from ground level
    float center_x, center_y, center_z;
    // unit length and width of tiling in x and y, can be different
    float ulength_x, uwidth_y;
    // number of tags in the grid
    uint8_t nx, ny;
} CoordDefs;

typedef enum sm_states
{
    STANDBY = 0,
    TAKEOFF = 1,
    GUIDED = 2,
    LANDING = 3,
    SM_LOITER = 4,
    NAILING = 5,
    RETURN = 6,
} sm_states;

int init_transmit_pose(UARTInfo *uart_info, Settings *settings, CoordDefs *cd);

int compare_integers(const void *a, const void *b);

int pose_transform(matd_t *p, matd_t *q, apriltag_pose_t *poses, CoordDefs *cd, int *ids, uint8_t nids);

int transmit_pose(UARTInfo *uart_info, struct timeval *time, matd_t *p, matd_t *q, uint8_t state);

#endif

#ifdef __cplusplus
}
#endif
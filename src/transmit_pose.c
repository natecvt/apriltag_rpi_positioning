#include <transmit_pose.h>

int compare_integers(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int init_transmit_pose(UARTInfo *uart_info, Settings *settings, CoordDefs *cd) {
    // Open UART device
    if (uart_open(uart_info, settings->uart_path) != 0) {
        return -1;
    }

    // Configure UART device
    if (uart_configure(uart_info, settings->uart_baudrate, 0, 1, 8, 0, 10) != 0) {
        uart_close(uart_info);
        return -2;
    }

    (*cd).center_x = - settings->grid_unit_length * (float)(settings->center_id % settings->grid_units_x);
    (*cd).center_y = - settings->grid_unit_width * (float)(settings->center_id / settings->grid_units_x);
    (*cd).center_z = settings->grid_elevation;
    (*cd).ulength_x = settings->grid_unit_length;
    (*cd).uwidth_y = settings->grid_unit_width;
    (*cd).nx = settings->grid_units_x;
    (*cd).ny = settings->grid_units_y;

    printf("Coordinate Definitions: cx %.3f, cy %.3f, cz %.3f\n", cd->center_x, cd->center_y, cd->center_z);

    return 0;
}

int pose_transform(matd_t *p, matd_t *q, apriltag_pose_t *poses, CoordDefs *cd, int *ids, uint8_t nids) {
    if (p == NULL || q == NULL || poses == NULL) {
        printf("pose_transform: NULL pointer input\n");
        return -1;
    }

    matd_t *tfp = matd_create(3, 1);
    matd_t *tfR = matd_create(3, 3);

    tfR = matd_transpose(poses[0].R);
    tfp = matd_multiply(tfR, poses[0].t);

    // Extract translation vector
    float px = tfp->data[0];
    float py = tfp->data[1];
    float pz = tfp->data[2];

    MATD_EL(p, 0, 0) = cd->center_x + cd->ulength_x * (float)(ids[0] % cd->nx) + px;
    MATD_EL(p, 1, 0) = cd->center_y + cd->uwidth_y * (float)(ids[0] / cd->ny) + py;
    MATD_EL(p, 2, 0) = pz - cd->center_z;

    // Convert rotation matrix to quaternion
    float R[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            R[i][j] = matd_get(tfR, i, j);
        }
    }

    // Calculate quaternion components
    float trace = R[0][0] + R[1][1] + R[2][2];
    float qw, qx, qy, qz;

    if (trace > 0) {
        float S = sqrt(trace + 1.0) * 2; // S=4*qw

        if (abs(S) < SMALL_NUM) S = SMALL_NUM; // prevent division by zero

        qw = 0.25 * S;
        qx = (R[2][1] - R[1][2]) / S;
        qy = (R[0][2] - R[2][0]) / S;
        qz = (R[1][0] - R[0][1]) / S;
    } else if ((R[0][0] > R[1][1]) && (R[0][0] > R[2][2])) {
        float S = sqrt(1.0 + R[0][0] - R[1][1] - R[2][2]) * 2; // S=4*qx

        if (abs(S) < SMALL_NUM) S = SMALL_NUM;

        qw = (R[2][1] - R[1][2]) / S;
        qx = 0.25 * S;
        qy = (R[0][1] + R[1][0]) / S;
        qz = (R[0][2] + R[2][0]) / S;
    } else if (R[1][1] > R[2][2]) {
        float S = sqrt(1.0 + R[1][1] - R[0][0] - R[2][2]) * 2; // S=4*qy

        if (abs(S) < SMALL_NUM) S = SMALL_NUM;

        qw = (R[0][2] - R[2][0]) / S;
        qx = (R[0][1] + R[1][0]) / S;
        qy = 0.25 * S;
        qz = (R[1][2] + R[2][1]) / S;
    } else {
        float S = sqrt(1.0 + R[2][2] - R[0][0] - R[1][1]) * 2; // S=4*qz

        if (abs(S) < SMALL_NUM) S = SMALL_NUM;

        qw = (R[1][0] - R[0][1]) / S;
        qx = (R[0][2] + R[2][0]) / S;
        qy = (R[1][2] + R[2][1]) / S;
        qz = 0.25 * S;
    }

    MATD_EL(q, 0, 0) = qx;
    MATD_EL(q, 1, 0) = qy;
    MATD_EL(q, 2, 0) = qz;
    MATD_EL(q, 3, 0) = qw;

    return 0;
}

int transmit_pose(UARTInfo *uart_info, matd_t *p, matd_t *q) {
    // Transmit the pose data over UART
    uint8_t start_bytes = {0, 255};
    ssize_t bytes_written = uart_write(uart_info, start_bytes, 2);

    bytes_written = uart_write(uart_info, (uint8_t *)p->data, sizeof(float) * 3);
    if (bytes_written == -1 || bytes_written != sizeof(float) * 3) {
        return -1;
    }

    return 0;
}

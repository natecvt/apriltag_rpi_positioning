#include <transmit_pose.h>

int init_transmit_pose(UARTInfo *uart_info, Settings *settings, CoordDefs *cd) {
    // Open UART device
    if (uart_open(uart_info, settings->uart_path) == -1) {
        return -1;
    }

    // Configure UART device
    if (uart_configure(uart_info, settings->uart_baudrate, 0, 1, 8, 0, 10) == -1) {
        uart_close(uart_info);
        return -1;
    }

    (*cd).center_x = settings->grid_unit_length * (float)(settings->center_id % settings->grid_units_x);
    (*cd).center_y = settings->grid_unit_width * (float)(settings->center_id / settings->grid_units_x);
    (*cd).center_z = settings->grid_elevation;
    (*cd).ulength_x = settings->grid_unit_length;
    (*cd).uwidth_y = settings->grid_unit_width;
    (*cd).nx = settings->grid_units_x;
    (*cd).ny = settings->grid_units_y;

    return 0;
}

int pose_transform(matd_t *p, matd_t *q, apriltag_pose_t *pose) {};

int transmit_pose(UARTInfo *uart_info, float *pose) {
    // Transmit the pose data over UART
    ssize_t bytes_written = uart_write(uart_info, (uint8_t *)pose, sizeof(float) * 3);
    if (bytes_written == -1 || bytes_written != sizeof(float) * 3) {
        return -1;
    }

    return 0;
}

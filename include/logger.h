#ifndef LOGGER_H
#define LOGGER_H

#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <apriltag/common/matd.h>

#define LOG_DIR "/home/natec/apriltag_rpi_positioning/output/"
#define DEFAULT_LOG_NAME "log"
#define LOG_EXTENSION ".csv"

#define LO_NONE 0b00000000
#define LO_EN 0b00000001
#define LO_EN_IMAGES 0b00000010
#define LO_EN_TIME 0b00000100
#define LO_EN_IDS 0b00001000
#define LO_EN_POSES 0b00010000
#define LO_EN_QUATS 0b00100000

typedef struct Logger {
    int log_fd;

    bool do_logging;
    bool log_images;
    bool log_time;
    bool log_ids;
    bool log_poses;
    bool log_quats;
} Logger;

int name_logfile(char *buf);

int init_logger(Logger *logger, const char *log_file_path, uint8_t options);

int log_message(Logger *logger, matd_t *p, matd_t *q, int *ids, int num_ids);

int close_logger(Logger *logger);

#endif // LOGGER_H
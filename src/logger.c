#include <logger.h>

int name_logfile(char *buf) {
    int i = 0;
    struct stat st;
    
    if (stat(LOG_DIR, &st) == -1) {
        mkdir(LOG_DIR, 0755);
    }

    while (1) {
        snprintf(buf, 256, "%s%s%d%s", LOG_DIR, DEFAULT_LOG_NAME, i, LOG_EXTENSION);
        if (stat(buf, &st) == -1) {
            break;
        }
        i++;
    }

    return 0;
}

int init_logger(Logger *logger, const char *log_file_path, uint8_t options) {

    int log_fd = open(log_file_path, O_CREAT | O_WRONLY | O_APPEND, 0644);

    if (log_fd == -1) {
        perror("Failed to open log file");
        return -1;
    }

    logger->log_fd = log_fd;
    logger->do_logging = 0b00000001 & options;
    logger->log_images = 0b00000010 & options;
    logger->log_time =   0b00000100 & options;
    logger->log_ids =    0b00001000 & options;
    logger->log_poses =  0b00010000 & options;
    logger->log_quats =  0b00100000 & options;

    // Write CSV header
    if (logger->log_time) dprintf(logger->log_fd, "Time,");
    if (logger->log_ids) {
        dprintf(logger->log_fd, "IDs,");
        dprintf(logger->log_fd, "nIDs,");
    }
    if (logger->log_poses) dprintf(logger->log_fd, "pX,pY,pZ,");
    if (logger->log_quats) dprintf(logger->log_fd, "qW,qX,qY,qZ");

    dprintf(logger->log_fd, "\n");

    return 0;
}

int log_message(Logger *logger, matd_t *p, matd_t *q, int *ids, int num_ids, struct timeval *tstart, struct timeval *tstop) {
    if (!logger->do_logging) {
        printf("Logging not enabled.\n");
        return 0;
    }

    
    if (logger->log_time) {
        gettimeofday(tstop, NULL);

        int ms_elapsed = (tstop->tv_sec - tstart->tv_sec) * 1000 + (tstop->tv_usec - tstart->tv_usec) / 1000;

        dprintf(logger->log_fd, "%d,", ms_elapsed);
        gettimeofday(tstart, NULL);
    }
    

    if (logger->log_ids) {
        for (int i = 0; i < num_ids; i++) {
            dprintf(logger->log_fd, "%d/", ids[i]);
        }
        dprintf(logger->log_fd, ",%d,", num_ids);
    }

    if (logger->log_poses) {
        dprintf(logger->log_fd, "%.6f,%.6f,%.6f,", matd_get(p, 0, 0), matd_get(p, 1, 0), matd_get(p, 2, 0));
    }

    if (logger->log_quats) {
        dprintf(logger->log_fd, "%.6f,%.6f,%.6f,%.6f", matd_get(q, 0, 0), matd_get(q, 1, 0), matd_get(q, 2, 0), matd_get(q, 3, 0));
    }

    dprintf(logger->log_fd, "\n");

    return 0;
}

int close_logger(Logger *logger) {
    if (close(logger->log_fd) == -1) {
        perror("Failed to close log file");
        return -1;
    }

    return 0;
}
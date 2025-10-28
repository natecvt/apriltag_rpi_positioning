#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

int init_logger(const char *log_file_path);

int log_message();

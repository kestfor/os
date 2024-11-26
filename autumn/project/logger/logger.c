#include "logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

typedef struct Logger {
    FILE *stream;
    int logging_level;
} Logger;



Logger *logger_create(FILE *stream, enum LogLevel level) {
    Logger *res = malloc(sizeof(Logger));
    res->stream = stream;
    res->logging_level = level;
    return res;
}

void logger_clear(Logger *l) {
    free(l);
}

void logger_message(Logger *l, const char *msg, enum LogLevel level) {

    char *level_str;
    char *color;
    switch (level) {
        case INFO:
            level_str = "INFO";
            color = GRN;
            break;
        case ERROR:
            level_str = "ERROR";
            color = MAG;
            break;
        case WARNING:
            level_str = "WARNING";
            color = YEL;
            break;
        case CRITICAL:
            level_str = "CRITICAL";
            color = RED;
            break;
    }

    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char time_str[128];
    asctime_r(timeinfo, time_str);
    time_str[strlen(time_str) - 1] = '\0';
    if (l->logging_level > level) {
        return;
    } else {
        char *reset = RESET;
        if (l->stream != stdout && l->stream != stderr) {
            color = "";
            reset = "";
        }
        fprintf(l->stream,  "%s%s::%s::%s%s\n", color, level_str, time_str, msg, reset);
    }
}

void logger_info(Logger *l, const char *format) {
    logger_message(l, format, INFO);
}

void logger_warning(Logger *l, const char *format) {
    logger_message(l, format, WARNING);
}

void logger_error(Logger *l, const char *format) {
    logger_message(l, format, ERROR);
}

void logger_critical(Logger *l, const char *format) {
    logger_message(l, format, CRITICAL);
}
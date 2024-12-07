#ifndef PROXY_LOGGER_H
#define PROXY_LOGGER_H
#include <stdio.h>
typedef struct Logger Logger;

enum LogLevel {
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4,
};

Logger *logger_create(FILE *stream, enum LogLevel level);

void logger_clear(Logger *l);

void logger_message(const Logger *l, const char *msg, enum LogLevel level);

void logger_info(const Logger *l, const char *msg);

void logger_warning(const Logger *l, const char *msg);

void logger_error(const Logger *l, const char *msg);

void logger_critical(const Logger *l, const char *msg);

#endif //PROXY_LOGGER_H

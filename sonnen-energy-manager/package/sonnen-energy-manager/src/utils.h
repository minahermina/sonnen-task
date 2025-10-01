#ifndef SONNEN_UTILS_H
#define SONNEN_UTILS_H

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>

/* Global signal handler flag */
extern volatile sig_atomic_t g_keep_running;

/* Utility Functions (implemented in utils.c) */
void setup_signal_handlers(void);
void signal_handler(int signum);

/* Logging functions */
void log_info(const char *prefix, const char *format, ...);
void log_warning(const char *prefix, const char *format, ...);
void log_error(const char *prefix, const char *format, ...);
void log_debug(const char *prefix, const char *format, ...);

#endif /*SONNEN_UTILS_H*/

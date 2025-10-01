#include "utils.h"
volatile sig_atomic_t g_keep_running = 1;

/**
 * Signal handler for graceful shutdown
 */
void
signal_handler(int signum)
{
    (void)signum;
    g_keep_running = 0;
}

void
setup_signal_handlers(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Ignore SIGPIPE (write to closed socket)*/
    signal(SIGPIPE, SIG_IGN);
}

/**
 * Internal logging function (used by all log functions)
 */
static void
log_internal(const char *prefix, int level, const char *format, va_list args)
{
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);

    /* Log to syslog with appropriate level */
    syslog(level, "[%s] %s", prefix, buffer);
}

void
log_info(const char *prefix, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(prefix, LOG_INFO, format, args);
    va_end(args);
}

void
log_warning(const char *prefix, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(prefix, LOG_WARNING, format, args);
    va_end(args);
}

void
log_error(const char *prefix, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(prefix, LOG_ERR, format, args);
    va_end(args);
}

void
log_debug(const char *prefix, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_internal(prefix, LOG_DEBUG, format, args);
    va_end(args);
}

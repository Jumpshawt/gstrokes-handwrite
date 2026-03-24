#ifndef LOG_H
#define LOG_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_indent(void);
void log_unindent(void);
void log_error(const char *const fmt, ...);
void log_warning(const char *const fmt, ...);
void log_debug(const char *const fmt, ...);
void log_info(const char *const fmt, ...);
void log_append_error(const char *const fmt, ...);
void log_append_warning(const char *const fmt, ...);
void log_append_debug(const char *const fmt, ...);
void log_append_info(const char *const fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */

#ifndef LOGGER_H
#define LOGGER_H

#include "types.h"

// Log severity levels (matching mGBA debug levels)
typedef enum {
    LOG_FATAL = 0,
    LOG_ERROR = 1,
    LOG_WARN  = 2,
    LOG_INFO  = 3,
    LOG_DEBUG = 4
} LogLevel;

// SRAM Header for persistent emulator log ring buffer
#define SRAM_LOG_MAGIC       "AEROLOG1"
#define SRAM_LOG_MAGIC_LEN   8
#define SRAM_LOG_BASE        (0x0E000200)
#define SRAM_LOG_TOTAL_SIZE  (0x0E008000 - SRAM_LOG_BASE) // 32256 bytes

typedef struct {
    char magic[8];        // "AEROLOG1"
    u32 write_offset;    // Current byte offset in data buffer (0 .. capacity - 1)
    u32 entry_count;     // Total log entries written since creation
    u32 wrap_count;      // How many times ring buffer has wrapped
    u32 capacity;        // Usable data buffer size in bytes
} __attribute__((packed)) SramLogHeader;

#ifdef __cplusplus
extern "C" {
#endif

// Initialize logging system: handshakes with mGBA/No$GBA and validates SRAM ring buffer
void log_init(void);

// Advance engine frame counter (called on vsync)
void log_tick(void);

// Returns current engine frame count
u32 log_get_frame(void);

// Check if mGBA debug interface was detected
bool log_is_mgba_active(void);

// Primary formatted logging function
void log_printf(LogLevel level, const char* tag, const char* fmt, ...);

// Convenience macros
#define LOG_DEBUG(tag, fmt, ...) log_printf(LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#define LOG_INFO(tag, fmt, ...)  log_printf(LOG_INFO,  tag, fmt, ##__VA_ARGS__)
#define LOG_WARN(tag, fmt, ...)  log_printf(LOG_WARN,  tag, fmt, ##__VA_ARGS__)
#define LOG_ERROR(tag, fmt, ...) log_printf(LOG_ERROR, tag, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // LOGGER_H

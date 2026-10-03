#include "logger.h"
#include "gba.h"
#include <stdarg.h>

// Hardware register definitions for mGBA and No$GBA debug I/O
#define REG_MGBA_ENABLE     (*(vu16*)0x04FFF780)
#define REG_MGBA_FLAGS      (*(vu16*)0x04FFF700)
#define REG_MGBA_STRING     ((char*)0x04FFF600)
#define REG_NOCASH_CHAR     (*(vu8*)0x04FFFA1C)
#define REG_WAITCNT         (*(vu16*)(IO_BASE + 0x0204))

// SRAM Identification tag (detected by emulators to configure 32KB SRAM save file)
__attribute__((used)) static const char s_sram_tag[] = "SRAM_V110";

static bool s_mgba_active = false;
static u32 s_frame_counter = 0;

#define SRAM_DATA_BASE (SRAM_LOG_BASE + sizeof(SramLogHeader))
#define SRAM_LOG_CAPACITY (SRAM_LOG_TOTAL_SIZE - sizeof(SramLogHeader))

// 8-bit safe SRAM read/write helpers
static inline u8 sram_read_u8(u32 addr) {
    return *(vu8*)addr;
}

static inline void sram_write_u8(u32 addr, u8 val) {
    *(vu8*)addr = val;
}

static inline u32 sram_read_u32(u32 addr) {
    vu8* p = (vu8*)addr;
    return ((u32)p[0]) | (((u32)p[1]) << 8) | (((u32)p[2]) << 16) | (((u32)p[3]) << 24);
}

static inline void sram_write_u32(u32 addr, u32 val) {
    vu8* p = (vu8*)addr;
    p[0] = (u8)(val & 0xFF);
    p[1] = (u8)((val >> 8) & 0xFF);
    p[2] = (u8)((val >> 16) & 0xFF);
    p[3] = (u8)((val >> 24) & 0xFF);
}

void log_init(void) {
    // 1. Configure waitstates for 8-cycle SRAM access (safe for all flashcarts & emulators)
    REG_WAITCNT |= 0x0003;

    // 2. Handshake with mGBA debug interface
    REG_MGBA_ENABLE = 0xC0DE;
    if (REG_MGBA_ENABLE == 0x1DEA) {
        s_mgba_active = true;
    }

    // 3. Initialize or validate Cartridge SRAM Ring Buffer
    bool valid = true;
    for (int i = 0; i < SRAM_LOG_MAGIC_LEN; i++) {
        if (sram_read_u8(SRAM_LOG_BASE + i) != (u8)SRAM_LOG_MAGIC[i]) {
            valid = false;
            break;
        }
    }

    if (!valid) {
        // Write fresh header
        for (int i = 0; i < SRAM_LOG_MAGIC_LEN; i++) {
            sram_write_u8(SRAM_LOG_BASE + i, (u8)SRAM_LOG_MAGIC[i]);
        }
        sram_write_u32(SRAM_LOG_BASE + 8,  0);                 // write_offset
        sram_write_u32(SRAM_LOG_BASE + 12, 0);                 // entry_count
        sram_write_u32(SRAM_LOG_BASE + 16, 0);                 // wrap_count
        sram_write_u32(SRAM_LOG_BASE + 20, SRAM_LOG_CAPACITY); // capacity
    }
}

void log_tick(void) {
    s_frame_counter++;
}

u32 log_get_frame(void) {
    return s_frame_counter;
}

bool log_is_mgba_active(void) {
    return s_mgba_active;
}

// Lightweight custom integer-to-string formatters (avoids libc bloat)
static int mini_utoa(u32 val, char* buf, int base, bool uppercase, int min_digits) {
    char temp[16];
    int count = 0;
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    if (val == 0) {
        temp[count++] = '0';
    } else {
        while (val > 0) {
            temp[count++] = digits[val % base];
            val /= base;
        }
    }

    while (count < min_digits && count < 15) {
        temp[count++] = '0';
    }

    for (int i = 0; i < count; i++) {
        buf[i] = temp[count - 1 - i];
    }
    return count;
}

static int mini_itoa(s32 val, char* buf) {
    if (val < 0) {
        buf[0] = '-';
        return 1 + mini_utoa((u32)(-val), buf + 1, 10, false, 1);
    }
    return mini_utoa((u32)val, buf, 10, false, 1);
}

// Freestanding snprintf-compatible formatter
static int mini_vsnprintf(char* buf, u32 size, const char* fmt, va_list args) {
    if (size == 0) return 0;
    u32 written = 0;

    while (*fmt && written < size - 1) {
        if (*fmt != '%') {
            buf[written++] = *fmt++;
            continue;
        }

        fmt++; // Skip '%'
        if (*fmt == '%') {
            buf[written++] = '%';
            fmt++;
            continue;
        }

        // Optional width / padding like %02d or %06lu
        int min_digits = 1;
        if (*fmt == '0') {
            fmt++;
            if (*fmt >= '0' && *fmt <= '9') {
                min_digits = *fmt - '0';
                fmt++;
            }
        }

        // Long modifier (%lu, %ld, %lx)
        bool is_long = false;
        if (*fmt == 'l') {
            is_long = true;
            fmt++;
        }

        if (*fmt == 's') {
            const char* s = va_arg(args, const char*);
            if (!s) s = "(null)";
            while (*s && written < size - 1) {
                buf[written++] = *s++;
            }
            fmt++;
        } else if (*fmt == 'd' || *fmt == 'i') {
            s32 val = is_long ? (s32)va_arg(args, long) : va_arg(args, int);
            char num_buf[16];
            int len = mini_itoa(val, num_buf);
            for (int i = 0; i < len && written < size - 1; i++) {
                buf[written++] = num_buf[i];
            }
            fmt++;
        } else if (*fmt == 'u') {
            u32 val = is_long ? (u32)va_arg(args, unsigned long) : va_arg(args, unsigned int);
            char num_buf[16];
            int len = mini_utoa(val, num_buf, 10, false, min_digits);
            for (int i = 0; i < len && written < size - 1; i++) {
                buf[written++] = num_buf[i];
            }
            fmt++;
        } else if (*fmt == 'x' || *fmt == 'X') {
            u32 val = is_long ? (u32)va_arg(args, unsigned long) : va_arg(args, unsigned int);
            char num_buf[16];
            int len = mini_utoa(val, num_buf, 16, (*fmt == 'X'), min_digits);
            for (int i = 0; i < len && written < size - 1; i++) {
                buf[written++] = num_buf[i];
            }
            fmt++;
        } else if (*fmt == 'c') {
            char c = (char)va_arg(args, int);
            buf[written++] = c;
            fmt++;
        } else {
            // Unrecognized specifier, output as-is
            buf[written++] = '%';
            if (written < size - 1) buf[written++] = *fmt;
            fmt++;
        }
    }

    buf[written] = '\0';
    return written;
}

static const char* s_level_names[] = {
    "FATAL", "ERROR", "WARN ", "INFO ", "DEBUG"
};

void log_printf(LogLevel level, const char* tag, const char* fmt, ...) {
    char line[256];
    u32 pos = 0;

    // 1. Format standard timestamp prefix: [F:001234] [INFO ] [TAG] 
    const char* lvl_str = (level <= LOG_DEBUG) ? s_level_names[level] : "?????";
    char prefix[48];
    // e.g. "[F:000123] [INFO ] [PLAYER] "
    int p_len = mini_utoa(s_frame_counter, prefix, 10, false, 6);
    line[pos++] = '[';
    line[pos++] = 'F';
    line[pos++] = ':';
    for (int i = 0; i < p_len; i++) line[pos++] = prefix[i];
    line[pos++] = ']';
    line[pos++] = ' ';
    line[pos++] = '[';
    for (int i = 0; i < 5; i++) line[pos++] = lvl_str[i];
    line[pos++] = ']';
    line[pos++] = ' ';
    line[pos++] = '[';
    while (*tag && pos < 40) line[pos++] = *tag++;
    line[pos++] = ']';
    line[pos++] = ' ';

    // 2. Format user message
    va_list args;
    va_start(args, fmt);
    pos += mini_vsnprintf(line + pos, sizeof(line) - pos - 2, fmt, args);
    va_end(args);

    // 3. Ensure newline and null-terminator
    line[pos++] = '\n';
    line[pos] = '\0';

    // 4. Output to mGBA Debug Interface
    if (s_mgba_active) {
        // Copy line up to 255 chars into mGBA debug register buffer
        for (u32 i = 0; i < pos && i < 255; i++) {
            REG_MGBA_STRING[i] = line[i];
        }
        REG_MGBA_STRING[(pos < 255) ? pos : 255] = '\0';
        REG_MGBA_FLAGS = (level & 0xF) | 0x100;
    }

    // 5. Output to No$GBA char port
    for (u32 i = 0; i < pos; i++) {
        REG_NOCASH_CHAR = (u8)line[i];
    }

    // 6. Write to Persistent SRAM Ring Buffer
    u32 write_offset = sram_read_u32(SRAM_LOG_BASE + 8);
    u32 entry_count  = sram_read_u32(SRAM_LOG_BASE + 12);
    u32 wrap_count   = sram_read_u32(SRAM_LOG_BASE + 16);
    u32 capacity     = sram_read_u32(SRAM_LOG_BASE + 20);
    if (capacity == 0 || capacity > SRAM_LOG_CAPACITY) {
        capacity = SRAM_LOG_CAPACITY;
    }

    for (u32 i = 0; i < pos; i++) {
        sram_write_u8(SRAM_DATA_BASE + write_offset, (u8)line[i]);
        write_offset++;
        if (write_offset >= capacity) {
            write_offset = 0;
            wrap_count++;
        }
    }

    entry_count++;
    sram_write_u32(SRAM_LOG_BASE + 8,  write_offset);
    sram_write_u32(SRAM_LOG_BASE + 12, entry_count);
    sram_write_u32(SRAM_LOG_BASE + 16, wrap_count);
}

#ifndef GBA_H
#define GBA_H

#include "types.h"

// Memory map base addresses
#define EWRAM_BASE      0x02000000
#define IWRAM_BASE      0x03000000
#define IO_BASE         0x04000000
#define PALRAM_BASE     0x05000000
#define VRAM_BASE       0x06000000
#define OAM_BASE        0x07000000
#define ROM_BASE        0x08000000

// Display registers
#define REG_DISPCNT     (*(vu16*)(IO_BASE + 0x0000))
#define REG_DISPSTAT    (*(vu16*)(IO_BASE + 0x0004))
#define REG_VCOUNT      (*(vu16*)(IO_BASE + 0x0006))

// Background Control registers
#define REG_BG0CNT      (*(vu16*)(IO_BASE + 0x0008))
#define REG_BG1CNT      (*(vu16*)(IO_BASE + 0x000A))
#define REG_BG2CNT      (*(vu16*)(IO_BASE + 0x000C))
#define REG_BG3CNT      (*(vu16*)(IO_BASE + 0x000E))

// Background Scroll offsets (Write-only)
#define REG_BG0HOFS     (*(vu16*)(IO_BASE + 0x0010))
#define REG_BG0VOFS     (*(vu16*)(IO_BASE + 0x0012))
#define REG_BG1HOFS     (*(vu16*)(IO_BASE + 0x0014))
#define REG_BG1VOFS     (*(vu16*)(IO_BASE + 0x0016))

// Key Input register
#define REG_KEYINPUT    (*(vu16*)(IO_BASE + 0x0130))

// DMA Channel 3 (General purpose high-speed transfer)
#define REG_DMA3SAD     (*(vu32*)(IO_BASE + 0x00D4))
#define REG_DMA3DAD     (*(vu32*)(IO_BASE + 0x00D8))
#define REG_DMA3CNT     (*(vu32*)(IO_BASE + 0x00DC))

// Sound registers
#define REG_SOUNDCNT_L  (*(vu16*)(IO_BASE + 0x0080))
#define REG_SOUNDCNT_H  (*(vu16*)(IO_BASE + 0x0082))
#define REG_SOUNDCNT_X  (*(vu16*)(IO_BASE + 0x0084))
#define REG_SOUND1CNT_L (*(vu16*)(IO_BASE + 0x0060))
#define REG_SOUND1CNT_H (*(vu16*)(IO_BASE + 0x0062))
#define REG_SOUND1CNT_X (*(vu16*)(IO_BASE + 0x0064))
#define REG_SOUND4CNT_L (*(vu16*)(IO_BASE + 0x0078))
#define REG_SOUND4CNT_H (*(vu16*)(IO_BASE + 0x007C))

// DISPCNT bitmasks
#define MODE_0          0x0000
#define MODE_1          0x0001
#define MODE_2          0x0002
#define BG0_ENABLE      0x0100
#define BG1_ENABLE      0x0200
#define BG2_ENABLE      0x0400
#define BG3_ENABLE      0x0800
#define OBJ_ENABLE      0x1000
#define OBJ_1D_MAP      0x0040

// BG Control bit helpers
#define BG_CBB(x)       ((x) << 2)   // Character Base Block (tiles) 0..3
#define BG_SBB(x)       ((x) << 8)   // Screen Base Block (map) 0..31
#define BG_SIZE_32x32   (0 << 14)
#define BG_SIZE_64x32   (1 << 14)
#define BG_COLOR_16     (0 << 7)
#define BG_COLOR_256    (1 << 7)

// DMA Control flags
#define DMA_ENABLE      0x80000000
#define DMA_16          0x00000000
#define DMA_32          0x04000000
#define DMA_IMMEDIATE   0x00000000

// Controller Button Masks (Active LOW in hardware)
#define KEY_A           (1 << 0)
#define KEY_B           (1 << 1)
#define KEY_SELECT      (1 << 2)
#define KEY_START       (1 << 3)
#define KEY_RIGHT       (1 << 4)
#define KEY_LEFT        (1 << 5)
#define KEY_UP          (1 << 6)
#define KEY_DOWN        (1 << 7)
#define KEY_R           (1 << 8)
#define KEY_L           (1 << 9)

// Memory pointers
#define BG_PALETTE_RAM   ((vu16*)PALRAM_BASE)
#define OBJ_PALETTE_RAM  ((vu16*)(PALRAM_BASE + 0x0200))
#define VRAM             ((vu16*)VRAM_BASE)
#define OAM              ((vu16*)OAM_BASE)

// Hardware OAM Sprite Attribute Struct (8 bytes per sprite, 128 total)
typedef struct {
    u16 attr0;
    u16 attr1;
    u16 attr2;
    u16 dummy;
} __attribute__((packed, aligned(4))) OBJ_ATTR;

#define MAX_SPRITES 128

#endif // GBA_H

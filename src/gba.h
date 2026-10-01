// Minimal GBA hardware definitions.
#ifndef GBA_H
#define GBA_H
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

#define REG_DISPCNT (*(volatile u16 *)0x04000000)
#define REG_VCOUNT (*(volatile u16 *)0x04000006)
#define REG_BG0CNT (*(volatile u16 *)0x04000008)
#define REG_BG1CNT (*(volatile u16 *)0x0400000A)
#define REG_BG0HOFS (*(volatile u16 *)0x04000010)
#define REG_BG0VOFS (*(volatile u16 *)0x04000012)
#define REG_BG1HOFS (*(volatile u16 *)0x04000014)
#define REG_BG1VOFS (*(volatile u16 *)0x04000016)
#define REG_BLDCNT (*(volatile u16 *)0x04000050)
#define REG_BLDY (*(volatile u16 *)0x04000054)
#define REG_KEYINPUT (*(volatile u16 *)0x04000130)
#define REG_WAITCNT (*(volatile u16 *)0x04000204)

#define DCNT_MODE0 0
#define DCNT_OBJ_1D 0x0040
#define DCNT_BG0 0x0100
#define DCNT_BG1 0x0200
#define DCNT_OBJ 0x1000

#define BG_CBB(n) ((n) << 2)
#define BG_SBB(n) ((n) << 8)
#define BG_4BPP 0
#define BG_REG_32x32 0
#define BG_REG_64x64 0xC000
#define BG_PRIO(n) (n)

#define MEM_PAL_BG ((volatile u16 *)0x05000000)
#define MEM_PAL_OBJ ((volatile u16 *)0x05000200)
#define MEM_VRAM ((volatile u16 *)0x06000000)
#define MEM_OAM ((volatile u16 *)0x07000000)
#define CHARBLOCK(n) ((volatile u16 *)(0x06000000 + (n) * 0x4000))
#define SCREENBLOCK(n) ((volatile u16 *)(0x06000000 + (n) * 0x800))
#define OBJ_TILES ((volatile u16 *)0x06010000)
#define SRAM ((volatile u8 *)0x0E000000)

#define KEY_A 0x0001
#define KEY_B 0x0002
#define KEY_SELECT 0x0004
#define KEY_START 0x0008
#define KEY_RIGHT 0x0010
#define KEY_LEFT 0x0020
#define KEY_UP 0x0040
#define KEY_DOWN 0x0080
#define KEY_R 0x0100
#define KEY_L 0x0200

// OAM attributes
#define ATTR0_HIDE 0x0200
#define ATTR0_SQUARE 0x0000
#define ATTR0_TALL 0x8000
#define ATTR1_SIZE(n) ((n) << 14)  // square: 1 = 16x16; tall: 2 = 16x32, 3 = 32x64
#define ATTR1_HFLIP 0x1000

#define RGB15(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

void *memcpy(void *dst, const void *src, unsigned n);
void *memset(void *dst, int v, unsigned n);
void copy16(volatile u16 *dst, const void *src, unsigned bytes);
void fill16(volatile u16 *dst, u16 v, unsigned count);
void vsync(void);

#endif

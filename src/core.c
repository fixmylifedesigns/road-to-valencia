// Low-level helpers: memory, vsync and sprite attribute memory.
#include "game.h"

void *memcpy(void *dst, const void *src, unsigned n) {
  u8 *d = dst;
  const u8 *s = src;
  while (n--) *d++ = *s++;
  return dst;
}

void *memset(void *dst, int v, unsigned n) {
  u8 *d = dst;
  while (n--) *d++ = (u8)v;
  return dst;
}

// VRAM, palette and OAM only take 16/32-bit writes.
void copy16(volatile u16 *dst, const void *src, unsigned bytes) {
  const u16 *s = src;
  for (unsigned i = 0; i < bytes / 2; i++) dst[i] = s[i];
}

void fill16(volatile u16 *dst, u16 v, unsigned count) {
  for (unsigned i = 0; i < count; i++) dst[i] = v;
}

void vsync(void) {
  while (REG_VCOUNT >= 160) {}
  while (REG_VCOUNT < 160) {}
}

static u16 oam[128 * 4];

void oam_clear(void) {
  for (int i = 0; i < 128; i++) {
    oam[i * 4] = ATTR0_HIDE;
    oam[i * 4 + 1] = 0;
    oam[i * 4 + 2] = 0;
  }
}

void oam_set(int i, u16 a0, u16 a1, u16 a2) {
  oam[i * 4] = a0;
  oam[i * 4 + 1] = a1;
  oam[i * 4 + 2] = a2;
}

void oam_commit(void) {
  for (int i = 0; i < 128 * 4; i++) {
    if ((i & 3) != 3) MEM_OAM[i] = oam[i];
  }
}

// A 16x32 person sprite whose top-left is at screen (x, y).
void spr_person(int slot, int tile, int pal, int x, int y) {
  if (x <= -16 || x >= 240 || y <= -32 || y >= 160) {
    oam_set(slot, ATTR0_HIDE, 0, 0);
    return;
  }
  oam_set(slot, (y & 0xFF) | ATTR0_TALL, (x & 0x1FF) | ATTR1_SIZE(2), tile | (1 << 10) | (pal << 12));
}

// Unsigned division without the libgcc helper (shift-and-subtract).
u32 udiv(u32 n, u32 d) {
  if (!d) return 0;
  u32 q = 0, r = 0;
  for (int i = 31; i >= 0; i--) {
    r = (r << 1) | ((n >> i) & 1);
    if (r >= d) {
      r -= d;
      q |= 1u << i;
    }
  }
  return q;
}

// Text and window drawing on background 1 (8x8 font, 30x20 characters on screen).
#include "game.h"

static volatile u16 *const map = SCREENBLOCK(TXT_SBB);

void txt_init(void) {
  copy16(CHARBLOCK(2), ui_tiles, sizeof(ui_tiles));
  copy16(MEM_PAL_BG + 12 * 16, ui_pal, sizeof(ui_pal));
  txt_clear();
}

void txt_clear(void) { fill16(map, 0, 32 * 32); }

void txt_put(int x, int y, int tile, int pal) {
  if (x < 0 || y < 0 || x >= 32 || y >= 32) return;
  map[y * 32 + x] = tile | (pal << 12);
}

void txt_fill(int x, int y, int w, int h, int tile, int pal) {
  for (int j = y; j < y + h; j++)
    for (int i = x; i < x + w; i++) txt_put(i, j, tile, pal);
}

void txt_box(int x, int y, int w, int h) {
  txt_fill(x + 1, y + 1, w - 2, h - 2, UI_FILL, PAL_TEXT);
  txt_put(x, y, UI_TL, PAL_TEXT);
  txt_put(x + w - 1, y, UI_TR, PAL_TEXT);
  txt_put(x, y + h - 1, UI_BL, PAL_TEXT);
  txt_put(x + w - 1, y + h - 1, UI_BR, PAL_TEXT);
  for (int i = x + 1; i < x + w - 1; i++) {
    txt_put(i, y, UI_T, PAL_TEXT);
    txt_put(i, y + h - 1, UI_B, PAL_TEXT);
  }
  for (int j = y + 1; j < y + h - 1; j++) {
    txt_put(x, j, UI_L, PAL_TEXT);
    txt_put(x + w - 1, j, UI_R, PAL_TEXT);
  }
}

int txt_print(int x, int y, const char *s, int pal) {
  int n = 0;
  while (s[n]) {
    char c = s[n];
    txt_put(x + n, y, (c >= 32 && c < 127) ? c : '?', pal);
    n++;
  }
  return n;
}

// Progress bar made of 8-pixel segments.
void txt_bar(int x, int y, int w, int pct, int pal) {
  int fill = (int)udiv((u32)(pct * w * 8), 100);
  for (int i = 0; i < w; i++) {
    int seg = fill - i * 8;
    if (seg < 0) seg = 0;
    if (seg > 8) seg = 8;
    txt_put(x + i, y, UI_BAR0 + seg, pal);
  }
}

int str_len(const char *s) {
  int n = 0;
  while (s[n]) n++;
  return n;
}

// Length of the next line when wrapping at `width` characters (breaks at spaces).
int txt_line(const char *s, int width) {
  int len = str_len(s);
  if (len <= width) return len;
  int cut = width;
  while (cut > 0 && s[cut] != ' ') cut--;
  return cut > 0 ? cut : width;
}

const char *txt_skip(const char *s, int len) {
  s += len;
  while (*s == ' ') s++;
  return s;
}

int txt_wrap_lines(const char *s, int width) {
  int lines = 0;
  while (*s) {
    s = txt_skip(s, txt_line(s, width));
    lines++;
  }
  return lines ? lines : 1;
}

char *fmt_int(char *dst, int v) {
  char tmp[8];
  int n = 0;
  if (v < 0) v = 0;
  do {
    int d = 0;
    while (v >= 10) {  // divide by 10 without the libgcc helper
      v -= 10;
      d++;
    }
    tmp[n++] = '0' + v;
    v = d;
  } while (v > 0 && n < 7);
  while (n) *dst++ = tmp[--n];
  *dst = 0;
  return dst;
}

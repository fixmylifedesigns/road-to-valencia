// Journal: journey progress, route choices and the full checklist with ticks.
#include "game.h"

#define PAGES (1 + STAGE_COUNT)
#define LIST_Y 6
#define LIST_ROWS 12

static int page, cursor, scroll;
static int items[TASK_COUNT], nitems;

static void collect(void) {
  nitems = 0;
  if (page == 0) {
    nitems = 5;  // 3 partner routes + 2 fingerprint routes
    return;
  }
  for (int t = 0; t < TASK_COUNT; t++)
    if (TASKS[t].stage == page - 1 && task_active(t)) items[nitems++] = t;
}

static int item_rows(int i) {
  if (page == 0) return 1;
  return txt_wrap_lines(TASKS[items[i]].title, 25);
}

static void header(void) {
  int d, t;
  progress(-1, &d, &t);
  txt_fill(0, 0, 30, 20, UI_FILL, PAL_TEXT);
  txt_fill(0, 0, 30, 3, UI_FILL, PAL_NAVY);
  txt_print(1, 0, "Journal", PAL_NAVY);
  char buf[16], *p = fmt_int(buf, d);
  *p++ = '/';
  p = fmt_int(p, t);
  const char *done = " done";
  while (*done) *p++ = *done++;
  *p = 0;
  txt_print(29 - str_len(buf), 0, buf, PAL_NAVY);
  txt_bar(1, 1, 28, percent(d, t), PAL_NAVY);

  const char *title = page == 0 ? "Routes" : STAGE_NAMES[page - 1];
  txt_print(1, 3, "<", PAL_TEXT);
  txt_print(28, 3, ">", PAL_TEXT);
  int len = str_len(title);
  txt_print(15 - len / 2, 3, title, PAL_TEXT);
  if (page > 0) {
    progress(page - 1, &d, &t);
    txt_bar(3, 4, 18, percent(d, t), PAL_TEXT);
    p = fmt_int(buf, d);
    *p++ = '/';
    fmt_int(p, t);
    txt_print(23, 4, buf, PAL_TEXT);
  } else {
    txt_print(3, 4, "A picks a route", PAL_DIM);
  }
  txt_print(1, 19, page == 0 ? "L/R page  B close" : "A tick  L/R page  B close", PAL_DIM);
}

static void draw(void) {
  header();
  collect();
  if (cursor >= nitems) cursor = nitems ? nitems - 1 : 0;
  if (cursor < scroll) scroll = cursor;
  // make sure the cursor's item fits on screen
  for (;;) {
    int rows = 0;
    for (int i = scroll; i <= cursor; i++) rows += item_rows(i) + (page > 0 ? 0 : 0);
    if (rows <= LIST_ROWS || scroll >= cursor) break;
    scroll++;
  }
  int y = LIST_Y;
  if (page == 0) {
    txt_print(1, y - 1, "Bringing Moeno", PAL_TEXT);
    for (int i = 0; i < 3; i++) {
      int on = g_save.partner == i + 1, sel = cursor == i;
      int pal = sel ? PAL_HILITE : PAL_TEXT;
      txt_fill(1, y + i, 28, 1, UI_FILL, pal);
      txt_put(2, y + i, on ? UI_BOX_ON : UI_BOX, pal);
      txt_print(4, y + i, PARTNER_LABELS[i + 1], pal);
    }
    txt_print(1, y + 4, "FBI fingerprints", PAL_TEXT);
    for (int i = 0; i < 2; i++) {
      int on = g_save.fbi == i + 1, sel = cursor == 3 + i;
      int pal = sel ? PAL_HILITE : PAL_TEXT;
      txt_fill(1, y + 5 + i, 28, 1, UI_FILL, pal);
      txt_put(2, y + 5 + i, on ? UI_BOX_ON : UI_BOX, pal);
      txt_print(4, y + 5 + i, FBI_LABELS[i + 1], pal);
    }
    txt_print(1, y + 9, "Moeno's Osaka missions", PAL_DIM);
    txt_print(1, y + 10, "arrive in the next update.", PAL_DIM);
    return;
  }
  for (int i = scroll; i < nitems; i++) {
    int t = items[i];
    int rows = item_rows(i);
    if (y + rows > LIST_Y + LIST_ROWS) break;
    int sel = i == cursor;
    int pal = sel ? PAL_HILITE : (task_done(t) ? PAL_DIM : PAL_TEXT);
    txt_fill(1, y, 28, rows, UI_FILL, pal);
    txt_put(2, y, task_done(t) ? UI_BOX_ON : UI_BOX, pal);
    const char *s = TASKS[t].title;
    for (int r = 0; r < rows; r++) {
      int len = txt_line(s, 25);
      for (int k = 0; k < len; k++) txt_put(4 + k, y + r, s[k], pal);
      s = txt_skip(s, len);
    }
    y += rows;
  }
  if (scroll > 0) txt_put(28, LIST_Y, '^', PAL_DIM);
  if (y >= LIST_Y + LIST_ROWS && nitems > 0) txt_put(28, LIST_Y + LIST_ROWS - 1, 'v', PAL_DIM);
}

void journal_open(void) {
  page = 1;
  cursor = scroll = 0;
  oam_clear();
  oam_commit();
  REG_DISPCNT &= ~DCNT_BG0;
  draw();
}

int journal_update(u16 pressed) {
  if (pressed & (KEY_B | KEY_SELECT | KEY_START)) {
    txt_clear();
    REG_DISPCNT |= DCNT_BG0;
    return 0;
  }
  int redraw = 0;
  if (pressed & (KEY_R | KEY_RIGHT)) {
    page = page + 1 < PAGES ? page + 1 : 0;
    cursor = scroll = 0;
    redraw = 1;
  }
  if (pressed & (KEY_L | KEY_LEFT)) {
    page = page ? page - 1 : PAGES - 1;
    cursor = scroll = 0;
    redraw = 1;
  }
  collect();
  if ((pressed & KEY_DOWN) && cursor + 1 < nitems) {
    cursor++;
    redraw = 1;
  }
  if ((pressed & KEY_UP) && cursor > 0) {
    cursor--;
    redraw = 1;
  }
  if (pressed & KEY_A) {
    if (page == 0) {
      if (cursor < 3) set_partner(g_save.partner == cursor + 1 ? P_NONE : cursor + 1);
      else set_fbi(g_save.fbi == cursor - 2 ? F_NONE : cursor - 2);
    } else if (nitems) {
      task_set(items[cursor], !task_done(items[cursor]));
    }
    save_write();
    redraw = 1;
  }
  if (redraw) draw();
  return 1;
}

// Road to Valencia: title, character select, overworld loop, menus and fades.
#include "game.h"

enum { S_TITLE, S_SELECT, S_WORLD, S_JOURNAL };

u32 g_frame;
static int state, has_save;
static int title_idx, confirm_new;
static int select_idx;
static int menu_open, menu_idx;
static int banner_timer;
static const char *banner_text;

// ---------- fades ----------
static int fade_level, fade_dir;
static void (*fade_mid)(void);

void fade_to(void (*mid)(void)) {
  fade_mid = mid;
  fade_dir = 1;
}

static void fade_step(void) {
  if (!fade_dir) return;
  fade_level += fade_dir * 2;
  if (fade_level >= 16) {
    fade_level = 16;
    fade_dir = -1;
    if (fade_mid) {
      void (*m)(void) = fade_mid;
      fade_mid = 0;
      m();
    }
  } else if (fade_level <= 0) {
    fade_level = 0;
    fade_dir = 0;
  }
  REG_BLDCNT = 0x3F | (3 << 6);  // darken every layer and the backdrop
  REG_BLDY = fade_level;
}

// ---------- overlays ----------
void show_banner(const char *text) {
  banner_text = text;
  banner_timer = 150;
}

void hud_draw(void) {
  int d, t;
  progress(-1, &d, &t);
  int pct = percent(d, t);
  char buf[20] = "Journey ";
  char *p = fmt_int(buf + 8, pct);
  *p++ = '%';
  *p = 0;
  txt_fill(18, 0, 12, 2, UI_FILL, PAL_NAVY);
  txt_print(19, 0, buf, PAL_NAVY);
  txt_bar(19, 1, 10, pct, PAL_NAVY);
}

static void draw_world_overlays(void) {
  txt_fill(0, 0, 30, 3, UI_BLANK, 0);
  txt_fill(0, 17, 30, 3, UI_BLANK, 0);
  if (menu_open) {
    const char *other = g_save.character == CH_IRVING ? "Play Moeno" : "Play Irving";
    const char *items[3] = {"Journal", other, "Close"};
    txt_box(17, 0, 13, 5);
    for (int i = 0; i < 3; i++) {
      txt_put(18, 1 + i, i == menu_idx ? UI_CURSOR : UI_FILL, PAL_TEXT);
      txt_fill(19, 1 + i, 10, 1, UI_FILL, PAL_TEXT);
      txt_print(19, 1 + i, items[i], PAL_TEXT);
    }
    return;
  }
  txt_fill(17, 3, 13, 2, UI_BLANK, 0);
  hud_draw();
  if (banner_timer > 0) {
    int w = str_len(banner_text) + 4;
    txt_box(0, 0, w, 3);
    txt_print(2, 1, banner_text, PAL_TEXT);
  }
  int door = world_facing_door();
  if (door >= 0) {
    static const char *const names[] = {"U.S. Consulate", "Osaka Station", "Home", "Cafe Nomado", "Fingerprint Service"};
    const char *name = door < 5 ? names[door] : "";
    int w = str_len(name) + 4;
    int x = 15 - w / 2;
    txt_box(x, 17, w, 3);
    txt_print(x + 2, 18, name, PAL_TEXT);
  }
}

// ---------- title ----------
static void title_bg(void) {
  REG_DISPCNT &= ~DCNT_BG0;
  copy16(CHARBLOCK(0), bg_title_tiles, sizeof(bg_title_tiles));
  copy16(MEM_PAL_BG, bg_title_pal, sizeof(bg_title_pal));
  copy16(SCREENBLOCK(28), bg_title_map, sizeof(bg_title_map));
  REG_BG0CNT = BG_CBB(0) | BG_SBB(28) | BG_REG_32x32 | BG_PRIO(1);
  REG_BG0HOFS = 0;
  REG_BG0VOFS = 0;
  REG_DISPCNT |= DCNT_BG0;
}

static void title_draw(void) {
  txt_clear();
  if (confirm_new) {
    txt_box(3, 11, 24, 6);
    txt_print(5, 12, "Erase your save and", PAL_TEXT);
    txt_print(5, 13, "start a new game?", PAL_TEXT);
    txt_put(6, 15, title_idx == 0 ? UI_CURSOR : UI_FILL, PAL_TEXT);
    txt_print(7, 15, "No", PAL_TEXT);
    txt_put(14, 15, title_idx == 1 ? UI_CURSOR : UI_FILL, PAL_TEXT);
    txt_print(15, 15, "Yes", PAL_TEXT);
    return;
  }
  int n = has_save ? 2 : 1;
  txt_box(9, 12, 12, n + 2);
  int row = 13;
  if (has_save) {
    txt_put(10, row, title_idx == 0 ? UI_CURSOR : UI_FILL, PAL_TEXT);
    txt_print(11, row++, "Continue", PAL_TEXT);
  }
  txt_put(10, row, (!has_save || title_idx == 1) ? UI_CURSOR : UI_FILL, PAL_TEXT);
  txt_print(11, row, "New game", PAL_TEXT);
  txt_print(1, 19, "fixmylife", PAL_NAVY);
  txt_print(29 - str_len(GAME_VERSION), 19, GAME_VERSION, PAL_NAVY);
}

static void title_enter(void) {
  state = S_TITLE;
  title_idx = 0;
  confirm_new = 0;
  oam_clear();
  title_bg();
  title_draw();
}

static void start_world(void) {
  state = S_WORLD;
  menu_open = 0;
  txt_clear();
  world_enter();
  show_banner("Osaka");
  if (!g_save.intro_seen) {
    dlg_begin();
    script_intro();
    dlg_start(0);
  }
}

// ---------- character select ----------
static const char *const CH_NAMES[2] = {"Irving", "Moeno"};
static const char *const CH_LINES[2] = {
    "Remote developer. Holder of the laptop and most of the paperwork.",
    "Coming to Valencia with Mui. Her Osaka missions arrive next update.",
};

static void select_draw(void) {
  txt_clear();
  txt_box(1, 0, 28, 3);
  txt_print(2, 1, "Who's walking around Osaka?", PAL_TEXT);
  for (int i = 0; i < 2; i++) {
    int bx = i == 0 ? 6 : 18;
    if (i == select_idx) txt_box(bx, 4, 6, 10);
    else txt_fill(bx, 4, 6, 10, UI_FILL, PAL_NAVY);
    int pal = i == select_idx ? PAL_TEXT : PAL_NAVY;
    txt_print(bx + 3 - str_len(CH_NAMES[i]) / 2, 12, CH_NAMES[i], pal);
  }
  txt_box(0, 14, 30, 6);
  const char *s = CH_LINES[select_idx];
  for (int r = 0; r < 3 && *s; r++) {
    int len = txt_line(s, 26);
    for (int k = 0; k < len; k++) txt_put(2 + k, 15 + r, s[k], PAL_TEXT);
    s = txt_skip(s, len);
  }
  oam_clear();
  oam_set(0, (32 & 0xFF) | ATTR0_TALL, 56 | ATTR1_SIZE(3), OBJ_IRVING_BIG | (OBJPAL_IRVING << 12));
  oam_set(1, (32 & 0xFF) | ATTR0_TALL, 152 | ATTR1_SIZE(3), OBJ_MOENO_BIG | (OBJPAL_MOENO << 12));
}

static void choose_character(void) {
  save_reset();
  g_save.character = select_idx;
  save_write();
  has_save = 1;
  fade_to(start_world);
}

// ---------- main loop ----------
static void sparkle_title(void) {
  if (g_frame % 24) return;
  int flip = (g_frame / 24) & 1;
  for (int i = 0; i < TITLE_SPARK_A_COUNT; i++) MEM_PAL_BG[title_spark_a[i]] = flip ? SPARK_B_COLOR : SPARK_A_COLOR;
  for (int i = 0; i < TITLE_SPARK_B_COUNT; i++) MEM_PAL_BG[title_spark_b[i]] = flip ? SPARK_A_COLOR : SPARK_B_COLOR;
}

int main(void) {
  REG_WAITCNT = 0x4317;  // faster cartridge access
  REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG1;
  REG_BG1CNT = BG_CBB(2) | BG_SBB(TXT_SBB) | BG_REG_32x32 | BG_PRIO(0);
  txt_init();
  copy16(OBJ_TILES, obj_tiles, sizeof(obj_tiles));
  copy16(MEM_PAL_OBJ, obj_pal, sizeof(obj_pal));
  MEM_PAL_BG[0] = RGB15(3, 5, 9);
  has_save = save_load();
  title_enter();

  u16 prev = 0xFFFF;
  for (;;) {
    vsync();
    oam_commit();
    u16 keys = ~REG_KEYINPUT & 0x3FF;
    u16 pressed = keys & ~prev;
    prev = keys;
    g_frame++;
    fade_step();
    if (fade_dir) continue;

    switch (state) {
      case S_TITLE:
        sparkle_title();
        if (confirm_new) {
          if (pressed & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN)) title_idx ^= 1, title_draw();
          if (pressed & KEY_B) confirm_new = 0, title_idx = 1, title_draw();
          if (pressed & (KEY_A | KEY_START)) {
            confirm_new = 0;
            if (title_idx == 1) {
              state = S_SELECT;
              select_idx = 0;
              select_draw();
            } else {
              title_idx = 1;
              title_draw();
            }
          }
          break;
        }
        if (has_save && (pressed & (KEY_UP | KEY_DOWN))) title_idx ^= 1, title_draw();
        if (pressed & (KEY_A | KEY_START)) {
          if (has_save && title_idx == 0) {
            fade_to(start_world);
          } else if (has_save) {
            confirm_new = 1;
            title_idx = 0;
            title_draw();
          } else {
            state = S_SELECT;
            select_idx = 0;
            select_draw();
          }
        }
        break;

      case S_SELECT:
        sparkle_title();
        if (pressed & (KEY_LEFT | KEY_RIGHT)) select_idx ^= 1, select_draw();
        if (pressed & KEY_B) title_enter();
        else if (pressed & (KEY_A | KEY_START)) {
          oam_clear();
          choose_character();
        }
        break;

      case S_WORLD:
        if (dlg_active()) {
          dlg_update(pressed);
          world_draw();
          break;
        }
        if (banner_timer > 0) banner_timer--;
        if (menu_open) {
          if (pressed & KEY_UP) menu_idx = menu_idx ? menu_idx - 1 : 2;
          if (pressed & KEY_DOWN) menu_idx = menu_idx < 2 ? menu_idx + 1 : 0;
          if (pressed & (KEY_B | KEY_START)) menu_open = 0;
          if (pressed & KEY_A) {
            menu_open = 0;
            if (menu_idx == 0) {
              state = S_JOURNAL;
              journal_open();
              break;
            }
            if (menu_idx == 1) {
              g_save.character ^= 1;
              save_write();
              show_banner(g_save.character == CH_MOENO ? "Moeno" : "Irving");
            }
          }
          draw_world_overlays();
          world_draw();
          break;
        }
        if (pressed & KEY_START) {
          menu_open = 1;
          menu_idx = 0;
          draw_world_overlays();
          world_draw();
          break;
        }
        if (pressed & KEY_SELECT) {
          state = S_JOURNAL;
          journal_open();
          break;
        }
        world_update(keys, pressed);
        if (state == S_WORLD && !dlg_active() && !fade_dir) draw_world_overlays();
        world_draw();
        break;

      case S_JOURNAL:
        if (!journal_update(pressed)) state = S_WORLD;
        break;
    }
  }
}

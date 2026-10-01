// Shared game declarations.
#ifndef GAME_H
#define GAME_H
#include "gba.h"
#include "assets.h"

// ---------- facing ----------
enum { FACE_DOWN, FACE_UP, FACE_RIGHT, FACE_LEFT };

// ---------- checklist ----------
enum { ST_OSAKA, ST_PAPERWORK, ST_APPLY, ST_VALENCIA, STAGE_COUNT };
enum { WHO_IRVING, WHO_MOENO, WHO_BOTH };
enum { P_NONE, P_CA, P_PAREJA, P_OWN };
enum { F_NONE, F_OSAKA, F_LATER };
enum { CH_IRVING, CH_MOENO };

enum {
  TK_MEETING, TK_COC_REQUEST, TK_CHOOSE_PARTNER, TK_CHOOSE_FBI, TK_NOTARY, TK_FBI_PRINTS,
  TK_FBI_PRINTS_LATER, TK_FBI_CHECK, TK_FBI_APOSTILLE, TK_COC_RECEIVED, TK_CA_FILE, TK_CA_APOSTILLE,
  TK_TRANSLATIONS, TK_INSURANCE, TK_MOENO_VISA_PICK, TK_MUI_CHIP, TK_MUI_CERT,
  TK_SUBMIT_DNV, TK_MOENO_FAMILY, TK_MOENO_OWN_APPLY, TK_DNV_APPROVED, TK_MUI_FLIGHT,
  TK_FLAT, TK_PADRON, TK_TIE, TK_PAREJA_REGISTER, TK_MOENO_FAMILY_SPAIN, TK_MUI_HOME,
  TASK_COUNT
};

typedef struct {
  u8 stage, who, cond;
  const char *title;
  const char *where;
} Task;

typedef struct {
  u32 magic;
  u8 version, character, partner, fbi;
  u8 intro_seen, x, y, facing;
  u32 done;
} Save;

extern Save g_save;
extern const Task TASKS[TASK_COUNT];
extern const char *const STAGE_NAMES[STAGE_COUNT];
extern const char *const STAGE_BLURBS[STAGE_COUNT];
extern const char *const PARTNER_LABELS[4];
extern const char *const FBI_LABELS[3];

int task_active(int t);
int task_done(int t);
void task_set(int t, int done);
void set_partner(int p);
void set_fbi(int f);
void progress(int stage, int *done, int *total);  // stage -1 = whole journey
int percent(int done, int total);
int osaka_complete(void);
void save_reset(void);
int save_load(void);
void save_write(void);

// ---------- text layer ----------
#define TXT_SBB 27
#define PAL_HILITE 12
#define PAL_DIM 13
#define PAL_NAVY 14
#define PAL_TEXT 15
enum {
  UI_BLANK = 0, UI_FILL = 1, UI_TL = 2, UI_T, UI_TR, UI_L, UI_R, UI_BL, UI_B, UI_BR,
  UI_CURSOR = 10, UI_MORE = 11, UI_BOX = 12, UI_BOX_ON = 13, UI_BAR0 = 14, UI_FRAME = 23
};
void txt_init(void);
void txt_clear(void);
void txt_put(int x, int y, int tile, int pal);
void txt_fill(int x, int y, int w, int h, int tile, int pal);
void txt_box(int x, int y, int w, int h);
int txt_print(int x, int y, const char *s, int pal);
void txt_bar(int x, int y, int w, int pct, int pal);
int txt_line(const char *s, int width);  // length of the next wrapped line
const char *txt_skip(const char *s, int len);
int txt_wrap_lines(const char *s, int width);
char *fmt_int(char *dst, int v);
int str_len(const char *s);
u32 udiv(u32 n, u32 d);

// ---------- dialog ----------
enum { EFF_NONE, EFF_DONE, EFF_PARTNER, EFF_FBI, EFF_PRINTS_HERE, EFF_INTRO_SEEN };
typedef struct {
  const char *label;
  u8 eff, arg;
  void (*then)(void);
} Option;

void dlg_begin(void);
void dlg_say(const char *who, const char *text);
void dlg_ask(const char *who, const char *text, const Option *opts, int n);
void dlg_fx(int eff, int arg);
const char *dlg_str(const char *a, int n, const char *b);
void dlg_start(void (*on_done)(void));
int dlg_active(void);
void dlg_update(u16 pressed);
Option *dlg_options(int n);

// ---------- scripts ----------
void script_building(int b);
void script_intro(void);
void script_npc(int n);
void script_sign(int b);
void script_look(int tile);

// ---------- world ----------
void world_enter(void);
void world_update(u16 keys, u16 pressed);
void world_draw(void);
void world_load_overworld(void);
void interior_load(int b);
void interior_draw(void);
int world_facing_door(void);

// ---------- sprites ----------
void oam_clear(void);
void oam_set(int i, u16 a0, u16 a1, u16 a2);
void oam_commit(void);
void spr_person(int slot, int tile, int pal, int x, int y);

// ---------- journal ----------
void journal_open(void);
int journal_update(u16 pressed);  // returns 0 when closed

// ---------- main ----------
extern u32 g_frame;
void fade_to(void (*mid)(void));
void show_banner(const char *text);
void hud_draw(void);

#endif

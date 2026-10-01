// Text boxes with typewriter text, multiple pages and choice menus.
#include "game.h"

enum { N_SAY, N_ASK, N_FX };
typedef struct {
  u8 type, eff, arg, nopt;
  const char *who;
  const char *text;
  const Option *opts;
} Node;

#define MAX_NODES 48
#define BOX_Y 13
#define BOX_H 7
#define TEXT_X 2
#define TEXT_W 26
#define LINES 3

static Node q[MAX_NODES];
static int count, cur, ins, active;
static void (*done_cb)(void);

static const char *line_start[LINES];
static int line_len[LINES], nlines, shown, page_chars;
static const char *page_next;
static int choosing, choice;

static char arena[640];
static int arena_used;
static Option optpool[24];
static int opt_used;

void dlg_begin(void) {
  count = cur = ins = 0;
  arena_used = 0;
  opt_used = 0;
}

static Node *insert_node(void) {
  if (count >= MAX_NODES) return &q[MAX_NODES - 1];
  for (int i = count; i > ins; i--) q[i] = q[i - 1];
  count++;
  Node *n = &q[ins++];
  memset(n, 0, sizeof(*n));
  return n;
}

void dlg_say(const char *who, const char *text) {
  Node *n = insert_node();
  n->type = N_SAY;
  n->who = who;
  n->text = text;
}

void dlg_ask(const char *who, const char *text, const Option *opts, int nopt) {
  Node *n = insert_node();
  n->type = N_ASK;
  n->who = who;
  n->text = text;
  n->opts = opts;
  n->nopt = nopt;
}

void dlg_fx(int eff, int arg) {
  Node *n = insert_node();
  n->type = N_FX;
  n->eff = eff;
  n->arg = arg;
}

Option *dlg_options(int n) {
  Option *o = &optpool[opt_used];
  opt_used += n;
  if (opt_used > 24) opt_used = 24;
  memset(o, 0, sizeof(Option) * n);
  return o;
}

// Builds "a" + number + "b" in the dialog's scratch space.
const char *dlg_str(const char *a, int num, const char *b) {
  char *out = arena + arena_used;
  char *p = out;
  while (*a) *p++ = *a++;
  if (num >= 0) p = fmt_int(p, num);
  while (b && *b) *p++ = *b++;
  *p++ = 0;
  arena_used = p - arena;
  return out;
}

static void apply(int eff, int arg) {
  switch (eff) {
    case EFF_DONE: task_set(arg, 1); break;
    case EFF_PARTNER: set_partner(arg); break;
    case EFF_FBI: set_fbi(arg); break;
    case EFF_PRINTS_HERE:
      set_fbi(F_OSAKA);
      task_set(TK_FBI_PRINTS, 1);
      break;
    case EFF_INTRO_SEEN: g_save.intro_seen = 1; break;
    default: return;
  }
  save_write();
}

static void layout_page(const char *text) {
  nlines = 0;
  page_chars = 0;
  while (*text && nlines < LINES) {
    int len = txt_line(text, TEXT_W);
    line_start[nlines] = text;
    line_len[nlines] = len;
    page_chars += len;
    nlines++;
    text = txt_skip(text, len);
  }
  page_next = text;
  shown = 0;
  txt_fill(1, BOX_Y + 1, 28, BOX_H - 2, UI_FILL, PAL_TEXT);
}

static void draw_frame(const Node *n) {
  txt_fill(0, 0, 30, BOX_Y, UI_BLANK, 0);
  txt_box(0, BOX_Y, 30, BOX_H);
  if (n->who) {
    txt_put(1, BOX_Y, ' ', PAL_NAVY);
    int w = txt_print(2, BOX_Y, n->who, PAL_NAVY);
    txt_put(2 + w, BOX_Y, ' ', PAL_NAVY);
  }
}

static void finish(void) {
  active = 0;
  txt_clear();
  if (done_cb) {
    void (*cb)(void) = done_cb;
    done_cb = 0;
    cb();
  }
}

static void enter_node(void) {
  while (cur < count && q[cur].type == N_FX) {
    apply(q[cur].eff, q[cur].arg);
    cur++;
  }
  if (cur >= count) {
    finish();
    return;
  }
  choosing = 0;
  choice = 0;
  draw_frame(&q[cur]);
  layout_page(q[cur].text);
}

void dlg_start(void (*on_done)(void)) {
  done_cb = on_done;
  active = 1;
  cur = 0;
  enter_node();
}

int dlg_active(void) { return active; }

static void reveal(int n) {
  while (n-- > 0 && shown < page_chars) {
    int k = shown;
    for (int l = 0; l < nlines; l++) {
      if (k < line_len[l]) {
        txt_put(TEXT_X + k, BOX_Y + 1 + l * 2, line_start[l][k], PAL_TEXT);
        break;
      }
      k -= line_len[l];
    }
    shown++;
  }
}

static void draw_choices(void) {
  const Node *n = &q[cur];
  int w = 0;
  for (int i = 0; i < n->nopt; i++) {
    int l = str_len(n->opts[i].label);
    if (l > w) w = l;
  }
  w += 4;
  int h = n->nopt + 2;
  int x = 30 - w, y = BOX_Y - h;
  txt_box(x, y, w, h);
  for (int i = 0; i < n->nopt; i++) {
    txt_put(x + 1, y + 1 + i, i == choice ? UI_CURSOR : UI_FILL, PAL_TEXT);
    txt_print(x + 2, y + 1 + i, n->opts[i].label, PAL_TEXT);
  }
}

void dlg_update(u16 pressed) {
  if (!active) return;
  Node *n = &q[cur];
  if (shown < page_chars) {
    if (pressed & (KEY_A | KEY_B)) reveal(page_chars);
    else reveal(2);
    if (shown >= page_chars && n->type == N_ASK && !*page_next) {
      choosing = 1;
      draw_choices();
    }
    return;
  }
  if (choosing) {
    if (pressed & KEY_UP) choice = choice ? choice - 1 : n->nopt - 1;
    if (pressed & KEY_DOWN) choice = choice + 1 < n->nopt ? choice + 1 : 0;
    if (pressed & KEY_B) choice = n->nopt - 1;
    if (pressed & (KEY_UP | KEY_DOWN | KEY_B)) draw_choices();
    if (pressed & KEY_A) {
      const Option *o = &n->opts[choice];
      apply(o->eff, o->arg);
      ins = cur + 1;
      if (o->then) o->then();
      cur++;
      enter_node();
    }
    return;
  }
  if ((g_frame >> 4) & 1) txt_put(27, BOX_Y + BOX_H - 2, UI_MORE, PAL_TEXT);
  else txt_put(27, BOX_Y + BOX_H - 2, UI_FILL, PAL_TEXT);
  if (pressed & (KEY_A | KEY_B)) {
    if (*page_next) {
      layout_page(page_next);
    } else {
      cur++;
      enter_node();
    }
  }
}

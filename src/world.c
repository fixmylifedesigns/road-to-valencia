// Overworld: map loading, walking, Mui following, NPCs, doors and interiors.
#include "game.h"

#define OW_SBB 28
static const s8 DX[4] = {0, 0, 1, -1};
static const s8 DY[4] = {1, -1, 0, 0};

typedef struct {
  u8 x, y, facing;
  u8 fx, fy;  // tile we are walking from
  u8 moving, step;
  u8 alt;
} Walker;

static Walker player, dog;
static int turn_wait;
static int inside = -1;  // building index when inside
static u16 spark_timer;

static u8 tile_at(int x, int y) {
  if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return T_TREE;
  return world_grid[y * MAP_W + x];
}

static int npc_at(int x, int y) {
  for (int i = 0; i < NPC_COUNT; i++)
    if (npc_pos[i * 2] == x && npc_pos[i * 2 + 1] == y) return i;
  return -1;
}

static int walkable(int x, int y) {
  u8 t = tile_at(x, y);
  if (t == T_WATER || t == T_TREE || t == T_SOLID) return 0;
  return npc_at(x, y) < 0;
}

static int building_at(int x, int y) {
  for (int i = 0; i < BUILDING_COUNT; i++) {
    const u8 *r = &building_rects[i * 6];
    if (x >= r[0] && x < r[0] + r[2] && y >= r[1] && y < r[1] + r[3]) return i;
  }
  return -1;
}

static int door_at(int x, int y) {
  for (int i = 0; i < BUILDING_COUNT; i++) {
    const u8 *r = &building_rects[i * 6];
    if (r[4] == x && r[5] == y) return i;
  }
  return -1;
}

static void load_bg(const void *tiles, unsigned tsize, const void *pal, unsigned psize, const void *map, unsigned msize, int big) {
  REG_DISPCNT &= ~DCNT_BG0;
  copy16(CHARBLOCK(0), tiles, tsize);
  copy16(MEM_PAL_BG, pal, psize);
  copy16(SCREENBLOCK(OW_SBB), map, msize);
  REG_BG0CNT = BG_CBB(0) | BG_SBB(OW_SBB) | BG_4BPP | (big ? BG_REG_64x64 : BG_REG_32x32) | BG_PRIO(1);
  REG_DISPCNT |= DCNT_BG0;
}

void world_load_overworld(void) {
  load_bg(bg_overworld_tiles, sizeof(bg_overworld_tiles), bg_overworld_pal, sizeof(bg_overworld_pal),
          bg_overworld_map, sizeof(bg_overworld_map), 1);
  inside = -1;
}

static void place(int x, int y, int facing) {
  player = (Walker){x, y, facing, x, y, 0, 0, 0};
  int bx = x - DX[facing], by = y - DY[facing];
  // Mui stands behind you, or beside you if that spot is taken.
  const s8 cand[4][2] = {{bx, by}, {x - 1, y}, {x + 1, y}, {x, y + 1}};
  dog = (Walker){x, y, facing, x, y, 0, 0, 0};
  for (int i = 0; i < 4; i++) {
    if (walkable(cand[i][0], cand[i][1]) && door_at(cand[i][0], cand[i][1]) < 0) {
      dog.x = dog.fx = cand[i][0];
      dog.y = dog.fy = cand[i][1];
      break;
    }
  }
}

void world_enter(void) {
  world_load_overworld();
  int x = g_save.x, y = g_save.y;
  if (!walkable(x, y) || door_at(x, y) >= 0) {
    x = START_X;
    y = START_Y;
  }
  place(x, y, g_save.facing & 3);
}

static int entering;
static void go_inside(void) { interior_load(entering); }

static void come_out(void) {
  const u8 *r = &building_rects[entering * 6];
  world_load_overworld();
  place(r[4], r[5] + 1, FACE_DOWN);
  g_save.x = player.x;
  g_save.y = player.y;
  g_save.facing = FACE_DOWN;
  save_write();
}

static void leave_building(void) { fade_to(come_out); }

void interior_load(int b) {
  switch (b) {
    case B_CONSULATE: load_bg(bg_in_consulate_tiles, sizeof(bg_in_consulate_tiles), bg_in_consulate_pal, sizeof(bg_in_consulate_pal), bg_in_consulate_map, sizeof(bg_in_consulate_map), 0); break;
    case B_CAFE: load_bg(bg_in_cafe_tiles, sizeof(bg_in_cafe_tiles), bg_in_cafe_pal, sizeof(bg_in_cafe_pal), bg_in_cafe_map, sizeof(bg_in_cafe_map), 0); break;
    case B_HOME: load_bg(bg_in_home_tiles, sizeof(bg_in_home_tiles), bg_in_home_pal, sizeof(bg_in_home_pal), bg_in_home_map, sizeof(bg_in_home_map), 0); break;
    case B_PRINTS: load_bg(bg_in_prints_tiles, sizeof(bg_in_prints_tiles), bg_in_prints_pal, sizeof(bg_in_prints_pal), bg_in_prints_map, sizeof(bg_in_prints_map), 0); break;
    default: load_bg(bg_in_station_tiles, sizeof(bg_in_station_tiles), bg_in_station_pal, sizeof(bg_in_station_pal), bg_in_station_map, sizeof(bg_in_station_map), 0); break;
  }
  REG_BG0HOFS = 0;
  REG_BG0VOFS = 0;
  inside = b;
  dlg_begin();
  script_building(b);
  dlg_start(leave_building);
}

static void start_step(int dir) {
  int nx = player.x + DX[dir], ny = player.y + DY[dir];
  if (!walkable(nx, ny)) return;
  dog.fx = dog.x;
  dog.fy = dog.y;
  if (dog.x != player.x || dog.y != player.y) {
    dog.facing = player.x > dog.x ? FACE_RIGHT : player.x < dog.x ? FACE_LEFT : player.y > dog.y ? FACE_DOWN : FACE_UP;
  }
  dog.x = player.x;
  dog.y = player.y;
  player.fx = player.x;
  player.fy = player.y;
  player.x = nx;
  player.y = ny;
  player.moving = 1;
  player.step = 0;
  player.alt ^= 1;
}

static void arrive(void) {
  player.moving = 0;
  g_save.x = player.x;
  g_save.y = player.y;
  g_save.facing = player.facing;
  int d = door_at(player.x, player.y);
  if (d >= 0) {
    entering = d;
    fade_to(go_inside);
    return;
  }
  save_write();
}

static void interact(void) {
  int fx = player.x + DX[player.facing], fy = player.y + DY[player.facing];
  int n = npc_at(fx, fy);
  dlg_begin();
  if (n >= 0) {
    script_npc(n);
  } else {
    int b = building_at(fx, fy);
    if (b >= 0 && door_at(fx, fy) < 0) script_sign(b);
    else script_look(tile_at(fx, fy));
  }
  dlg_start(0);
}

int world_facing_door(void) {
  if (inside >= 0 || player.moving || player.facing != FACE_UP) return -1;
  return door_at(player.x, player.y - 1);
}

void world_update(u16 keys, u16 pressed) {
  if (inside >= 0) return;
  // Canal shimmer: swap the two sparkle colours now and then.
  if (++spark_timer >= 20) {
    spark_timer = 0;
    static int flip;
    flip ^= 1;
    for (int i = 0; i < OW_SPARK_A_COUNT; i++) MEM_PAL_BG[ow_spark_a[i]] = flip ? SPARK_B_COLOR : SPARK_A_COLOR;
    for (int i = 0; i < OW_SPARK_B_COUNT; i++) MEM_PAL_BG[ow_spark_b[i]] = flip ? SPARK_A_COLOR : SPARK_B_COLOR;
  }
  if (player.moving) {
    int speed = (keys & KEY_B) ? 4 : 2;
    player.step += speed;
    if (player.step >= 16) arrive();
    return;
  }
  if (pressed & KEY_A) {
    interact();
    return;
  }
  int dir = -1;
  if (keys & KEY_DOWN) dir = FACE_DOWN;
  else if (keys & KEY_UP) dir = FACE_UP;
  else if (keys & KEY_RIGHT) dir = FACE_RIGHT;
  else if (keys & KEY_LEFT) dir = FACE_LEFT;
  if (dir < 0) {
    turn_wait = 0;
    return;
  }
  if (dir != player.facing) {
    player.facing = dir;
    turn_wait = (pressed & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) ? 4 : 0;
    if (turn_wait) return;
  }
  if (turn_wait > 0) {
    turn_wait--;
    return;
  }
  start_step(dir);
}

static int person_tile(int who) { return who == CH_MOENO ? OBJ_MOENO : OBJ_IRVING; }
static int person_pal(int who) { return who == CH_MOENO ? OBJPAL_MOENO : OBJPAL_IRVING; }

void world_draw(void) {
  oam_clear();
  if (inside >= 0) {
    int px = IN_CAFE_PX, py = IN_CAFE_PY, face = IN_CAFE_FACE;
    if (inside == B_HOME) {
      px = IN_HOME_PX;
      py = IN_HOME_PY;
      face = IN_HOME_FACE;
    }
    spr_person(0, person_tile(g_save.character) + face * 3 * 8, person_pal(g_save.character), px, py);
    return;
  }
  int t = player.moving ? player.step : 16;
  int ppx = player.fx * 16 + (player.x - player.fx) * t;
  int ppy = player.fy * 16 + (player.y - player.fy) * t;
  int dpx = dog.fx * 16 + (dog.x - dog.fx) * t;
  int dpy = dog.fy * 16 + (dog.y - dog.fy) * t;
  if (!player.moving) {
    dpx = dog.x * 16;
    dpy = dog.y * 16;
  }

  int cx = ppx + 8 - 120, cy = ppy + 8 - 80;
  if (cx < 0) cx = 0;
  if (cy < 0) cy = 0;
  if (cx > MAP_W * 16 - 240) cx = MAP_W * 16 - 240;
  if (cy > MAP_H * 16 - 160) cy = MAP_H * 16 - 160;
  REG_BG0HOFS = cx;
  REG_BG0VOFS = cy;

  // Draw order: things lower on screen go in front (lower OAM index = on top).
  struct { int y, kind, idx; } list[8];
  int n = 0;
  list[n++] = (typeof(list[0])){ppy, 0, 0};
  list[n++] = (typeof(list[0])){dpy, 1, 0};
  for (int i = 0; i < NPC_COUNT; i++) list[n++] = (typeof(list[0])){npc_pos[i * 2 + 1] * 16, 2, i};
  for (int i = 1; i < n; i++)
    for (int j = i; j > 0 && list[j].y > list[j - 1].y; j--) {
      typeof(list[0]) tmp = list[j];
      list[j] = list[j - 1];
      list[j - 1] = tmp;
    }
  for (int i = 0; i < n; i++) {
    if (list[i].kind == 0) {
      int frame = 0;
      if (player.moving && player.step < 8) frame = player.alt ? 1 : 2;
      spr_person(i, person_tile(g_save.character) + (player.facing * 3 + frame) * 8, person_pal(g_save.character), ppx - cx, ppy - cy - 16);
    } else if (list[i].kind == 1) {
      int hop = player.moving && player.step < 8;
      int x = dpx - cx, y = dpy - cy;
      if (x <= -16 || x >= 240 || y <= -16 || y >= 160) oam_set(i, ATTR0_HIDE, 0, 0);
      else oam_set(i, (y & 0xFF) | ATTR0_SQUARE, (x & 0x1FF) | ATTR1_SIZE(1), (OBJ_MUI + (dog.facing * 2 + hop) * 4) | (1 << 10) | (OBJPAL_MUI << 12));
    } else {
      int k = list[i].idx;
      spr_person(i, npc_tile[k], npc_pal[k], npc_pos[k * 2] * 16 - cx, npc_pos[k * 2 + 1] * 16 - cy - 16);
    }
  }
}

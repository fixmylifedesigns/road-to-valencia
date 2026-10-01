// The real-life checklist. Tasks appear or disappear depending on the routes chosen.
#include "game.h"

enum { C_ALWAYS, C_CA, C_PAREJA, C_OWN, C_FBI_OSAKA, C_FBI_LATER };

#define SAVE_MAGIC 0x52545631u  // "RTV1"

Save g_save;

const char *const STAGE_NAMES[STAGE_COUNT] = {"Osaka", "Paperwork", "Visa application", "Valencia"};
const char *const STAGE_BLURBS[STAGE_COUNT] = {
    "Errands before leaving Japan",
    "Documents, apostilles, translations",
    "Submitting the Digital Nomad Visa",
    "Landing and settling in",
};
const char *const PARTNER_LABELS[4] = {"Not decided", "California partnership", "Pareja de hecho", "Moeno's own visa"};
const char *const FBI_LABELS[3] = {"Not decided", "Prints in Osaka", "Prints later"};

const Task TASKS[TASK_COUNT] = {
    [TK_MEETING] = {ST_OSAKA, WHO_IRVING, C_ALWAYS, "Join the call with the company and the lawyer", "Cafe Nomado or home"},
    [TK_COC_REQUEST] = {ST_OSAKA, WHO_IRVING, C_ALWAYS, "Company submits the social security certificate of coverage request", "In the call"},
    [TK_CHOOSE_PARTNER] = {ST_OSAKA, WHO_BOTH, C_ALWAYS, "Decide how Moeno is coming", "In the call"},
    [TK_CHOOSE_FBI] = {ST_OSAKA, WHO_IRVING, C_ALWAYS, "Decide where to get FBI fingerprints", "Call or fingerprint office"},
    [TK_NOTARY] = {ST_OSAKA, WHO_BOTH, C_CA, "Sign the California partnership declaration at the consulate notary", "U.S. Consulate"},
    [TK_FBI_PRINTS] = {ST_OSAKA, WHO_IRVING, C_FBI_OSAKA, "Get FBI fingerprints taken", "Fingerprint Service"},

    [TK_FBI_PRINTS_LATER] = {ST_PAPERWORK, WHO_IRVING, C_FBI_LATER, "Get FBI fingerprints taken outside Osaka", 0},
    [TK_FBI_CHECK] = {ST_PAPERWORK, WHO_IRVING, C_ALWAYS, "Receive the FBI background check", 0},
    [TK_FBI_APOSTILLE] = {ST_PAPERWORK, WHO_IRVING, C_ALWAYS, "Apostille the FBI background check", 0},
    [TK_COC_RECEIVED] = {ST_PAPERWORK, WHO_IRVING, C_ALWAYS, "Receive the certificate of coverage", 0},
    [TK_CA_FILE] = {ST_PAPERWORK, WHO_BOTH, C_CA, "Mail the notarized declaration to the California Secretary of State", 0},
    [TK_CA_APOSTILLE] = {ST_PAPERWORK, WHO_BOTH, C_CA, "Apostille the registered partnership certificate", 0},
    [TK_TRANSLATIONS] = {ST_PAPERWORK, WHO_BOTH, C_ALWAYS, "Sworn Spanish translations of the documents", 0},
    [TK_INSURANCE] = {ST_PAPERWORK, WHO_BOTH, C_ALWAYS, "Private Spanish health insurance", 0},
    [TK_MOENO_VISA_PICK] = {ST_PAPERWORK, WHO_MOENO, C_OWN, "Pick Moeno's own visa type", 0},
    [TK_MUI_CHIP] = {ST_PAPERWORK, WHO_MOENO, C_ALWAYS, "Mui: microchip and rabies vaccination up to date", 0},
    [TK_MUI_CERT] = {ST_PAPERWORK, WHO_MOENO, C_ALWAYS, "Mui: EU health certificate endorsed before the flight", 0},

    [TK_SUBMIT_DNV] = {ST_APPLY, WHO_IRVING, C_ALWAYS, "Submit the Digital Nomad Visa application", 0},
    [TK_MOENO_FAMILY] = {ST_APPLY, WHO_MOENO, C_CA, "Moeno applies as a family member", 0},
    [TK_MOENO_OWN_APPLY] = {ST_APPLY, WHO_MOENO, C_OWN, "Moeno submits her own visa application", 0},
    [TK_DNV_APPROVED] = {ST_APPLY, WHO_IRVING, C_ALWAYS, "Digital Nomad Visa approved", 0},
    [TK_MUI_FLIGHT] = {ST_APPLY, WHO_MOENO, C_ALWAYS, "Book a pet-friendly flight for Mui", 0},

    [TK_FLAT] = {ST_VALENCIA, WHO_BOTH, C_ALWAYS, "Sign the lease on our Valencia flat", 0},
    [TK_PADRON] = {ST_VALENCIA, WHO_BOTH, C_ALWAYS, "Register at the padron (empadronamiento)", 0},
    [TK_TIE] = {ST_VALENCIA, WHO_BOTH, C_ALWAYS, "Get the TIE cards", 0},
    [TK_PAREJA_REGISTER] = {ST_VALENCIA, WHO_BOTH, C_PAREJA, "Register as pareja de hecho in Valencia", 0},
    [TK_MOENO_FAMILY_SPAIN] = {ST_VALENCIA, WHO_MOENO, C_PAREJA, "Moeno applies for residence as family from Spain", 0},
    [TK_MUI_HOME] = {ST_VALENCIA, WHO_MOENO, C_ALWAYS, "Mui lands in Valencia", 0},
};

int task_active(int t) {
  switch (TASKS[t].cond) {
    case C_CA: return g_save.partner == P_CA;
    case C_PAREJA: return g_save.partner == P_PAREJA;
    case C_OWN: return g_save.partner == P_OWN;
    case C_FBI_OSAKA: return g_save.fbi == F_OSAKA;
    case C_FBI_LATER: return g_save.fbi == F_LATER;
    default: return 1;
  }
}

int task_done(int t) { return (g_save.done >> t) & 1; }

void task_set(int t, int done) {
  if (done) g_save.done |= 1u << t;
  else g_save.done &= ~(1u << t);
}

void set_partner(int p) {
  g_save.partner = p;
  task_set(TK_CHOOSE_PARTNER, p != P_NONE);
}

void set_fbi(int f) {
  g_save.fbi = f;
  task_set(TK_CHOOSE_FBI, f != F_NONE);
}

void progress(int stage, int *done, int *total) {
  *done = *total = 0;
  for (int t = 0; t < TASK_COUNT; t++) {
    if ((stage >= 0 && TASKS[t].stage != stage) || !task_active(t)) continue;
    (*total)++;
    if (task_done(t)) (*done)++;
  }
}

int percent(int done, int total) { return total ? (int)udiv((u32)done * 100, (u32)total) : 0; }

int osaka_complete(void) {
  int d, t;
  progress(ST_OSAKA, &d, &t);
  return d == t;
}

// ---------- battery save (SRAM, byte access only) ----------
// Emulators and flash carts look for this string to pick SRAM saving.
const char save_type_tag[] __attribute__((aligned(4))) = "SRAM_V113";

void save_reset(void) {
  memset(&g_save, 0, sizeof(g_save));
  g_save.magic = SAVE_MAGIC;
  g_save.version = 1;
  g_save.x = START_X;
  g_save.y = START_Y;
}

int save_load(void) {
  volatile const char *tag = save_type_tag;
  (void)tag[0];
  u8 *dst = (u8 *)&g_save;
  for (unsigned i = 0; i < sizeof(Save); i++) dst[i] = SRAM[i];
  if (g_save.magic != SAVE_MAGIC || g_save.version != 1) {
    save_reset();
    return 0;
  }
  return 1;
}

void save_write(void) {
  const u8 *src = (const u8 *)&g_save;
  for (unsigned i = 0; i < sizeof(Save); i++) SRAM[i] = src[i];
}

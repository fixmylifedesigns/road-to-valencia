// Dialogue for each place in Osaka.
#include "game.h"

#define LAWYER "Lawyer"
#define MANAGER "Manager"

// ---------- the call with the company and the lawyer ----------
static void coc_yes(void) {
  dlg_say(MANAGER, "Submitted! The certificate keeps you on U.S. social security while you work from Spain. It can take a few weeks to arrive.");
}
static void coc_no(void) { dlg_say(MANAGER, "Okay. Ping us when you're ready and we'll file it the same day."); }

static void coc_question(void) {
  Option *o = dlg_options(2);
  o[0] = (Option){"Yes, submit it", EFF_DONE, TK_COC_REQUEST, coc_yes};
  o[1] = (Option){"Not yet", EFF_NONE, 0, coc_no};
  dlg_ask(MANAGER, "Ask the company to submit the certificate of coverage request?", o, 2);
}

static void partner_ca(void) {
  dlg_say(LAWYER, "California domestic partnership it is. You both sign the declaration with the consular notary here in Osaka, mail it to Sacramento, then apostille the certificate.");
}
static void partner_pareja(void) {
  dlg_say(LAWYER, "Pareja de hecho, then. You move first, register together with the Valencian registry, and Moeno applies for residence as family from inside Spain.");
}
static void partner_own(void) {
  dlg_say(LAWYER, "Moeno takes her own visa route. Less partnership paperwork, but a separate application to prepare.");
}
static void partner_later(void) { dlg_say(LAWYER, "No rush. We can pick this up on the next call."); }

static void partner_question(void) {
  Option *o = dlg_options(4);
  o[0] = (Option){"CA partnership", EFF_PARTNER, P_CA, partner_ca};
  o[1] = (Option){"Pareja de hecho", EFF_PARTNER, P_PAREJA, partner_pareja};
  o[2] = (Option){"Her own visa", EFF_PARTNER, P_OWN, partner_own};
  o[3] = (Option){"Still deciding", EFF_NONE, 0, partner_later};
  dlg_ask(LAWYER, "How is Moeno coming to Spain?", o, 4);
}

static void fbi_osaka(void) { dlg_say(LAWYER, "Quick and done before you fly. The fingerprint office is south of the canal."); }
static void fbi_later(void) { dlg_say(LAWYER, "Fine, just leave time for the FBI results and the apostille afterwards."); }
static void fbi_unsure(void) { dlg_say(LAWYER, "Let me know soon. The background check is the slowest piece."); }

static void fbi_question(void) {
  Option *o = dlg_options(3);
  o[0] = (Option){"Osaka (pricey)", EFF_FBI, F_OSAKA, fbi_osaka};
  o[1] = (Option){"Cheaper, later", EFF_FBI, F_LATER, fbi_later};
  o[2] = (Option){"Not sure yet", EFF_NONE, 0, fbi_unsure};
  dlg_ask(LAWYER, "Where will you get the FBI fingerprints done?", o, 3);
}

static void leave_call(void) { dlg_say(LAWYER, "Talk soon!"); }

static void meeting(int home) {
  if (home) {
    dlg_say(0, "Home sweet home. You open the laptop on the kotatsu and join the call.");
  } else {
    dlg_say("Barista", "Irasshaimase! Grab any seat, the Wi-Fi password is on the cup.");
    dlg_say(0, "You order an iced coffee and join the call.");
  }
  if (!task_done(TK_MEETING)) {
    dlg_say(LAWYER, "Morning, Irving! Let's map out the Digital Nomad Visa for Spain.");
    dlg_say(MANAGER, "Hi from the company side. First thing: the social security certificate of coverage.");
    coc_question();
    partner_question();
    fbi_question();
    dlg_fx(EFF_DONE, TK_MEETING);
    dlg_say(LAWYER, "Great call. Press SELECT any time to open your journal and see what's left.");
    return;
  }
  int n = task_done(TK_COC_REQUEST) ? 3 : 4;
  Option *o = dlg_options(n);
  int i = 0;
  if (n == 4) o[i++] = (Option){"Certificate request", EFF_NONE, 0, coc_question};
  o[i++] = (Option){"How Moeno comes", EFF_NONE, 0, partner_question};
  o[i++] = (Option){"Fingerprint plan", EFF_NONE, 0, fbi_question};
  o[i++] = (Option){"Leave the call", EFF_NONE, 0, leave_call};
  dlg_say(LAWYER, "Back again? What do you want to go over?");
  dlg_ask(LAWYER, "Pick a topic.", o, n);
}

// ---------- U.S. Consulate ----------
static void notary_signed(void) {
  dlg_say(0, "Moeno signs. You sign. The notary stamps both signatures.");
  dlg_say("Notary", "All done. Mail the original to Sacramento and keep a copy for yourselves.");
}
static void notary_later(void) { dlg_say("Notary", "Bring both passports when you come back."); }

static void consulate(void) {
  dlg_say("Guard", "Welcome to the U.S. Consulate General Osaka-Kobe. Appointments only, please.");
  if (g_save.partner != P_CA) {
    dlg_say("Guard", "I don't see a notary appointment for you today.");
    dlg_say(0, "(You need one for the California partnership route. Pick it on the call with your lawyer.)");
    return;
  }
  if (task_done(TK_NOTARY)) {
    dlg_say("Notary", "Your declaration is already notarized. Next stop: the California Secretary of State.");
    return;
  }
  dlg_say("Notary", "Notarial services, window 2. I need both of your passports.");
  Option *o = dlg_options(2);
  o[0] = (Option){"Sign it", EFF_DONE, TK_NOTARY, notary_signed};
  o[1] = (Option){"Come back later", EFF_NONE, 0, notary_later};
  dlg_ask("Notary", "Sign the Declaration of Domestic Partnership now?", o, 2);
}

// ---------- Fingerprint office ----------
static void prints_taken(void) { dlg_say("Clerk", "Roll, press, lift... all ten fingers. Your cards are ready to mail to the FBI."); }
static void prints_elsewhere(void) { dlg_say("Clerk", "No problem. Leave time for the results and the apostille."); }
static void prints_bye(void) { dlg_say("Clerk", "Come back any time."); }
static void prints_stick(void) { dlg_say("Clerk", "Understood. Good luck!"); }

static void prints(void) {
  dlg_say("Clerk", "Fingerprint Service Osaka. We roll FBI-format fingerprint cards.");
  if (task_done(TK_FBI_PRINTS)) {
    dlg_say("Clerk", "Your cards are done. Send them to the FBI and watch for the background check result.");
    return;
  }
  if (g_save.fbi == F_LATER) {
    Option *o = dlg_options(2);
    o[0] = (Option){"Do them here", EFF_PRINTS_HERE, 0, prints_taken};
    o[1] = (Option){"Stick to the plan", EFF_NONE, 0, prints_stick};
    dlg_ask("Clerk", "You planned to do the prints elsewhere. Do them here after all?", o, 2);
    return;
  }
  dlg_say("Clerk", "It's on the expensive side, I have to warn you.");
  Option *o = dlg_options(3);
  o[0] = (Option){"Yes, take them", EFF_PRINTS_HERE, 0, prints_taken};
  o[1] = (Option){"Elsewhere", EFF_FBI, F_LATER, prints_elsewhere};
  o[2] = (Option){"Just looking", EFF_NONE, 0, prints_bye};
  dlg_ask("Clerk", "Get your FBI fingerprints taken here?", o, 3);
}

// ---------- Osaka Station ----------
static void station(void) {
  if (osaka_complete()) {
    dlg_say("Station staff", "The limited express to Kansai Airport is boarding!");
    dlg_say(0, "Osaka is done. The Paperwork region opens in a future update.");
    dlg_say(0, "Keep ticking off real-life steps in your journal until then.");
    return;
  }
  int left = 0;
  for (int t = 0; t < TASK_COUNT; t++)
    if (TASKS[t].stage == ST_OSAKA && task_active(t) && !task_done(t)) left++;
  dlg_say("Station staff", "Trains to Kansai Airport leave from here.");
  dlg_say(0, dlg_str("You still have ", left, left == 1 ? " Osaka task left:" : " Osaka tasks left:"));
  int shown = 0;
  for (int t = 0; t < TASK_COUNT && shown < 3; t++) {
    if (TASKS[t].stage == ST_OSAKA && task_active(t) && !task_done(t)) {
      dlg_say(0, TASKS[t].title);
      shown++;
    }
  }
}

void script_building(int b) {
  if (b == B_STATION) {
    station();
    return;
  }
  if (g_save.character == CH_MOENO) {
    if (b == B_HOME) dlg_say(0, "Home. Mui's favourite spot is the sunny corner by the window.");
    if (b == B_CONSULATE && g_save.partner == P_CA && !task_done(TK_NOTARY))
      dlg_say("Guard", "Welcome! Your notary appointment is with Irving. Come back together.");
    else if (b != B_HOME)
      dlg_say(0, "This stop is one of Irving's errands.");
    dlg_say(0, "Moeno's Osaka missions arrive in the next update. Switch to Irving from the START menu to play this one.");
    return;
  }
  if (b == B_CAFE) meeting(0);
  else if (b == B_HOME) meeting(1);
  else if (b == B_CONSULATE) consulate();
  else if (b == B_PRINTS) prints();
}

void script_intro(void) {
  dlg_say(0, "Osaka, Japan.");
  dlg_say(0, "Irving, Moeno and Mui are moving to Valencia!");
  if (g_save.character == CH_MOENO) {
    dlg_say(0, "Moeno's own Osaka missions are coming soon. For now you can explore the city with Mui.");
  } else {
    dlg_say(0, "Three stops in Osaka: the U.S. Consulate, the fingerprint office south of the canal, and a call with the company and the lawyer.");
    dlg_say(0, "Take the call at Cafe Nomado or from home.");
  }
  dlg_say(0, "START opens the menu. SELECT opens your journal and the full checklist.");
  dlg_fx(EFF_INTRO_SEEN, 0);
}

void script_npc(int n) {
  switch (n) {
    case NPC_SALARYMAN:
      dlg_say(0, "Social security certificate? My company files ours through HR too.");
      dlg_say(0, "It stops you paying into two systems at once. Worth chasing!");
      break;
    case NPC_OBAACHAN:
      dlg_say(0, "Spain? Ara, how exciting.");
      dlg_say(0, "Bring me back some jamon. And eat properly over there!");
      break;
    case NPC_TRAVELER:
      dlg_say(0, "Trains to Kansai Airport leave from Osaka Station.");
      dlg_say(0, "Finish your Osaka list first. The airport won't wait for paperwork.");
      break;
    default:
      dlg_say(0, "Irasshai! Hot takoyaki!");
      dlg_say(0, "Careful, the inside is lava.");
  }
}

void script_sign(int b) {
  switch (b) {
    case B_CASTLE: dlg_say(0, "Osaka Castle. Tall walls, taller paperwork piles back home."); break;
    case B_TOWER: dlg_say(0, "An old steel tower. From the top you can almost see Valencia. Almost."); break;
    case B_TAKOYAKI: dlg_say(0, "Takoyaki, 8 pieces. Fuel for paperwork."); break;
    case B_CONSULATE: dlg_say(0, "U.S. Consulate General Osaka-Kobe. The entrance is round the front."); break;
    case B_STATION: dlg_say(0, "Osaka Station. The entrance is round the front."); break;
    case B_CAFE: dlg_say(0, "Cafe Nomado. Good Wi-Fi, better coffee."); break;
    case B_PRINTS: dlg_say(0, "Fingerprint Service Osaka. FBI-format cards rolled here."); break;
    default: dlg_say(0, "Home. The entrance is round the front.");
  }
}

void script_look(int tile) {
  if (tile == T_WATER) dlg_say(0, "The canal shimmers with neon reflections.");
  else if (tile == T_TREE) dlg_say(0, "A cicada is yelling in this tree.");
}

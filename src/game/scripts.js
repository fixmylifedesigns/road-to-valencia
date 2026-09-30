// Dialogue for each place. A script is a list of nodes:
//   say(who, text)                         show a line
//   ask(text, [{ label, fx, then }])       show a choice; fx(save) returns the new save, then are the follow-up nodes
//   fx(fn)                                 change the save without showing anything
import { ROUTES, osakaComplete, activeTasks, withDone, withRoute } from "./quests.js";

export const say = (who, text) => ({ t: "say", who, text });
export const ask = (text, options, who = null) => ({ t: "ask", text, options, who });
export const fx = (run) => ({ t: "fx", run });

const PAGE = 96; // characters that fit in the three-line text box

// Splits long lines into several text boxes, breaking between sentences where possible.
export function paginate(nodes) {
  const out = [];
  for (const n of nodes) {
    if (n.t !== "say" || n.text.length <= PAGE) {
      out.push(n);
      continue;
    }
    const pieces = n.text.match(/[^.!?]+[.!?]+["')]*\s*|[^.!?]+$/g) || [n.text];
    let page = "";
    const flush = () => {
      if (page.trim()) out.push({ ...n, text: page.trim() });
      page = "";
    };
    for (const piece of pieces) {
      if ((page + piece).trim().length <= PAGE) {
        page += piece;
        continue;
      }
      if (piece.length <= PAGE) {
        flush();
        page = piece;
        continue;
      }
      for (const word of piece.split(" ")) {
        const next = `${page.trimEnd()} ${word}`.trim();
        if (next.length > PAGE) {
          flush();
          page = word;
        } else page = next;
      }
    }
    flush();
  }
  return out;
}

const partnerExplain = {
  "ca-partnership":
    "California domestic partnership it is. You'll both sign the declaration in front of the consular notary here in Osaka, mail it to Sacramento, then apostille the certificate.",
  pareja:
    "Pareja de hecho, then. You move first, register together with the Valencian registry, and Moeno applies for residence as family from inside Spain.",
  "own-visa": "Moeno takes her own visa route. Less partnership paperwork, but a separate application to prepare.",
};

function partnerQuestion() {
  return ask(
    "How is Moeno coming to Spain?",
    [
      ...ROUTES.partner.options.map((o) => ({
        label: o.label,
        fx: (s) => withRoute(s, "partner", o.id),
        then: [say("Lawyer", partnerExplain[o.id])],
      })),
      { label: "Still deciding", then: [say("Lawyer", "No rush. We can pick this up on the next call.")] },
    ],
    "Lawyer"
  );
}

function fbiQuestion() {
  return ask(
    "Where will you get the FBI fingerprints done?",
    [
      {
        label: "Here in Osaka (pricey)",
        fx: (s) => withRoute(s, "fbi", "osaka"),
        then: [say("Lawyer", "Quick and done before you fly. Head to the fingerprint office down by the canal.")],
      },
      {
        label: "Somewhere cheaper, later",
        fx: (s) => withRoute(s, "fbi", "later"),
        then: [say("Lawyer", "Fine, just leave time for the FBI results and the apostille afterwards.")],
      },
      { label: "Not sure yet", then: [say("Lawyer", "Let me know soon, the background check is the slowest piece.")] },
    ],
    "Lawyer"
  );
}

function cocQuestion() {
  return ask(
    "Ask the company to submit the certificate of coverage request?",
    [
      {
        label: "Yes, submit it",
        fx: (s) => withDone(s, "coc-request"),
        then: [say("Manager", "Submitted! The certificate keeps you on U.S. social security while you work from Spain. It can take a few weeks to arrive.")],
      },
      { label: "Not yet", then: [say("Manager", "Okay. Ping us when you're ready and we'll file it the same day.")] },
    ],
    "Manager"
  );
}

function meeting(save, place) {
  const intro =
    place === "home"
      ? [say(null, "Home sweet home. You open the laptop on the kotatsu and join the call.")]
      : [say("Barista", "Irasshaimase! Grab any seat, the Wi-Fi password is on the cup."), say(null, "You order an iced coffee and join the call.")];

  if (!save.done.meeting) {
    return [
      ...intro,
      say("Lawyer", "Morning, Irving! Let's map out the Digital Nomad Visa for Spain."),
      say("Manager", "Hi from the company side. First thing: the social security certificate of coverage."),
      cocQuestion(),
      partnerQuestion(),
      fbiQuestion(),
      fx((s) => withDone(s, "meeting")),
      say("Lawyer", "Great call. Open your journal with START any time to see what's left."),
    ];
  }

  const options = [];
  if (!save.done["coc-request"]) options.push({ label: "Submit the certificate request", then: [cocQuestion()] });
  options.push({ label: "Change how Moeno comes", then: [partnerQuestion()] });
  options.push({ label: "Change the fingerprint plan", then: [fbiQuestion()] });
  options.push({ label: "Leave the call", then: [say("Lawyer", "Talk soon!")] });

  return [...intro, say("Lawyer", "Back again? What do you want to go over?"), ask("Pick a topic", options, "Lawyer")];
}

function consulate(save) {
  const lines = [say("Guard", "Welcome to the U.S. Consulate General Osaka-Kobe. Appointments only, please.")];
  if (save.routes.partner !== "ca-partnership") {
    return [
      ...lines,
      say("Guard", "I don't see a notary appointment for you today."),
      say(null, "(You'd need one for the California partnership route. Choose it on the call with your lawyer.)"),
    ];
  }
  if (save.done.notary) {
    return [...lines, say("Notary", "Your declaration is already notarized. Next stop: the California Secretary of State.")];
  }
  return [
    ...lines,
    say("Notary", "Notarial services, window 2. Both of you need your passports."),
    ask(
      "Sign the Declaration of Domestic Partnership now?",
      [
        {
          label: "Sign it",
          fx: (s) => withDone(s, "notary"),
          then: [
            say(null, "Moeno signs. You sign. The notary stamps both signatures."),
            say("Notary", "All done. Mail the original to Sacramento, and keep a copy for yourselves."),
          ],
        },
        { label: "Come back later", then: [say("Notary", "Bring both passports when you come back.")] },
      ],
      "Notary"
    ),
  ];
}

function prints(save) {
  const lines = [say("Clerk", "Fingerprint Service Osaka. We roll FBI-format fingerprint cards.")];
  if (save.done["fbi-prints"]) {
    return [...lines, say("Clerk", "Your cards are ready. Send them to the FBI and watch for the background check result.")];
  }
  if (save.routes.fbi === "later") {
    return [
      ...lines,
      ask(
        "You planned to do the prints elsewhere. Do them here after all?",
        [
          {
            label: "Yes, do them here",
            fx: (s) => withDone(withRoute(s, "fbi", "osaka"), "fbi-prints"),
            then: [say("Clerk", "Roll, press, lift... done. Pricey, but that's one less thing to do later.")],
          },
          { label: "No, stick to the plan", then: [say("Clerk", "Understood. Good luck!")] },
        ],
        "Clerk"
      ),
    ];
  }
  return [
    ...lines,
    say("Clerk", "It is on the expensive side, I have to warn you."),
    ask(
      "Get your FBI fingerprints taken here?",
      [
        {
          label: "Yes, take them",
          fx: (s) => withDone(withRoute(s, "fbi", "osaka"), "fbi-prints"),
          then: [say("Clerk", "Roll, press, lift... all ten fingers. Your cards are ready to mail to the FBI.")],
        },
        {
          label: "I'll do them elsewhere",
          fx: (s) => withRoute(s, "fbi", "later"),
          then: [say("Clerk", "No problem. Leave time for the results and the apostille.")],
        },
        { label: "Just looking", then: [say("Clerk", "Come back any time.")] },
      ],
      "Clerk"
    ),
  ];
}

function station(save) {
  if (osakaComplete(save)) {
    return [
      say("Station staff", "The limited express to Kansai Airport is boarding!"),
      say(null, "Osaka is done. The Paperwork region opens in a future update."),
      say(null, "Keep ticking off real-life steps in your journal until then."),
    ];
  }
  const left = activeTasks(save, "osaka").filter((t) => !save.done[t.id]);
  return [
    say("Station staff", "Trains to Kansai Airport leave from here."),
    say(null, `You still have ${left.length} Osaka ${left.length === 1 ? "task" : "tasks"} left:`),
    ...left.slice(0, 3).map((t) => say(null, `${t.title}.`)),
  ];
}

const MOENO_SOON = [
  say(null, "This stop is one of Irving's errands."),
  say(null, "Moeno's Osaka missions arrive in the next update. Switch to Irving from the START menu to play this one."),
];

export function buildingScript(id, save) {
  const isMoeno = save.character === "moeno";
  if (id === "station") return station(save);
  if (isMoeno) {
    if (id === "home") return [say(null, "Home. Mui's favourite spot is the sunny corner by the window."), ...MOENO_SOON];
    if (id === "consulate" && save.routes.partner === "ca-partnership" && !save.done.notary) {
      return [say("Guard", "Welcome! Your notary appointment is with Irving. Come back together."), ...MOENO_SOON.slice(1)];
    }
    return MOENO_SOON;
  }
  if (id === "cafe" || id === "home") return meeting(save, id);
  if (id === "consulate") return consulate(save);
  if (id === "prints") return prints(save);
  return [say(null, "The door is locked.")];
}

export function introScript(save) {
  const lines = [say(null, "Osaka, Japan."), say(null, "Irving, Moeno and Mui are moving to Valencia!")];
  if (save.character === "moeno") {
    lines.push(say(null, "Moeno's own Osaka missions are coming soon. For now you can explore the city with Mui."));
  } else {
    lines.push(
      say(null, "Three stops in Osaka: the U.S. Consulate, the fingerprint office by the canal, and a call with the company and the lawyer."),
      say(null, "Take the call at Cafe Nomado or from home.")
    );
  }
  lines.push(say(null, "Press START for your journal and the full checklist."));
  return lines;
}

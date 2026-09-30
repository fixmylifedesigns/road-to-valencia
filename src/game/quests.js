// The real-life checklist behind the game. Tasks appear or disappear depending on the routes chosen.

export const STAGES = [
  { id: "osaka", name: "Osaka", blurb: "Errands before leaving Japan", playable: true },
  { id: "paperwork", name: "Paperwork", blurb: "Documents, apostilles and translations", playable: false },
  { id: "apply", name: "Visa application", blurb: "Submitting the Digital Nomad Visa", playable: false },
  { id: "valencia", name: "Valencia", blurb: "Landing, registering and settling in", playable: false },
];

export const ROUTES = {
  partner: {
    title: "Bringing Moeno",
    options: [
      {
        id: "ca-partnership",
        label: "California domestic partnership",
        detail:
          "Register the partnership in California (both sign in front of a notary), apostille the certificate, then Moeno applies as a family member of the Digital Nomad Visa holder.",
      },
      {
        id: "pareja",
        label: "Pareja de hecho in Spain",
        detail:
          "Move first, then register as a pareja de hecho with the Valencian registry and apply for Moeno's residence as family from inside Spain.",
      },
      {
        id: "own-visa",
        label: "Moeno gets her own visa",
        detail: "Skip the partnership paperwork and have Moeno apply on her own visa route.",
      },
    ],
  },
  fbi: {
    title: "FBI fingerprints",
    options: [
      { id: "osaka", label: "Take them in Osaka", detail: "Fast and done before leaving Japan, but expensive." },
      { id: "later", label: "Take them somewhere else", detail: "Cheaper, but it has to fit into the plan later." },
    ],
  },
};

const always = () => true;
const partnerIs = (id) => (r) => r.partner === id;
const fbiIs = (id) => (r) => r.fbi === id;

export const TASKS = [
  // Osaka
  { id: "meeting", stage: "osaka", who: "irving", title: "Join the call with the company and the lawyer", where: "at Cafe Nomado or from home", when: always },
  { id: "coc-request", stage: "osaka", who: "irving", title: "Company submits the social security certificate of coverage request", where: "in the call", when: always },
  { id: "choose-partner", stage: "osaka", who: "both", title: "Decide how Moeno is coming", where: "in the call", when: always },
  { id: "choose-fbi", stage: "osaka", who: "irving", title: "Decide where to get FBI fingerprints", where: "in the call or at the fingerprint office", when: always },
  { id: "notary", stage: "osaka", who: "both", title: "Sign the California partnership declaration at the consulate notary", where: "at the U.S. Consulate General Osaka-Kobe", when: partnerIs("ca-partnership") },
  { id: "fbi-prints", stage: "osaka", who: "irving", title: "Get FBI fingerprints taken", where: "at Fingerprint Service Osaka", when: fbiIs("osaka") },

  // Paperwork
  { id: "fbi-prints-later", stage: "paperwork", who: "irving", title: "Get FBI fingerprints taken outside Osaka", when: fbiIs("later") },
  { id: "fbi-check", stage: "paperwork", who: "irving", title: "Receive the FBI background check", when: always },
  { id: "fbi-apostille", stage: "paperwork", who: "irving", title: "Apostille the FBI background check", when: always },
  { id: "coc-received", stage: "paperwork", who: "irving", title: "Receive the certificate of coverage", when: always },
  { id: "ca-file", stage: "paperwork", who: "both", title: "Mail the notarized declaration to the California Secretary of State", when: partnerIs("ca-partnership") },
  { id: "ca-apostille", stage: "paperwork", who: "both", title: "Apostille the registered partnership certificate", when: partnerIs("ca-partnership") },
  { id: "translations", stage: "paperwork", who: "both", title: "Sworn Spanish translations of the documents", when: always },
  { id: "insurance", stage: "paperwork", who: "both", title: "Private Spanish health insurance", when: always },
  { id: "moeno-visa-pick", stage: "paperwork", who: "moeno", title: "Pick Moeno's own visa type", when: partnerIs("own-visa") },
  { id: "mui-chip", stage: "paperwork", who: "moeno", title: "Mui: microchip and rabies vaccination up to date", when: always },
  { id: "mui-cert", stage: "paperwork", who: "moeno", title: "Mui: EU health certificate endorsed before the flight", when: always },

  // Visa application
  { id: "submit-dnv", stage: "apply", who: "irving", title: "Submit the Digital Nomad Visa application", when: always },
  { id: "moeno-family", stage: "apply", who: "moeno", title: "Moeno applies as a family member", when: partnerIs("ca-partnership") },
  { id: "moeno-own-apply", stage: "apply", who: "moeno", title: "Moeno submits her own visa application", when: partnerIs("own-visa") },
  { id: "dnv-approved", stage: "apply", who: "irving", title: "Digital Nomad Visa approved", when: always },
  { id: "mui-flight", stage: "apply", who: "moeno", title: "Book a pet-friendly flight for Mui", when: always },

  // Valencia
  { id: "flat", stage: "valencia", who: "both", title: "Sign the lease on our Valencia flat", when: always },
  { id: "padron", stage: "valencia", who: "both", title: "Register at the padron (empadronamiento)", when: always },
  { id: "tie", stage: "valencia", who: "both", title: "Get the TIE cards", when: always },
  { id: "pareja-register", stage: "valencia", who: "both", title: "Register as pareja de hecho in Valencia", when: partnerIs("pareja") },
  { id: "moeno-family-spain", stage: "valencia", who: "moeno", title: "Moeno applies for residence as family from Spain", when: partnerIs("pareja") },
  { id: "mui-home", stage: "valencia", who: "moeno", title: "Mui lands in Valencia", when: always },
];

export const INITIAL_SAVE = {
  character: null,
  routes: { partner: null, fbi: null },
  done: {},
  pos: null,
  introSeen: false,
};

export function activeTasks(save, stage) {
  return TASKS.filter((t) => (!stage || t.stage === stage) && t.when(save.routes));
}

export function progress(save, stage) {
  const list = activeTasks(save, stage);
  const done = list.filter((t) => save.done[t.id]).length;
  return { done, total: list.length, pct: list.length ? Math.round((done / list.length) * 100) : 0 };
}

export function osakaComplete(save) {
  const p = progress(save, "osaka");
  return p.done === p.total;
}

export function routeLabel(kind, id) {
  return ROUTES[kind].options.find((o) => o.id === id)?.label || "Not decided";
}

// Applying a route choice also marks the decision task done.
export function withRoute(save, kind, id) {
  const done = { ...save.done };
  if (kind === "partner") done["choose-partner"] = Boolean(id);
  if (kind === "fbi") done["choose-fbi"] = Boolean(id);
  return { ...save, routes: { ...save.routes, [kind]: id }, done };
}

export function withDone(save, ...ids) {
  const done = { ...save.done };
  for (const id of ids) done[id] = true;
  return { ...save, done };
}

export function toggleDone(save, id) {
  const done = { ...save.done };
  if (done[id]) delete done[id];
  else done[id] = true;
  return { ...save, done };
}

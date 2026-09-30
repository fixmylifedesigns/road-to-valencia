# Road to Valencia 🍊

A GBA-style adventure where Irving and Moeno (and Mui) work through the real checklist for moving from Osaka to Valencia on Spain's Digital Nomad Visa.

**Play:** https://fixmylifedesigns.github.io/road-to-valencia/

## Controls

| Action | Keyboard | Touch |
| --- | --- | --- |
| Walk | Arrow keys or WASD | D-pad |
| Talk, confirm | Z or Space | A |
| Back, hold to run | X | B |
| Menu | Enter | Start |
| Journal | Shift | Select |

## What's in the game

- **Osaka** (playable): the U.S. Consulate General Osaka-Kobe (notary for the California partnership declaration), Fingerprint Service Osaka (FBI prints) and the call with the company and lawyer at Cafe Nomado or at home (social security certificate of coverage request, route decisions).
- **Routes** change the checklist: California domestic partnership, pareja de hecho in Spain, or Moeno on her own visa; FBI prints in Osaka or later.
- **Journal**: journey progress bar and every stage through Valencia. Tick off real-life steps as they happen. Progress saves in the browser.

## Code

Next.js (app router, `src/`), static export.

- `src/game/map.js`: Osaka tile map, buildings, NPCs
- `src/game/sprites.js`: pixel sprites for Irving, Moeno, Mui and townsfolk
- `src/game/quests.js`: stages, routes and checklist tasks
- `src/game/scripts.js`: dialogue for every building
- `src/game/render.js`: canvas drawing
- `src/components/`: game loop, handheld shell, journal

```bash
npm install
npm run dev
```

## Releases

Every push to `main` builds the game and deploys it to GitHub Pages. If the `version` in `package.json` hasn't been released yet, the workflow tags `vX.Y.Z` and publishes a GitHub release with the notes from `CHANGELOG.md` and a zip of the build. Bump the version with every change.

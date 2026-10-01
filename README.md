# Road to Valencia 🍊

A Game Boy Advance game where Irving and Moeno (and Mui) work through the real checklist for moving from Osaka to Valencia on Spain's Digital Nomad Visa.

**Download:** grab `road-to-valencia-vX.Y.Z.gba` from the latest [release](https://github.com/fixmylifedesigns/road-to-valencia/releases/latest) and open it in mGBA, another GBA emulator, or put it on a flash cart. Progress uses battery save (SRAM).

## Controls

| Button | What it does |
| --- | --- |
| D-pad | Walk, move cursors |
| A | Talk, read signs, confirm |
| B | Hold to run, back |
| START | Menu (journal, switch between Irving and Moeno) |
| SELECT | Journal |
| L / R | Change journal page |

## What's in it

- **Osaka** (playable): the U.S. Consulate General Osaka-Kobe (notary for the California partnership declaration), Fingerprint Service Osaka (FBI prints) and the call with the company and lawyer at Cafe Nomado or at home (social security certificate of coverage request and route decisions). Osaka Station tells you what's left.
- **Routes** change the checklist: California domestic partnership, pareja de hecho in Spain, or Moeno on her own visa; FBI prints in Osaka or later.
- **Journal**: journey progress bar and every stage through Valencia. Tick off real-life steps as they happen.

## Building

Needs `arm-none-eabi-gcc`, Python 3 with Pillow, `make` and a host C compiler.

```bash
sudo apt-get install gcc-arm-none-eabi python3-pil
make
```

`make` generates all graphics from `tools/` into `build/assets.c`, compiles the game in `src/`, and fixes the ROM header with devkitPro's `gbafix` (downloaded on first build).

- `tools/world.py`: Osaka map layout, buildings, NPCs
- `tools/art.py`: map, interior and title artwork
- `tools/sprite_data.py`: pixel sprites for Irving, Moeno, Mui and townsfolk
- `tools/gen_assets.py`: turns everything into GBA tiles and palettes
- `src/quests.c`: stages, routes and checklist tasks
- `src/scripts.c`: dialogue for every building
- `src/world.c`, `src/dialog.c`, `src/journal.c`, `src/main.c`: the game

The 8x8 font is Daniel Hepper's public-domain `font8x8`, downloaded on first build.

## Releases

Every push to `main` builds the ROM. Bump `VERSION` and add a section to `CHANGELOG.md` with each change; the workflow then tags `vX.Y.Z` and publishes a release with the `.gba` attached.

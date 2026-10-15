# MONSTERMONS

*A supernatural creature-collecting RPG for the Game Boy Advance.*

Ten years ago the moon went dark for a single night — **the Nightfall**. When dawn came, vampires,
werewolves, ghosts, demons and cryptids walked among us. The Hunter Guild answered with the
**Soulbind**, a way to bond a creature's soul to a human's. You are a rookie hunter from the town of
Hollowmere — and the deeper you hunt, the clearer it becomes that the **Order of the Pale Sun** knows
exactly what caused the Nightfall.

**ROM:** [`MonsterMons.gba`](MonsterMons.gba) — a real GBA cartridge image (valid Nintendo header and
checksum, 32 KB battery save). Play it in any GBA emulator (mGBA, VisualBoyAdvance-M, RetroArch,
Delta/Pizza Boy on phones) or on hardware via a flash cart.

## Controls

| Button | Overworld | Menus / battle |
| --- | --- | --- |
| D-Pad | Walk | Move cursor |
| A | Talk / examine | Confirm |
| B (hold) | Run | Back / cancel |
| START | Open menu (Team, Bestiary, Bag, Hunter card, Save) | Finish name entry |

## Features

- **60 Monstermons across 8 supernatural types** — Beast, Blood, Shade, Infernal, Cryptid, Arcane,
  Undead and Lumin — with a full strength/weakness chart, each with hand-designed pixel art, lore,
  habitats and personalities (Brave, Calm, Swift, Mystic, Loyal).
- **Back-view sprites:** your own team is seen from behind in battle, Pokemon-style.
- **FireRed-style battle presentation:** HP/EXP info boxes with an animated EXP bar, a type-specific
  attack animation for every move (slashes, blood, shadow orbs, hellfire, bones, light...), screen
  shake on critical and super-effective hits.
- **Abilities:** every species has a passive ability (Dread Aura, Bloodlust, Thick Hide, Nocturnal,
  Sunborn, Last Stand, Phantom Body, Regrowth, Soul Surge) that changes how it fights.
- **A living world:** trainers spot you, flash a "!" and walk over to challenge you; townsfolk
  wander; 16 glowing relics hide supplies across the map.
- **Four Warden Seals** (the gym challenge): Rowan of the Wild, Morrigan of the Dead, Nyx of the Neon
  and Warden Ash of the Dawn. Each Seal strengthens your Soulbind. Collect all four, finish the story,
  and face the **Hunter Council** — four back-to-back masters — for the Hall of Legends.
- **Team battles up to 3 vs 3.** Carry six Monstermons, deploy up to three. Trainer duels and
  boss fights use full three-on-three formations; night-time wild packs come in pairs.
- **Strategic combat:** 41 moves, single-target and spread attacks, burns, curses, sleep and fear,
  stat stages, critical hits, drain and recoil, healing, and type matchup hints in the move menu.
- **Bonds and SOUL COMBOS.** Bonds grow as you fight together. High bond lets a partner endure a
  lethal blow for you, raises crit rates, and fills the shared Soul Gauge for a team-wide combination
  attack.
- **Branching transformations:** Pupwolf becomes Direwolf by day or Moonhowl by night; Wispette
  becomes Lanternis if deeply bonded or Banshriek if not; Fangling becomes Countess under the moon or
  the sunlit Dawnfang through devotion; plus level- and night-gated evolutions.
- **Soulbinding with conditions:** spirits can only be bound at night, Candlewick needs a Ritual
  Candle lit in battle, Wendigo must first accept an Offering, and legendary bosses can't be bound by
  force at all.
- **Hunter gear:** customize your hunter's coat and hair; equip weapons (Iron Knife, Silver Blade,
  Holy Crossbow, Rune Staff) for the **HUNT** command that weakens wild Monstermons without knocking
  them out, and tools (Lantern, Tracker Lens, Ward Charm) that change how you explore.
- **Day/night cycle and weather.** A running clock tints the world (neon and lit windows keep glowing),
  changes which creatures appear and how strong their moves are. Rain, rolling fog (which lowers
  accuracy) and the rare **Blood Moon** (stronger, rarer encounters) shape every visit.
- **Rare "Eclipse" variants** with alternate colors and boosted stats.
- **Exploration and investigation:** follow claw marks, fur and tracks through the Whisper Woods,
  find a hidden glade behind supernatural fog, solve the brazier puzzle of Ravencrest Manor.
- **Meaningful choices:** spare or turn in a wounded werewolf, bind or free the Hollow King, and decide
  the fate of the legendary Eclipsa — **Destroy, Control or Coexist**. Your choices shape the ending
  and your hunter's "path".
- **Bestiary, Soul Vault storage, shops, a Night Market, bounty board, Hunter Trials competition,
  rival battles, two legendary boss fights and a postgame** (stronger Rift encounters, an Apex
  bounty, and a final rematch).
- Original chiptune music and sound effects on the GBA sound chip; battery save.

## World

Hollowmere (gothic town) → Whisper Woods (haunted forest) → Grave Hill (graveyard) → Neon Row
(neon-lit city & Trial Arena) → Ravencrest Manor (abandoned mansion) → Duskhold Castle → **The Rift**
(a supernatural dimension).

Side regions:
- **Mistfen Marsh → Saltmarrow** (east of Hollowmere): a fog-bound bog and a fishing village whose
  pier the Pale Sun has seized. Free it, and at night the legendary **Tidemother** rises from the moon
  pool to test hunters who hold two Warden Seals and bring an Offering.
- **Ashen Rails → Gallowmoor** (east of Neon Row): an abandoned rail yard where a phantom train still
  runs at night, and the Veil's hidden town of monster sympathizers with a library quest.

## Not included (yet)

Being honest about scope: **online/link-cable battles and trading are not implemented** — GBA
link-cable multiplayer needs two linked units and couldn't be built and verified here. The world is
a set of connected areas rather than a seamless open world. Monster count (36) is a solid start for
an expansion.

## Building from source

Requirements: `arm-none-eabi-gcc` (Ubuntu: `apt install gcc-arm-none-eabi`), Python 3 with Pillow.

```sh
make            # builds MonsterMons.gba
make debug      # builds MonsterMons_debug.gba (SELECT opens a warp / test-team / story-flag menu)
```

All graphics, maps and the title screen are generated procedurally by `tools/gen_assets.py`
(creature art in `tools/monsters.py`, maps in `tools/maps.py`, characters & scenes in
`tools/scenes.py`). There is no external SDK — the startup code, linker script and hardware header
are in `src/`.

### Automated play-testing

`tests/harness.c` drives the ROM headlessly through libmgba (`apt install libmgba-dev`) from a small
input script and writes screenshots:

```sh
gcc -O2 -o build/harness tests/harness.c -lmgba
./build/harness MonsterMons.gba script.txt outdir
```

Script commands: `wait N`, `press KEYS [N]`, `hold KEYS N`, `mash KEYS N`, `shot NAME`,
`save NAME` / `load NAME` (save states), `reset`, `peek ADDR`, `poke ADDR VALUE`.

## Source layout

| File | Purpose |
| --- | --- |
| `src/main.c` | Boot, frame loop, clock, title screen, intro |
| `src/world.c` | Overworld: scrolling maps, movement, NPCs, encounters, weather, day/night |
| `src/events.c` | Story scripts, NPCs, warps, trainers, puzzles, choices and endings |
| `src/battle.c` | 3v3 battle engine, AI, Soulbinding, HUNT, Soul Combos |
| `src/menus.c` | Team, summary, bag, bestiary, hunter card, vault, shops, bounties, naming |
| `src/data.c` | Species, moves, items, type chart |
| `src/mon.c` | Stats, experience, bonds, transformations |
| `src/ui.c` | Text, windows, dialog, sprites, palettes and fades |
| `src/sound.c` | Music sequencer and sound effects |
| `src/save.c` | Battery save |

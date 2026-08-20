# Classes

Living reference for every class's identity, weapon, abilities, and progression — the creative/content "what." For the technical "how" (GAS architecture, AttributeSet, damage pipeline, ASC placement), see `SystemsDesign.md` §2. Naming convention (the `DC` prefix, tag hierarchy) is `SystemsDesign.md` §1 — not repeated here.

This doc is explicitly ongoing — start here, keep adding as classes get built out and the skill system gets refined. Don't treat any single pass over this file as final.

---

## Ability-slot philosophy: 4 slots, forever (added 2026-08-16)

Direct answer to "how do abilities expand and swap as a player levels, without overloading a controller": **every class gets exactly 4 active-ability slots, permanently — leveling never adds a 5th.** Not a beta-only cut; meant to hold through the full 4-class/8-spec release too.

- **Controller parity drives this, not the other way around** (`SystemsDesign.md` §9): 4 face buttons is the hard ceiling for instantly-reachable combat input on a pad. Rather than design on KBM and cram the result onto a controller later, the whole ability system is designed to the tighter constraint from day one — which also happens to match pillar #3, "class expression, not class bloat" (`GameDevPlan.md` §3).
- **Growth happens *inside* the 4 slots**, two ways, both already decided elsewhere — stated together here so it reads as one system:
  1. **Mastery-line leveling** (per-class, below) — the slot's ability gets numerically deeper through play. Same button, same slot, same muscle memory, just stronger.
  2. **Specialization** (`SystemsDesign.md` §2.6) — a one-time binary modifier fork on a single existing ability, chosen at the Hub's Loadout station (`Docs/GameplayLoops.md`), never mid-run. This *is* the "swap": picking between two flavors of one slot, not adding a slot or choosing from a talent pool. Each class below has a first-draft fork example.
- **Reserved but not built**: `SystemsDesign.md` §9's controller map leaves both triggers unbound in the beta on purpose, so there's headroom if post-beta design genuinely needs more — a hold-trigger-plus-face-button chord (alternate/charged cast of that slot, or eventually a true 5th "ultimate") is the scalable answer *if* it's ever needed. Not speculated on further than that; build it only when something real demands it.
- **Cost-pattern rule** (formalizing what Knight's code already does): exactly one ability per class is a resource sink (Stamina cost + cooldown); the rest are free or cooldown-only. Wizard is the deliberate exception — two of its four abilities cost resource, fitting its "resource-conscious caster" identity distinctly from the three more physical classes.

**Universal targeting rule**: no ability on any class, on any platform, uses manual aiming or target-cycling. Every target resolves automatically — nearest valid enemy in a forward cone, nearest ally or self if none in range, or a fixed lobbed point ahead of the caster — extending `SystemsDesign.md` §9's existing no-hard-target-lock decision from "Knight's melee" to the whole roster. This is what keeps Wizard's ranged/ground-targeted kit exactly as controller-friendly as Knight's melee, with no twin-stick-aim system ever needing to be built.

**Resource model**: one shared `Stamina` attribute across every class (`SystemsDesign.md` §2.1), reflavored cosmetically per class in UI once that UI exists (e.g. Wizard displays it as "Focus") rather than a real per-class Mana/Energy/Focus attribute fork. Flagged as overridable there — the alternative reads more authentically ARPG but forks the AttributeSet, regen curve, and every future itemization trinket per class for what's otherwise a cosmetic difference.

**Not yet designed**: shields/absorb (Nine Lives' ward, Groom's barrier) need a mechanism `UDCAttributeSet` doesn't have yet — likely a `ShieldAmount` meta-attribute checked early in `UDCDamageExecCalculation`, mirroring the existing `Damage` meta-attribute pattern (`SystemsDesign.md` §2.1/§2.4). Not needed until Wizard or Healer actually enter production.

---

## Knight (beta class — fully speced, in progress in code)

**Identity**: sustained tank-and-cleave melee, the beta's only playable class. Fights with a real sword and shield — cat-flavored ability names are expressed through *how* the weapon techniques are posed/styled, not by dropping the weapon (see `AssetPipeline.md` §4's "weapon-consistent styling" rule; Bunny Kick is the one deliberate exception).

**Weapon**: sword (`Socket.WeaponHandR`) + shield (`Socket.WeaponHandL`).

**Controller**: South = Claw Flurry/Pounce · East = Zoomies · West = Headbutt · North = Bunny Kick (slot-role legend: `SystemsDesign.md` §9).

**Abilities** (implementation status: `ProductionPlan.md` P1 — all 4 compiled, not yet PIE-tested):
- [x] **Claw Flurry / Pounce** (`GA_DC_Knight_BasicAttack`) — tap-vs-hold on one hotkey. Tap chains a 3-hit cleave combo (Swipe → Rake → Shred). Hold charges Pounce, a leaping heavy strike for a single tough target.
- [x] **Headbutt** (`GA_DC_Knight_ShieldBash`) — gap-closer + stagger, Stamina cost, on cooldown.
- [x] **Zoomies** (`GA_DC_Knight_Dash`) — burst mobility + i-frames, cancels Claw Flurry specifically.
- [x] **Bunny Kick** (`GA_DC_Knight_Whirlwind`) — AoE crowd control, on cooldown. The one ability where the cat drops the weapon-fighting facade entirely.

| Ability | Type | Cost | Cooldown | Damage | Range/Radius | Notes |
|---|---|---|---|---|---|---|
| Claw Flurry (tap) | Free | — | — | 8 / 8 / 12 per hit | 150uu, 60uu radius | 1.2s re-tap window before the combo resets to hit 1 |
| Pounce (hold) | Free | — | — | 25 | 350uu, 80uu radius | Charges past a 0.25s hold, releases on button-up |
| Headbutt | Cost + cooldown | 15 Stamina | 4s | 15 | 200uu, 70uu radius | 900uu/s forward launch; self-slows for the swing (`State.Attacking`); Stagger-on-hit designed but not wired yet (nothing reads it without enemy AI, P2 work) |
| Zoomies | Cooldown only | — | 3s | — (i-frames) | 1400uu/s launch, 0.3s duration | Cancels an in-progress Claw Flurry; grants `State.Invulnerable` for the dash |
| Bunny Kick | Cooldown only | — | 8s | 10 per target | 250uu radius, self-centered | Self-slows for the swing, same as Headbutt |

*Mirrors current code defaults (`Source/DungeonCat/GAS/DCGameplayAbility_*.h`) as of 2026-08-16 — code is the source of truth if this table ever drifts from it.*

**Mastery-line skill** (progression brainstorm, see "Skill & progression system" below): **Shield Mastery** — deepens Headbutt, tiered per the "learn by doing + milestone" framework (first-draft numbers, added 2026-08-19):

| Tier | Unlock | Headbutt change |
|---|---|---|
| 1 (baseline) | — | 15 Stamina, 4s cooldown, 15 dmg, 200uu range (current ability table above) |
| 2 | Land 25 Headbutts in play | Cooldown 4s → 3.25s |
| 3 (milestone) | Stagger 15 enemies in a single run | Cooldown 3.25s → 2.5s; Stamina cost 15 → 10 |

**Specialization fork example** (`SystemsDesign.md` §2.6's system, first draft): **Bulwark** (bigger stagger window, cooldown rises to 5s) vs **Blitz** (cooldown drops to 3s, no self-slow after impact, smaller stagger) — control vs. tempo.

**Animation status**: full list in `AssetPipeline.md` §4 (locomotion, death, hit reacts, downed/revive, all 4 abilities named per-hit). This is the only class with a complete animation checklist right now — the other three now have designed mechanics to build one from (this pass) but no animation list yet, see each class's own "Animation status" line below.

---

## Rogue — ambush predator (post-beta, named + identity locked, mechanics not yet designed)

**Identity**: opener/burst role, not sustained DPS. Stalk-then-strike, deliberately distinct from Knight's sustained-cleave-tank role so the two melee classes don't feel like reskins of each other.

**Weapon**: dual daggers (`Socket.WeaponHandR` + `Socket.WeaponHandL` both used for offense — note this generalizes `AssetPipeline.md` §3's `WeaponHandL` socket beyond "shield only"; update that table's description when Rogue actually gets built).

**Controller**: South = Quickclaw · East = Hiss · West = Slink · North = Ambush.

**Abilities** (mechanics designed 2026-08-16, per `SystemsDesign.md` §2.6's naming — first-draft numbers, not yet implemented in code):
- [ ] **Quickclaw** — basic attack, a faster/lighter multi-hit combo than Claw Flurry, chip damage between openers.
- [ ] **Slink** — stealth/setup: reduces detection, enables repositioning behind or away from a target.
- [ ] **Ambush** — the payoff: guaranteed-bonus-damage strike, full power only from Slink or from behind the target.
- [ ] **Hiss** — escape/defensive: short-range intimidation burst, nearby enemies flinch/back off.

| Ability | Type | Cost | Cooldown | Damage/Effect | Range/Radius | Notes |
|---|---|---|---|---|---|---|
| Quickclaw (tap combo) | Free | — | — | 6 / 6 per hit | 130uu, 50uu radius | 0.8s re-tap window — tighter than Claw Flurry's, reinforces "faster, lighter" |
| Slink | Cost + cooldown | 20 Stamina | 6s | Detection-radius reduction + small move-speed buff | Self, 3s duration | Attacking ends the buff early; arms Ambush's bonus below |
| Ambush | Cooldown only | — | 7s | 18 base, ×2 (36) if used during Slink or from a rear cone (~120°) on the target | 200uu, 70uu radius | The bonus check is just "was `State.Slinking` active, or is the attack angle inside the rear cone" at hit time |
| Hiss | Cooldown only | — | 10s | No damage — flinch + knockback | 300uu radius, self-centered | Rogue's answer to Zoomies' escape role without raw i-frames — trades mobility for crowd-disable, keeping the two melee classes distinct (`GameDevPlan.md` §3) |

**Mastery-line skill**: **Predator's Instinct** — deepens Ambush, tiered per the "learn by doing + milestone" framework (first-draft numbers, added 2026-08-19):

| Tier | Unlock | Ambush change |
|---|---|---|
| 1 (baseline) | — | ×2 (36) bonus damage from Slink or rear-cone (current ability table above) |
| 2 | Land 20 Ambushes in play | Rear-cone angle widens ~120° → 140° |
| 3 (milestone) | Land 50 Ambushes total | Multiplier ×2 → ×2.25 |

**Specialization fork example**: **Executioner** (×2.5 multiplier, longer cooldown) vs **Opportunist** (×1.75 multiplier, but the rear-cone requirement widens to ~180° so it triggers far more often) — burst vs. consistency.

**Animation status**: not started — mechanics now speced above, ready to inform an animation list once Rogue actually enters production (`AssetPipeline.md` §4 has Knight's as the template).

---

## Wizard — familiar/hex caster (post-beta, named + identity locked, mechanics not yet designed)

**Identity**: curse/control specialist, sets up kills for the team rather than pure nuking. Leans into the witch's-familiar folklore hook instead of generic elemental blasting.

**Weapon**: a hex-focus staff or orb (reuses the existing `Socket.WeaponHandR` "sword, staff" convention already anticipated in `AssetPipeline.md` §3 — no socket-table change needed, unlike Rogue).

**Controller**: South = Jinx · East = Nine Lives · West = Evil Eye · North = Hairball.

**Abilities** (named, per `SystemsDesign.md` §2.6; mechanics designed 2026-08-16 — first-draft numbers, not yet implemented in code):
- [ ] **Jinx** — basic attack, ranged cursed bolt.
- [ ] **Evil Eye** — single-target curse/root/mark.
- [ ] **Hairball** — lobbed AoE, explodes into a damage-over-time zone. Deliberately the one joke ability in the whole roster.
- [ ] **Nine Lives** — signature: an activated ward absorbing the next lethal hit. **Decided this pass**: self-only, not ally-targetable — keeps it distinct from Healer's Groom (an ally/self shield already), so the two support-adjacent tools don't overlap.

| Ability | Type | Cost | Cooldown | Damage/Effect | Range/Radius | Notes |
|---|---|---|---|---|---|---|
| Jinx | Free | — | ~0.5s innate swing timer | 10 | 1200uu, 40uu radius | Reuses the existing `SphereTraceFromSocket` melee helper, just longer/narrower — a hitscan-style bolt, no new projectile-actor system needed for grey-box. A real traveling/dodgeable projectile is a post-beta feel upgrade. |
| Evil Eye | Cost + cooldown | 20 Focus (shared `Stamina` attribute, reflavored — see "Ability-slot philosophy" above) | 6s | Root/slow ~2.5s + a "mark" (+20% damage taken from the whole party) for 4s | 900uu, single target | The team-support half of "sets up kills" — the mark helps allies' damage, not just Wizard's own |
| Hairball | Cooldown only | — | 9s | 4 per tick × 3 ticks (12 total) over 3s | 250uu radius, auto-lobbed ~600uu ahead at the nearest enemy cluster | Needs one small new helper (`SphereOverlapAroundPoint`, a trivial variant of the existing self-overlap helper), not a new system. Auto-targeted per the universal targeting rule above — never a manual reticle. |
| Nine Lives | Cost + cooldown | 40 Focus | 45s | Absorbs the next lethal hit; expires unused after 10s | Self only | Long cooldown is deliberate — a rare clutch button, not a spammable shield |

**Mastery-line skill**: **Hex Mastery** — deepens Evil Eye, tiered per the "learn by doing + milestone" framework (first-draft numbers, added 2026-08-19):

| Tier | Unlock | Evil Eye change |
|---|---|---|
| 1 (baseline) | — | 2.5s root/slow + 4s mark, +20% damage taken (current ability table above) |
| 2 | Apply 20 marks that a teammate finishes off | Mark duration 4s → 5s |
| 3 (milestone) | 30 mark-assist kills total | Mark bonus +20% → +25% damage taken |

**Specialization fork example**: **Warden** (longer root, mark drops) vs **Coven** (shorter root, mark spreads to a second nearby enemy) — single-target control vs. party-support spread.

**Animation status**: not started — mechanics now speced above, ready to inform an animation list once Wizard actually enters production.

---

## Healer — purr and groom support (post-beta, named + identity locked, mechanics not yet designed)

**Identity**: proactive (heal-pulse, shields) rather than reactive click-to-heal, since reactive healing fits fast action combat poorly. Combat-capable, not a pure healbot, per the class-expression pillar (`GameDevPlan.md` §3).

**Weapon**: claw-gauntlets or a light mace — fits "Swat," a gentler support-class basic attack than the other three classes' weapons.

**Controller**: South = Swat · East = Groom · West = Purr · North = Biscuits.

**Abilities** (named, per `SystemsDesign.md` §2.6; mechanics designed 2026-08-16 — first-draft numbers, not yet implemented in code):
- [ ] **Swat** — basic attack, deliberately lighter than the other three classes' — Healer fights, but it isn't the kit's main draw.
- [ ] **Purr** — signature: proactive AoE heal-pulse.
- [ ] **Groom** — proactive shield/barrier on an ally or self.
- [ ] **Biscuits** — party buff/rally. **Decided this pass**: combine both leading candidates rather than pick one — a brief party move-speed buff + a short health-regen tick, cooldown-gated. Fits "rally" framing better than either alone.

| Ability | Type | Cost | Cooldown | Damage/Effect | Range/Radius | Notes |
|---|---|---|---|---|---|---|
| Swat | Free | — | — | 5 / 5 per hit | 120uu, 50uu radius | Deliberately the lightest basic in the roster |
| Purr | Cost + cooldown | 25 (shared resource) | 7s | +20 HP to allies in radius | 400uu radius, self-centered | Pulse, not a HoT — simplest grey-box shape; the class's core resource sink |
| Groom | Cooldown only | — | 8s | +25 absorb shield, 5s duration | 500uu, nearest ally in a front cone, self if none in range | Distinct from Wizard's Nine Lives — ally-targetable and an absorb shield, not a self-only lethal-hit ward |
| Biscuits | Cooldown only | — | 20s | +20% move speed + 2 HP/s regen, both 4s | 600uu radius, self-centered, whole party | Longest cooldown in the roster — a rally button, not a repeatable buff |

**Mastery-line skill**: **Nurturing Touch** — deepens Purr, tiered per the "learn by doing + milestone" framework (first-draft numbers, added 2026-08-19):

| Tier | Unlock | Purr change |
|---|---|---|
| 1 (baseline) | — | +20 HP to allies, 400uu radius, 7s cooldown (current ability table above) |
| 2 | Heal 2000 total HP with Purr | Radius 400uu → 475uu |
| 3 (milestone) | Heal 3+ allies with a single Purr, 25 times | Heal amount +20 → +28 |

**Specialization fork example**: **Sanctuary** (bigger Purr radius, longer cooldown) vs **Quickpurr** (smaller radius, shorter cooldown) — coverage vs. frequency.

**Animation status**: not started — mechanics now speced above, ready to inform an animation list once Healer actually enters production.

---

## Skill & progression system (brainstorm — starting framework, expect to iterate)

Direct design goal from the dev: *"how skills, leveling, and abilities are gated, achieved, made fun to achieve... I don't want to gate everything but I want players to have something to look forward to."*

This section is about *skill* progression (universal skills like Nimble Paws, milestone challenges). *Ability* progression (mastery lines, specialization forks) is a separate, related system — see "Ability-slot philosophy" above.

**Recommended framework: hybrid "learn by doing" + milestone unlocks.**
- Skills level through natural play, not a separate grind screen — bypassing a locked container grants XP toward the relevant skill; landing a class ability's signature use grants XP toward its mastery line. Progression is a byproduct of playing.
- Higher tiers additionally gate behind a one-time **milestone/challenge** (e.g. "bypass 5 different containers in one run," "land 20 Ambushes") rather than a flat XP wall — concrete, memorable goals instead of an abstract grind number. This is the "something to look forward to" part.
- **Only a minority of content is gated.** Core combat, main loot, dungeon progress, objectives, and bosses are never skill-gated. Skill-gating applies only to bonus/optional content (some containers, maybe a secret room or two, cosmetic unlocks). This is deliberate — the dev was explicit about not wanting everything gated.

**Universal skills** (any class, not tied to a mastery line — technical hook: `SystemsDesign.md` §6):
- **Nimble Paws** — bypasses skill-gated containers (the Lockpicking analog).
- **Keen Senses** — reveals nearby loot/secret rooms on the map within a radius.
- **Night Eyes** — extends visibility/map reveal in dark areas.
- **Quiet Paws** (added 2026-08-19) — passively reduces your detection radius, so enemies notice you later. Earned by successfully slipping past or breaking off encounters without triggering them, rather than by fighting.
- **Sure Grip** (added 2026-08-19) — speeds up every interact-hold action (opening containers, reviving a teammate). Earned through raw interact-usage count over time.
- **Treasure Sense** (added 2026-08-19) — a small upward shift in your own loot-rarity roll odds (`Docs/Items.md`'s drop tables). Earned via a total-loot-collected milestone.

**Class mastery lines** — one per class, each deepening that class's signature ability (see each class section above: Shield Mastery, Predator's Instinct, Hex Mastery, Nurturing Touch).

This is a starting draft, not a locked list — the exact skill count, XP curve, and milestone challenges are meant to keep evolving here as the game gets built, not be finalized in one pass.

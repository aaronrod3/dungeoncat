# Classes

Living reference for every class's identity, weapon, abilities, and progression — the creative/content "what." For the technical "how" (GAS architecture, AttributeSet, damage pipeline, ASC placement), see `SystemsDesign.md` §2. Naming convention (the `DC` prefix, tag hierarchy) is `SystemsDesign.md` §1 — not repeated here.

This doc is explicitly ongoing — start here, keep adding as classes get built out and the skill system gets refined. Don't treat any single pass over this file as final.

---

## Knight (beta class — fully speced, in progress in code)

**Identity**: sustained tank-and-cleave melee, the beta's only playable class. Fights with a real sword and shield — cat-flavored ability names are expressed through *how* the weapon techniques are posed/styled, not by dropping the weapon (see `AssetPipeline.md` §4's "weapon-consistent styling" rule; Bunny Kick is the one deliberate exception).

**Weapon**: sword (`Socket.WeaponHandR`) + shield (`Socket.WeaponHandL`).

**Abilities** (full mechanical spec: `SystemsDesign.md` §2.3; implementation status: `ProductionPlan.md` P1 — all 4 compiled, not yet PIE-tested):
- [x] **Claw Flurry / Pounce** (`GA_DC_Knight_BasicAttack`) — tap-vs-hold on one hotkey. Tap chains a 3-hit cleave combo (Swipe → Rake → Shred). Hold charges Pounce, a leaping heavy strike for a single tough target.
- [x] **Headbutt** (`GA_DC_Knight_ShieldBash`) — gap-closer + stagger, Stamina cost, on cooldown.
- [x] **Zoomies** (`GA_DC_Knight_Dash`) — burst mobility + i-frames, cancels Claw Flurry specifically.
- [x] **Bunny Kick** (`GA_DC_Knight_Whirlwind`) — AoE crowd control, on cooldown. The one ability where the cat drops the weapon-fighting facade entirely.

**Mastery-line skill** (progression brainstorm, see "Skill & progression system" below): **Shield Mastery** — deepens Headbutt (shorter cooldown / bigger stagger window as it levels), earned through landing Headbutts in play plus a milestone challenge.

**Animation status**: full list in `AssetPipeline.md` §4 (locomotion, death, hit reacts, downed/revive, all 4 abilities named per-hit). This is the only class with a complete animation checklist right now — the other three are blocked on ability-mechanics design below.

---

## Rogue — ambush predator (post-beta, named + identity locked, mechanics not yet designed)

**Identity**: opener/burst role, not sustained DPS. Stalk-then-strike, deliberately distinct from Knight's sustained-cleave-tank role so the two melee classes don't feel like reskins of each other.

**Weapon**: dual daggers (`Socket.WeaponHandR` + `Socket.WeaponHandL` both used for offense — note this generalizes `AssetPipeline.md` §3's `WeaponHandL` socket beyond "shield only"; update that table's description when Rogue actually gets built).

**Abilities** (named, per `SystemsDesign.md` §2.6 — **mechanics not yet designed**, only identity/role per ability below):
- [ ] **Quickclaw** — basic attack, a faster/lighter multi-hit combo than Claw Flurry, chip damage between openers.
- [ ] **Slink** — stealth/setup: reduces detection, enables repositioning behind or away from a target.
- [ ] **Ambush** — the payoff: guaranteed-bonus-damage strike, full power only from Slink or from behind the target.
- [ ] **Hiss** — escape/defensive: short-range intimidation burst, nearby enemies flinch/back off.

**Mastery-line skill**: **Predator's Instinct** — deepens Ambush (bigger bonus-damage multiplier as it levels).

**Animation status**: not started — blocked on ability-mechanics design above, same as Wizard/Healer.

---

## Wizard — familiar/hex caster (post-beta, named + identity locked, mechanics not yet designed)

**Identity**: curse/control specialist, sets up kills for the team rather than pure nuking. Leans into the witch's-familiar folklore hook instead of generic elemental blasting.

**Weapon**: a hex-focus staff or orb (reuses the existing `Socket.WeaponHandR` "sword, staff" convention already anticipated in `AssetPipeline.md` §3 — no socket-table change needed, unlike Rogue).

**Abilities** (named, per `SystemsDesign.md` §2.6 — **mechanics not yet designed**):
- [ ] **Jinx** — basic attack, ranged cursed bolt.
- [ ] **Evil Eye** — single-target curse/root/mark.
- [ ] **Hairball** — lobbed AoE, explodes into a damage-over-time zone. Deliberately the one joke ability in the whole roster.
- [ ] **Nine Lives** — signature: an activated ward absorbing the next lethal hit. **Decided this pass**: self-only, not ally-targetable — keeps it distinct from Healer's Groom (an ally/self shield already), so the two support-adjacent tools don't overlap.

**Mastery-line skill**: **Hex Mastery** — deepens Evil Eye (longer/stronger curse as it levels).

**Animation status**: not started — blocked on ability-mechanics design above.

---

## Healer — purr and groom support (post-beta, named + identity locked, mechanics not yet designed)

**Identity**: proactive (heal-pulse, shields) rather than reactive click-to-heal, since reactive healing fits fast action combat poorly. Combat-capable, not a pure healbot, per the class-expression pillar (`GameDevPlan.md` §3).

**Weapon**: claw-gauntlets or a light mace — fits "Swat," a gentler support-class basic attack than the other three classes' weapons.

**Abilities** (named, per `SystemsDesign.md` §2.6 — **mechanics not yet designed**):
- [ ] **Swat** — basic attack, deliberately lighter than the other three classes' — Healer fights, but it isn't the kit's main draw.
- [ ] **Purr** — signature: proactive AoE heal-pulse.
- [ ] **Groom** — proactive shield/barrier on an ally or self.
- [ ] **Biscuits** — party buff/rally. **Decided this pass**: combine both leading candidates rather than pick one — a brief party move-speed buff + a short health-regen tick, cooldown-gated. Fits "rally" framing better than either alone.

**Mastery-line skill**: **Nurturing Touch** — deepens Purr (bigger heal radius/potency as it levels).

**Animation status**: not started — blocked on ability-mechanics design above.

---

## Skill & progression system (brainstorm — starting framework, expect to iterate)

Direct design goal from the dev: *"how skills, leveling, and abilities are gated, achieved, made fun to achieve... I don't want to gate everything but I want players to have something to look forward to."*

**Recommended framework: hybrid "learn by doing" + milestone unlocks.**
- Skills level through natural play, not a separate grind screen — bypassing a locked container grants XP toward the relevant skill; landing a class ability's signature use grants XP toward its mastery line. Progression is a byproduct of playing.
- Higher tiers additionally gate behind a one-time **milestone/challenge** (e.g. "bypass 5 different containers in one run," "land 20 Ambushes") rather than a flat XP wall — concrete, memorable goals instead of an abstract grind number. This is the "something to look forward to" part.
- **Only a minority of content is gated.** Core combat, main loot, dungeon progress, objectives, and bosses are never skill-gated. Skill-gating applies only to bonus/optional content (some containers, maybe a secret room or two, cosmetic unlocks). This is deliberate — the dev was explicit about not wanting everything gated.

**Universal skills** (any class, not tied to a mastery line — technical hook: `SystemsDesign.md` §6):
- **Nimble Paws** — bypasses skill-gated containers (the Lockpicking analog).
- **Keen Senses** — reveals nearby loot/secret rooms on the map within a radius.
- **Night Eyes** — extends visibility/map reveal in dark areas.

**Class mastery lines** — one per class, each deepening that class's signature ability (see each class section above: Shield Mastery, Predator's Instinct, Hex Mastery, Nurturing Touch).

This is a starting draft, not a locked list — the exact skill count, XP curve, and milestone challenges are meant to keep evolving here as the game gets built, not be finalized in one pass.

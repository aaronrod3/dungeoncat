# Idea Backlog

Running capture for future-feature brainstorming — deliberately separate from the "living" design docs (`Classes.md`, `SystemsDesign.md`, `GameDevPlan.md`, `Items.md`, `GameplayLoops.md`, etc.). Nothing logged here is decided, scoped, or committed to the roadmap; it's a staging area for ideas as they come to mind. An idea only affects real work once it's deliberately **promoted** — copied into the doc it belongs in and removed from here — which is always a separate, later step, not automatic.

**Companion view**: rendered as an HTML board (kanban-style: columns = dev-timeline phase, cards = ideas tagged by feature area), regenerated from this file whenever it changes. This markdown file is the source of truth; the board is a display of it.

## How an idea gets logged

Every entry gets two tags, both best-guess and revisable:
1. **Feature area** — which system it belongs to, and which real doc it would eventually update.
2. **Timeline placement** — which dev phase makes sense to revisit it in, not a promise it ships there.

## Feature area legend

| Tag | Covers | Target doc |
|---|---|---|
| Classes/Abilities | Class kits, abilities, specs, mastery lines | `Docs/Classes.md` |
| Combat/GAS | Ability-system architecture, damage pipeline | `SystemsDesign.md` §2 |
| Dungeon Gen | Procedural generation, room catalog | `SystemsDesign.md` §4 |
| Enemy AI | Archetypes, StateTree/EQS, boss AI | `SystemsDesign.md` §5 |
| Itemization/Loot | Items, drop tables, affixes, rarity | `Docs/Items.md`, `SystemsDesign.md` §6 |
| Co-op/Netcode | Replication, session model, server authority | `SystemsDesign.md` §3 |
| Progression/Saves | Skills, XP, unlocks, save scopes | `SystemsDesign.md` §7, `Docs/Classes.md` |
| UI/UX/Controls | HUD, menus, camera, input/controller | `SystemsDesign.md` §8/§9 |
| Hub/Gameplay Loop | The Den, run loop, game modes | `Docs/GameplayLoops.md` |
| Boss Design | Boss mechanics/phases | `SystemsDesign.md` Boss design section |
| Art/Animation | Modeling, rigging, animation pipeline | `Docs/AssetPipeline.md` |
| Audio | SFX/music/VO, GameplayCue hookups | `SystemsDesign.md` Audio section |
| Release/Platform | Storefront, business/marketing | `SystemsDesign.md` §3.1.1 |

## Timeline placement legend

`P0` reuse audit — done · `P1` GAS foundation — in progress · `P2` dungeon + AI v1 — **current** · `P3` co-op hardening · `P4` loop + boss · `After Beta` near-term post-beta (art pass, class 2, 4-player) · `Full Release` long-term (remaining classes/modes/polish) · `Inbox` not yet placed · `Parked` explicitly shelved or rejected

---

## Inbox (not yet placed)

*(none yet)*

## P0 — Reuse audit

*(done — new entries here would only be small addenda)*

## P1 — GAS foundation

### Universal heavy attack costs Stamina
- **Feature area**: Classes/Abilities
- **Captured**: 2026-08-16
- **Status**: Inbox
- **Idea**: Every character has a basic (light) attack and a heavy attack; the heavy attack costs Stamina.
- **Placement reasoning**: For Knight this means Pounce (the hold-charge half of `GA_DC_Knight_BasicAttack`) should cost Stamina — currently implemented free, no cost. That's a small revision to already-compiled P1 code, and cheap to make now since nothing's been PIE-tested yet (`Docs/SessionHandoff.md`). It also revises `Docs/Classes.md`'s "cost-pattern rule" ("exactly one resource-sink ability per class") — Basic Attack was the one universally-free slot; if its heavy half now costs Stamina too, that rule needs restating. And it implies Rogue/Wizard/Healer's basic attacks (Quickclaw/Jinx/Swat, currently drafted with no hold-charge at all) need a matching heavy variant added once those classes get built — that part is After Beta scope, not P1.

## P2 — Dungeon + AI v1 *(current)*

*(none yet)*

## P3 — Co-op hardening

*(none yet)*

## P4 — Loop + boss

*(none yet)*

## After the Beta

### Branch point per base ability: prestige or new use-case
- **Feature area**: Classes/Abilities
- **Captured**: 2026-08-16
- **Status**: Inbox
- **Idea**: Each class's 4 base abilities (one per ability-type slot) gets its own branch as the player levels. At the branch, the player picks between (a) a **prestige** of that ability — an upgraded/evolved version of the same one — or (b) a different ability entirely, filling the same slot but built for a different tactical use-case.
- **Placement reasoning**: Generalizes the existing spec system (`SystemsDesign.md` §2.6 — currently one binary fork on one ability, explicitly "not a talent tree") to all 4 slots, with a second branch type beyond the mastery-line's numeric deepening. Still respects the 4-slots/controller rule (`Classes.md`'s "Ability-slot philosophy") since the choice is made at the Loadout station and each slot still holds exactly one ability at a time — but it's a bigger build than "8 cheap binary forks": closer to a real talent layer, and roughly doubles the per-class ability-design surface (up to 4 alternate full abilities per class on top of the 4 base ones). Worth resolving before class-2 + spec work starts.

### Per-class resource split: Stamina for physical, Mana for casters
- **Feature area**: Combat/GAS
- **Captured**: 2026-08-16
- **Status**: Inbox
- **Idea**: Abilities consume either Stamina or Mana depending on class — physical classes (Knight, Rogue, Healer) spend Stamina, caster classes (Wizard) spend Mana as a real, separate resource rather than a reskinned Stamina.
- **Placement reasoning**: Directly reverses the resource-model recommendation logged in `SystemsDesign.md` §2.1 last session (one shared `Stamina` attribute, reflavored cosmetically per class) — that paragraph explicitly flagged itself as overridable, and this is the dev doing so. When promoted, this **replaces** that paragraph rather than appending to it: needs a new replicated `Mana`/`MaxMana` pair on `UDCAttributeSet`, its own regen/drain curve, and touches every Wizard ability cost already drafted in `Classes.md` (currently written as "shared `Stamina` attribute, reflavored"). Could be done as early as P1 for the schema only (same reasoning as why `CritChance`/`CritMultiplier` were added early, before any class needed them) even though it doesn't matter in practice until a caster class exists — dev's call whether to front-load that or wait for Wizard's own turn.

### Character leveling grants tokens for stat upgrades
- **Feature area**: Progression/Saves
- **Captured**: 2026-08-16
- **Status**: Inbox
- **Idea**: Leveling up the character rewards tokens, spendable on upgrading stats (Health/Stamina/Armor/MoveSpeed/etc. — the AttributeSet fields).
- **Placement reasoning**: A new progression axis, separate from the existing mastery-line (ability depth) and universal-skill (Nimble Paws etc., bonus-content-gating) systems already documented — this one is about core stat power. Notably, `SystemsDesign.md` §7 already lists "meta-currency" as a beta-scope profile-save field with no defined purpose yet — this idea could be exactly what that field is for, which would pull it into beta scope (or even P4, when run-end/reward flow gets built) rather than After Beta. Placed After Beta for now since no beta-scope doc currently designs a stat-upgrade spend-screen, but flag the §7 connection since it changes the calculus. Also needs bounding against the enemy-damage tuning already locked in (`SystemsDesign.md` §5.1) so stat growth doesn't break it. Assumed persistent/profile-scope leveling, not per-run — flag if that's wrong.

## Full Release

*(none yet)*

## Parked / Out of scope

*(none yet)*

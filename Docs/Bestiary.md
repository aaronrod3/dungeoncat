# Bestiary

Living reference for every enemy archetype, specific creature, and boss in the game — the creative/content "what," extending the beta's roster toward `GameDevPlan.md` §6's full-release target (4 reskinned archetypes across ~8-12 enemies, several multi-phase bosses). For technical architecture (StateTree/EQS specs, tuning-number rationale, the multi-config `DA_DC_EnemyConfig_*` pattern), see `SystemsDesign.md` §5 — this doc owns identity, theme, and creature roster; that doc owns "how an archetype behaves in code."

Ongoing doc — start here, keep adding as more specific creatures and bosses get designed. Only the beta's 2 archetypes (melee chaser, ranged spitter) and Swarm-mother are anywhere near production; everything else here is first-draft, same status as every other doc in this repo.

---

## Enemy archetypes

4 archetypes total, matching `GameDevPlan.md` §6's full-release target — each with one strong hand-modeled base creature, then recolored/rescaled/accessorized for the rest of its family (`GameDevPlan.md` §5's reuse pillar), rather than all-new meshes per creature.

### Melee chaser — rush and cleave
**Base creature: Giant Rat.** Wiry, fast, snapping. The most common early-run threat — teaches players the basic "gap-close, dodge, punish" combat rhythm.
- **Family**: Sewer Rat (default, weakest), Dire Rat (bigger, later-run reskin, same stats scaled up), Cave Weasel (a leaner recolor for underground rooms, same archetype).
- Technical spec (StateTree/EQS, tuning numbers): `SystemsDesign.md` §5.1.

### Ranged spitter — keep distance, punish melee
**Base creature: Venom Spider.** Skitters to a preferred range and spits, backing off if approached. Teaches players to close distance fast or use cover.
- **Family**: Cave Spider (default), Spitting Widow (later-run reskin, bigger venom payload), **Broodmother's Kin** — a visually-marked variant used specifically as the Broodmother boss's phase-2 summoned adds below; same archetype/stats, distinct coloring so players recognize "these came from her" the moment they've seen one earlier in a run.
- Technical spec: `SystemsDesign.md` §5.1.

### Swarm/leaper — numerous, flank, overwhelm
**Base creature: Roach Cluster.** Weak alone, dangerous in numbers — the archetype that punishes standing still.
- **Family**: Spiderlings (small, fast, spider-themed reskin), Mouse Horde (slightly tougher per-unit, smaller packs).
- Technical spec: `SystemsDesign.md` §5.1.

### Brute/Tank — slow, relentless, hits hard (new, added 2026-08-19)
**Base creature: Armored Lizard.** The missing "one big threat" test — the other 3 archetypes all emphasize either a single medium target, ranged pressure, or numbers; nothing tests positioning/kiting against one slow, high-HP, hard-hitting enemy until now.
- **Identity**: relentless slow approach (no retreat/reposition behavior, unlike the ranged spitter), a long telegraphed heavy hit, then an exploitable recovery window — readable and fair per the "readable cat power fantasy" pillar (`GameDevPlan.md` §3), but genuinely punishing if ignored.
- **Family**: Stone-scale Lizard (default), Bull Toad (a stockier recolor/rescale, same archetype), Ironhide Beetle (an armored-insect variant for later-run rooms).
- Technical spec (new): `SystemsDesign.md` §5.1 gets this archetype's StateTree/EQS/tuning entry alongside the existing 3.

---

## Bosses

Every boss is a mandatory gate immediately before its mode's final objective, never optional (`GameplayLoops.md`'s boss-gates-objective rule). All reuse an existing archetype's adds/assets rather than needing bespoke boss-only content, per `GameDevPlan.md` §5's reuse pillar — matches how Swarm-mother was originally justified.

### Swarm-mother (the beta's only boss — content relocated here from `SystemsDesign.md`, unchanged)

Identity: an enlarged vermin matriarch that spawns adds using the swarm/leaper archetype above — the most on-hook choice for the "cat vs vermin" hook (`GameDevPlan.md` §1).

- **Phases**: 2, threshold split at 50% HP.
- **Telegraphs**: every attack gets a visible ~0.5-1s wind-up (animation + optional cue) before it lands.
- **Phase 2 change**: a new attack pattern, plus 2-3 swarm adds spawn, reusing swarm-archetype content directly.
- **Arena**: mechanically flat for the beta stub (no hazards) — matches "boss stub," not "boss vertical slice." Environmental mechanics are a good post-beta expansion.
- **StateTree**: pure StateTree, no BT escape hatch needed at this scope (`GameDevPlan.md` §4.5).

### The Serpent (new, post-beta)

Identity: a lone predator-threat, filling in the "snakes" quarter of the bestiary hook that no fodder archetype above claims — a boss-tier creature rather than vermin filler.

- **Phases**: 2, threshold split at 50% HP.
- **Phase 1**: telegraphed lunge-bite (single-target burst) + a **constrict grab** — the Serpent coils around whoever it grabs; that player can't act and needs a teammate to free them within a few seconds (a co-op-specific moment, distinct from every existing attack pattern in the game).
- **Phase 2 (<50% HP)**: every attack now leaves a lingering venom-pool zone on the floor where it lands — the arena gets progressively more hazardous to stand in as the fight goes on, forcing the party to keep repositioning rather than tank-and-spank in one spot.
- **Arena**: starts flat, becomes de-facto hazardous through phase 2's venom pools rather than pre-built environmental hazards — cheap to build (reuses the "damage volume" pattern already used for `CombatLavaFloor` in the stock scaffold) while still escalating the fight.
- **StateTree**: pure StateTree — 2 phases, a handful of attack patterns, well within what StateTree handles cleanly without a BT escape hatch.

### The Broodmother (new, post-beta)

Identity: the spider matriarch — ties directly to the ranged-spitter archetype's "Broodmother's Kin" reskin above, so her adds are recognizably *hers* the moment a player has seen a Kin spider earlier in a run.

- **Phases**: 2, threshold split at 50% HP.
- **Phase 1**: ranged venom-spit (reuses the ranged-spitter's attack shape) + a **web-shot** — a slow-moving projectile that roots/slows whoever it hits, a control tool rather than raw damage.
- **Phase 2 (<50% HP)**: summons 2-3 **Broodmother's Kin** adds (the ranged-spitter reskin above — no new archetype needed) and begins weaving **persistent web zones** across the arena floor that slow anyone standing in them, layering add-pressure with a shrinking-safe-space arena mechanic.
- **Arena**: starts flat; phase 2's web zones are the escalation, same "mechanic creates the hazard, not pre-built geometry" approach as the Serpent.
- **StateTree**: pure StateTree.

### Stonehide (new, post-beta)

Identity: the Brute/Tank archetype's boss-tier cousin — a sustained-damage/positioning test rather than an add-pressure fight, giving the boss roster real variety in *what kind* of fight each one is (swarm-management, co-op-grab-rescue, arena-hazard-management, and now raw sustained tanking).

- **Phases**: 2, threshold split at 50% HP.
- **Phase 1**: a heavy, slow, telegraphed melee combo — tests dodge timing and sustained damage output rather than positioning against adds or hazards.
- **Phase 2 (<50% HP)**: unlocks a **charge/ram attack** that cracks part of the arena floor where it lands, permanently removing that section as usable space for the rest of the fight — a one-way arena shrink, distinct from the Serpent/Broodmother's temporary-hazard pattern.
- **Arena**: starts flat; phase 2 permanently alters it (contrast with the other two bosses' temporary/lingering-hazard approach, for variety across the roster).
- **StateTree**: pure StateTree.

---

## Open items

- Exact HP/damage tuning for the Brute/Tank archetype and all 3 new bosses — first-draft identity/mechanics only; numeric balance needs the grey-box arena to feel out, same status the beta's existing archetypes had before their `SystemsDesign.md` §5.1 tuning pass.
- Boss flavor/name/lore beyond the working titles above — intentionally left blank, see `Docs/Narrative.md`.

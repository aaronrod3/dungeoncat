# Session Handoff

> Rewritten every session — current status only, not appended to. Full history lives in the git commit log.
>
> **Away session?** Read `Docs/AsyncSessionProtocol.md` once now and follow it for the rest of the session without re-reading it.

## Status (2026-08-19)

**Full-game design expansion pass** — a large, multi-doc planning session while the dev models the cat in Blender (doesn't touch code, doesn't block on doc work). Scope: go broad across the full-release vision (not just beta/near-term), overriding several docs' earlier "design this when its turn comes" convention at the dev's explicit request, with one guardrail — anything with real downstream/architectural dependencies gets flagged for the dev's confirmation rather than silently decided. Worked through 10 elements one at a time with the dev's sign-off before each write, not a single unreviewed dump.

**New docs created**:
- `Docs/Bestiary.md` — full enemy archetype roster (including a new 4th archetype, Brute/Tank) + specific creatures + full boss roster (Swarm-mother plus 3 new bosses: the Serpent, the Broodmother, Stonehide).
- `Docs/UIUX.md` — wireframe-level layout for every screen (exact components, UMG anchor/position specs, Blueprint-build steps). All screen functionality is native C++; Blueprint is positioning/styling only, per the dev's explicit rule.
- `Docs/Narrative.md` — structural skeleton only for the game's world/story layer, every actual lore field left blank for the dev to fill in (dev-specified: structure now, content later).

**Resolved this pass**:
- **Dungeon generation, closed** — was reopened since 2026-08-13. The dev's own call after hearing the difficulty tradeoff: real procedural generation is deferred past the beta (it's genuine implementation scope and hollow without a room-art catalog that doesn't exist yet); **the beta ships one hand-authored dungeon layout** instead, built from the same 400uu-grid/door-socket module convention so nothing's wasted when generation gets built later. Per-run variety stays in seeded encounter/loot rolling. This fully unblocks P2 — updated `SystemsDesign.md` §4, `GameDevPlan.md` §4.2/§7/§8/§9, `P2_DungeonAI.md`, and added a new procedural-generation entry to `ProductionPlan.md`'s After-the-beta section (design preserved, not deleted).
- **Skill/progression depth** — 3 new universal skills (Quiet Paws, Sure Grip, Treasure Sense) added to `Classes.md`; each class's one-line mastery description turned into a real 3-tier table (baseline → play-earned → milestone-gated).
- **Weapon-vs-ability animation split, new rule** (dev-specified) — basic/weapon attacks are authored per weapon type (shareable across any class using that weapon), not per class; specific abilities stay bespoke per-class. Applied to `AssetPipeline.md`'s new Rogue/Wizard/Healer animation lists and folded into the naming convention (§6).
- **Full itemization pass** — a 6-affix pool matching `UDCAttributeSet` 1:1, rarity-tiered affix counts, itemized weapon variety (stat rolls only, never new movesets), one unique per class, and a cosmetics/customization catalog (fur colors/patterns, accessories, size — `Items.md`). Boss loot tables added on top: guaranteed rolls scaled to party size + a bonus-roll chance, weighted toward higher rarity than regular tables (dev-requested addition).
- **Post-beta game modes designed** — Kill-boss (no item stakes, the boss itself is the point, draws from the new 4-boss roster), Rescue (escort an NPC captive — flagged below, needs new AI tech), Combo (remixes the other 3). Hub World gained a Trophy Board and Cosmetics Mirror; the Portal gains a mode-select step once these exist.
- **Audio creative direction** — tone reference (playful/cartoon-bright, not horror) and a concrete GameplayCue-category checklist, replacing the old policy-only statement.

**Flagged for the dev — needs your call before any of these get treated as locked** (consolidated from throughout this pass; each is drafted as a first-draft option in its doc, not decided):
1. **Customization tech, two forks** (`SystemsDesign.md`'s new Customization section) — size via blend-shape morph target vs. bone-scaling; gear-fitting via skeletal mesh merge vs. Leader Pose Component. Both were already posed as open either/ors in `GameDevPlan.md` §4.3; still open.
2. **Rescue mode's escort AI** (`GameplayLoops.md`) — needs a genuinely new AI behavior (follow/flee, no combat capability, survivability tracking) that doesn't reuse any existing archetype pattern. Flagged as real new scope, not a light lift.
3. **Pounce should cost Stamina** (`Docs/IdeaBacklog.md`, pre-existing item, untouched this pass) — still open; touches an already-compiled ability's economy.
4. **Character leveling → stat-token spend system** (`Docs/IdeaBacklog.md`, pre-existing item) — still just a backlog idea, no design drafted yet this pass; would touch `UDCAttributeSet` balance that P2's enemy tuning already anchors to.
5. **Per-class Mana vs. shared Stamina** and **prestige-vs-new-ability branch fork** (`Docs/IdeaBacklog.md`, pre-existing items) — both still open, referenced but not resolved wherever this pass touched Wizard costs or mastery tables.

**Still open, unaffected by this pass**: everything below is exactly as it was — this was a docs-only session, no code touched.

**Still blocked on the dev's hands**:
- **Enhanced Input wiring (P1)**: `ADCPlayerCharacter` binds 4 ability Input Actions to `AbilityInputPressed`/`AbilityInputReleased` via `SetupPlayerInputComponent`, compiles clean. Still needs editor-side work: create 4 Input Actions + `IMC_DC_Default` (+ a gamepad-equivalent context per `SystemsDesign.md` §9) + `BP_DCPlayerCharacter`/`BP_DCPlayerController` in editor, then set `Default Pawn Class`/`Player Controller Class` on `Lvl_ThirdPerson`'s World Settings, before it can be PIE-tested with 2 clients.
- **P2 multiplayer-targeting fix**: done, see `Docs/P2_DungeonAI.md` Stage 2. Compiles clean, **not yet PIE-tested** — needs Stage 3's 2-player check.

Neither has been PIE-tested yet — simulated PIE input is unreliable, confirmed multiple times now, and this pass didn't touch code either way.

## Next step

Dev's hands needed on the same two fronts as before, unchanged by this pass:

1. **Enhanced Input**: editor asset/BP creation above, then PIE-test the 4 Knight abilities with 2 clients.
2. **P2 targeting fix**: PIE-test with 2 players standing apart in `Lvl_Combat`.

Independent of both, and now genuinely unblocked: **P2's dungeon work can start** — build the one hand-authored layout per `SystemsDesign.md` §4.2, using the 400uu-grid modules. Also worth a look whenever convenient: the 5 flagged decisions above are all quick reads in their respective docs, and none of them block any other work — they just shouldn't be treated as settled until you've weighed in.

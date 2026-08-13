# P2 — Dungeon + AI v1 (Weeks 3-4)

Detail doc for `ProductionPlan.md`'s P2 entry. Structure: Stage 1 (plan finalization/open questions) → Stage 2 (do-yourself/autonomous) → Stage 3 (testing/manual steps) — verify everything in Stage 1 before starting Stage 2 work, per `AsyncSessionProtocol.md`'s Mode A discipline.

---

## Stage 1 — Plan finalization / open questions

- [ ] **BLOCKING: dungeon generation technique.** Reopened 2026-08-13 (`SystemsDesign.md` §4) — the dev is reconsidering the approach, previously "prefab-room graph stitching." **Do not start any dungeon-generation-specific work below until this is confirmed.** Everything else in this phase (multiplayer-targeting fix, AI archetypes, basic loot pickup) is unaffected and can proceed in parallel.
- [x] Room module grid/socket convention — resolved as provisional pending the above (`AssetPipeline.md` §5).
- [x] AI archetype StateTree/EQS specs + tuning numbers — resolved, `SystemsDesign.md` §5.1.
- [x] Difficulty curve — resolved, `SystemsDesign.md` §5.1.
- [x] Multiplayer-targeting bug scope — confirmed both code sites during the 2026-08-13 design pass: `EnvQueryContext_Player.cpp:13` (`GetPlayerPawn(Owner, 0)`) and `CombatStateTreeUtility.cpp` (`FStateTreeGetPlayerInfoTask`, which may already partially loop by index in one place — worth double-checking its actual selection logic, not just the EQS context, when this gets fixed).
- [ ] **Automation-test governance**: this phase's exit criteria wants a repeatable-seed test in `Source/DungeonCat/Tests/`, but `AsyncSessionProtocol.md` forbids building/running that suite unsupervised in an away session. Write/run it only in a present, supervised session.
- [ ] **Exit-criteria gap**: the original exit criteria only tested "against a solo player," which can't actually validate a multiplayer-targeting fix. Fixed below (Stage 3) — add a minimal 2-player smoke check; the full server-authority audit still correctly belongs to P3.

## Stage 2 — Do-yourself (autonomous, once Stage 1's blocker clears)

Everything here needs the dungeon-gen technique confirmed first, **except** the targeting fix and loot pickup, which can start immediately:

- [ ] Fix multiplayer targeting — rewrite `EnvQueryContext_Player` and `CombatStateTreeUtility`'s player-selection logic to consider all of `GameState->PlayerArray`, not index 0. **Not blocked by the dungeon-gen decision — start this first.**
- [ ] Two enemy archetypes (melee chaser + ranged spitter) as StateTree AI, per `SystemsDesign.md` §5.1's specs and tuning numbers, each a `DA_DC_EnemyConfig_*` instance off one shared `ADCEnemyCharacter` base. **Not blocked — the AI spec doesn't depend on room layout.**
- [ ] Basic loot pickup (`Docs/Items.md`'s beta item list, no rarity/affix system yet — pick up, apply/add to carried-list). **Not blocked.**
- [ ] *(Once Stage 1's blocker clears)* Room module grid/socket convention, generation algorithm, `ADCDungeonGenerator`, runtime NavMesh rebuild.

## Stage 3 — Testing & manual steps (needs the dev's hands)

- [ ] Confirm the multiplayer-targeting fix in PIE with 2 players standing apart — an enemy shouldn't always default to player 0. (Minimal check; the full audit is P3's job.)
- [ ] Both enemy archetypes chase/attack correctly in PIE against a solo player.
- [ ] *(Once dungeon generation exists)* Same seed produces the same layout twice in a row — verify explicitly. A generated dungeon is fully nav-mesh-covered (no enemy stuck failing to path).
- [ ] Write/run the repeatable-seed automation test in `Source/DungeonCat/Tests/` — supervised session only, per Stage 1's governance note.

**Exit criteria**: 2-player targeting smoke check passes; both archetypes function correctly solo; *(once unblocked)* seed-repeatability and nav-mesh coverage verified.

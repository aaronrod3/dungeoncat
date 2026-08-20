# P2 — Dungeon + AI v1 (Weeks 3-4)

Detail doc for `ProductionPlan.md`'s P2 entry. Structure: Stage 1 (plan finalization/open questions) → Stage 2 (do-yourself/autonomous) → Stage 3 (testing/manual steps) — verify everything in Stage 1 before starting Stage 2 work, per `AsyncSessionProtocol.md`'s Mode A discipline.

---

## Stage 1 — Plan finalization / open questions

- [x] **Dungeon generation technique — resolved 2026-08-19.** Real procedural generation deferred post-beta; the beta ships one hand-authored layout instead, built from the same module-grid convention (`SystemsDesign.md` §4). No longer blocking — every item below can proceed.
- [x] Room module grid/socket convention — no longer provisional; confirmed in active use for the hand-authored v1 layout too (`AssetPipeline.md` §5).
- [x] AI archetype StateTree/EQS specs + tuning numbers — resolved, `SystemsDesign.md` §5.1.
- [x] Difficulty curve — resolved, `SystemsDesign.md` §5.1.
- [x] Multiplayer-targeting bug scope — confirmed both code sites during the 2026-08-13 design pass: `EnvQueryContext_Player.cpp:13` (`GetPlayerPawn(Owner, 0)`) and `CombatStateTreeUtility.cpp` (`FStateTreeGetPlayerInfoTask`, which may already partially loop by index in one place — worth double-checking its actual selection logic, not just the EQS context, when this gets fixed).
- [ ] **Automation-test governance**: this phase's exit criteria wants a repeatable-seed test in `Source/DungeonCat/Tests/`, but `AsyncSessionProtocol.md` forbids building/running that suite unsupervised in an away session. Write/run it only in a present, supervised session.
- [ ] **Exit-criteria gap**: the original exit criteria only tested "against a solo player," which can't actually validate a multiplayer-targeting fix. Fixed below (Stage 3) — add a minimal 2-player smoke check; the full server-authority audit still correctly belongs to P3.

## Stage 2 — Do-yourself (autonomous, once Stage 1's blocker clears)

Everything here needs the dungeon-gen technique confirmed first, **except** the targeting fix and loot pickup, which can start immediately:

- [x] Fix multiplayer targeting — **done 2026-08-13, not yet PIE-tested.** Both sites (`EnvQueryContext_Player::ProvideContext`, `FStateTreeGetPlayerInfoTask::EnterState`) now enumerate `GameState->PlayerArray` (via `APlayerState::GetPawn()`) instead of `UGameplayStatics::GetPlayerPawn`/`GetNumLocalPlayerControllers` — those are local-player-indexed APIs meant for split-screen, and from AI code running server-side they only ever resolved to the listen-server host's own pawn, silently ignoring every remote client. `CombatStateTreeUtility`'s loop looked multiplayer-aware (it does iterate) but wasn't, for the same reason - confirmed the Stage 1 suspicion. Compiles clean (`Build.bat DungeonCatEditor`). Needs the Stage 3 2-player PIE check below before this is considered verified.
- [ ] Two enemy archetypes (melee chaser + ranged spitter) as StateTree AI, per `SystemsDesign.md` §5.1's specs and tuning numbers, each a `DA_DC_EnemyConfig_*` instance off one shared `ADCEnemyCharacter` base. **Not blocked — the AI spec doesn't depend on room layout.**
- [ ] Basic loot pickup (`Docs/Items.md`'s beta item list, no rarity/affix system yet — pick up, apply/add to carried-list). **Not blocked.**
- [ ] Build the one hand-authored dungeon layout in-editor per `SystemsDesign.md` §4.2 — Entry → 6-10 rooms → Objective/Boss room → Exit, one optional Loot side-branch, all door-socket-snapped to the 400uu grid modules (`AssetPipeline.md` §5). Bake nav normally; no `ADCDungeonGenerator`, no runtime rebuild.

## Stage 3 — Testing & manual steps (needs the dev's hands)

- [ ] Confirm the multiplayer-targeting fix in PIE with 2 players standing apart — an enemy shouldn't always default to player 0. (Minimal check; the full audit is P3's job.)
- [ ] Both enemy archetypes chase/attack correctly in PIE against a solo player.
- [ ] The hand-authored layout is fully nav-mesh-covered (no enemy stuck failing to path).
- [ ] Same seed produces the same encounter/loot rolls twice in a row within the fixed layout — this replaces the old "same seed → same generated layout" check, since the layout itself is no longer generated. Write/run the repeatable-seed automation test in `Source/DungeonCat/Tests/` — supervised session only, per Stage 1's governance note.

**Exit criteria**: 2-player targeting smoke check passes; both archetypes function correctly solo; the hand-authored layout is nav-mesh-covered; encounter/loot seed-repeatability verified.

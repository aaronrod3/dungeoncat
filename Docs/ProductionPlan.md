# Production Plan (beta phases — index)

Turns `GameDevPlan.md` §8's 8-week roadmap into concrete phases. `SessionHandoff.md` tracks which phase is actually active and what's done within it — this doc is the phase structure itself, not live status.

**P0 and P1 are done and stay in full below.** From P2 on, each phase has its own detail doc (mirrors `zombieshooter`'s `Docs/Beta/` pattern) — this doc becomes a lightweight index for those, not the deep content itself. Each phase doc follows the same 3-stage structure: **Stage 1** (plan finalization/open questions) → **Stage 2** (do-yourself/autonomous work) → **Stage 3** (testing/manual steps needing the dev's hands).

---

## P0 — Reuse audit (before P1 starts)

Not on the original 8-week clock, but cheap and worth doing before writing new systems code.

- [x] Read through `zombieshooter`'s `Combat/UZSHealthComponent`, `Survival/`, `Interaction/UZSInteractableComponent`, and `Weapons/` for patterns worth porting (not copying) into `DC` equivalents — the single-damage-entry-point convention, the interactable-component pattern, and the replicated-property + `OnRep` + delegate convention are all called out as reuse targets in `SystemsDesign.md`.
- [x] Confirm which parts of the existing stock `Variant_Combat` scaffold (StateTree AI setup, `AnimNotify_*` attack-trace pattern, combo system) survive into the real game vs. get replaced — `SystemsDesign.md` already assumes the attack-trace and StateTree patterns survive; verify that assumption holds once you're actually looking at the code, not just describing it from memory.

**Exit criteria**: a short note in `SessionHandoff.md` confirming what's being reused vs. rebuilt — not a deliverable in itself, just make sure this isn't skipped silently. **Done 2026-08-12** — full findings in `SystemsDesign.md` §10. Headline result: the attack-trace/combo/StateTree *mechanisms* hold up as assumed, but `CombatEnemy`'s damage/health is entirely non-replicated (rebuild from scratch on GAS, don't adapt it), and enemy player-targeting is hardcoded to player index 0 (`EnvQueryContext_Player`) — a real blocker for P2/P3, see P2 below.

---

## P1 — GAS foundation (Weeks 1-2)

The highest-risk phase on the whole clock is proving replication works at all, so that's the gate, not a nice-to-have at the end.

- [x] `UDCAttributeSet` (per `SystemsDesign.md` §2.1) — Health/MaxHealth/Stamina/MaxStamina/Armor/MoveSpeed/CritChance/CritMultiplier + a `Damage` meta attribute, `PostGameplayEffectExecute` converts Damage into a real Health change per §2.4's single-entry-point rule.
- [x] `UDCAbilitySystemComponent` on `ADCPlayerState` (§2.2) — `ADCPlayerCharacter` forwards `IAbilitySystemInterface` there, with the standard `PossessedBy`(server)/`OnRep_PlayerState`(client) `InitAbilityActorInfo` timing fix.
- [x] `UDCDamageExecCalculation` single-entry-point damage pipeline (§2.4) — reads a `Data.Damage` SetByCaller magnitude, rolls crit (0% by default per the P1 decision below), applies flat Armor.
- [x] `UDCGameplayAbility` base class.
- [x] **Compiles clean** — `Build.bat DungeonCatEditor Win64 Development` succeeded, first real attempt, 2026-08-12 (one real bug caught and fixed along the way: `GameplayAbilities` needed enabling as a plugin in `DungeonCat.uproject`, not just as a `Build.cs` module dependency).
- [x] **Get one ability replicating across 2 PIE clients** — **PASSED, dev-confirmed 2026-08-12.** 2-player listen-server PIE, `DC_TestAbility_DealDamageToSelf` console command, on-screen "took X damage" message and the resulting Health value both correct on both clients. This was hands-on-keyboard work, not something drivable via `unreal-mcp` — simulated PIE console/keyboard input into a live viewport is confirmed unreliable (same finding as `zombieshooter`'s identical tooling), and a second PIE client process isn't `unreal-mcp`-reachable at all (only one editor instance's endpoint exists). **P1's core risk is retired** — GAS replication works.
- [x] **All 4 real Knight abilities implemented** (away session, 2026-08-13, Mode A per `AsyncSessionProtocol.md`) — `UDCGameplayAbility_BasicAttack` (Claw Flurry tap-combo + Pounce hold-charge, via `UAbilityTask_WaitInputRelease`/`UAbilityTask_WaitDelay` racing each other for the tap-vs-hold threshold), `UDCGameplayAbility_ShieldBash` (Headbutt), `UDCGameplayAbility_Dash` (Zoomies, cancels Claw Flurry via `CancelAbilitiesWithTag`), `UDCGameplayAbility_Whirlwind` (Bunny Kick). Throwaway test ability/effect deleted, superseded. Granted with `EDCAbilityInputID`-based `InputID`s on `ADCPlayerCharacter`, ready for Enhanced Input to call `AbilityInputPressed`/`AbilityInputReleased` once IMC_DC_Default exists.
  - **Compiled clean on the 4th attempt** (Mode A's cap) after two real bugs the compiler and a headless smoke-test caught, not guesses: (1) `EGameplayModOp::Multiplicative` isn't a real enum value (`MultiplyCompound` is), plus the deprecated `AbilityTags` property needed `SetAssetTags()` instead. (2) **Bigger one**: `UGameplayEffect::FindOrAddComponent<UTargetTagsGameplayEffectComponent>()` (the mechanism for granting a GameplayTag from a GameplayEffect in UE5.8's post-5.3 GameplayEffectComponent system) crashes with a fatal error when called from the GE's own constructor ("NewObject with empty name can't be used to create default subobjects... Use ObjectInitializer.CreateDefaultSubobject<> instead") — caught by the Tier-1 headless smoke test, not the compiler, exactly why that step exists separately from the compile gate. **Architecture pivot in response**: state tags (`State.Invulnerable` for Zoomies) now use `UAbilitySystemComponent::AddLooseGameplayTag`/`RemoveLooseGameplayTag` called directly from ability code at runtime instead of a GameplayEffect granting the tag; cooldowns (Headbutt/Zoomies/Bunny Kick) use manual timestamp tracking (`UDCGameplayAbility::CooldownDurationSeconds`/`StartCooldown()`/`CanActivateAbility()` override) instead of GAS's `CooldownGameplayEffectClass` system, since that also requires a GE to grant a cooldown tag via the same broken mechanism. **Scope cut, clearly flagged, not silent**: Headbutt's Stagger application is deferred — nothing in the codebase reads `State.Staggered` yet (no enemy AI exists, P2 work), so it wasn't worth working around the crash blind for an unconsumed tag. Headbutt deals damage only for now; revisit with the same loose-tag pattern once an enemy needs to react to it.
  - Smoke-tested clean (headless `-nullrhi` launch, no crash/assert signature) after the fix, confirmed via log inspection, not assumed.
- [ ] Enhanced Input wired for the 4 abilities + movement, via a new `IMC_DC_Default` — still blocked on hands-on editor work (Input Action/IMC asset creation), not on code. **Scope addition 2026-08-13**: controller support is now planned from the start (`SystemsDesign.md` §9), not just KBM — this needs a gamepad-equivalent input context too, not only `IMC_DC_Default`.
- [ ] Grey-box test arena (a flat room, no dungeon generation yet) to iterate combat feel in.
- [ ] **Not yet PIE-tested** — the away session's ceiling is "compiles clean + smoke-tested," per `AsyncSessionProtocol.md`'s known limits. Real gameplay verification (does Claw Flurry's combo timing feel right, does Pounce's trace actually reach, does Zoomies' cancel-into-Basic-Attack work as intended) needs a human at the keyboard with 2 PIE clients, same as the original replication proof.

**Exit criteria — MET.** Two PIE clients (Multiplayer Options, Players ≥ 2, listen-server) can both activate at least one Knight ability, and its effect is visible and correct on both clients. The remaining P1 checklist items (real abilities, Enhanced Input, grey-box arena) are no longer risk-gated by this — they're normal build-out work now.

**P1 design decisions (2026-08-12, answered by the dev):** attacks slow movement rather than rooting the player (a `State.Attacking` tag applies a MoveSpeed modifier for the attack's duration, not a movement block); Dash cancels Basic Attack specifically (via `CancelAbilitiesWithTag(Ability.Knight.BasicAttack)` on Dash's activation) but Shield Bash and Whirlwind commit fully once started; CritChance/CritMultiplier are plumbed through the AttributeSet and exec calc now but default to 0%, not live until a later itemization pass; the replication proof is a debug log + on-screen message, not a visual GameplayCue.

---

## P2 — Dungeon + AI v1 (Weeks 3-4)

Full detail: **`Docs/P2_DungeonAI.md`**. Summary: two enemy archetypes (melee chaser + ranged spitter) as StateTree AI with concrete tuning numbers, the multiplayer-targeting bug fix, and basic loot pickup — all buildable now. Dungeon generation itself (room catalog, generation algorithm) is **blocked pending the dev's dungeon-generation-technique decision** (reopened 2026-08-13, see `SystemsDesign.md` §4).

## P3 — Co-op replication hardening (Weeks 5-6)

Full detail: **`Docs/P3_CoopHardening.md`**. Summary: server-authority audit (7-scenario test checklist) with 2 real clients, swarm/leaper archetype, co-op downed/revive, 4-player forward-compat pass. Session model and Hub-as-lobby are already resolved, not open items here anymore.

## P4 — Get-item-escape loop + boss (Weeks 7-8)

Full detail: **`Docs/P4_LoopAndBoss.md`**. Summary: objective item + extraction logic, the Swarm-mother boss (2 phases, fully speced in `SystemsDesign.md`'s Boss design section), run-end flow, balance pass. Boss-gates-objective structure and boss identity are already resolved, not open items here anymore.

---

## After the beta

Per `GameDevPlan.md` §8's "After the beta" section (self-authored art pass, second class, 4-player hardening, remaining modes) — not detailed here yet since it depends on what P1-P4 actually reveal. Expand this doc with real phases (P5+) once the beta ships, rather than speculatively planning post-beta work now.

### Real procedural dungeon generation

Design preserved from `SystemsDesign.md` §4, which was reopened 2026-08-13 and resolved 2026-08-19 to a fixed hand-authored v1 layout for the beta (real generation is genuine implementation scope and produces nothing meaningful without a room-module art catalog that doesn't exist yet). Once both exist, this is the algorithm to build against, unchanged from the original design:

1. Seed an `FRandomStream` from the run seed.
2. Start at a fixed entry room. Random-walk a chain of 6-10 rooms, at each step picking an unvisited door-compatible module from the catalog.
3. Tag rooms by position in the chain: first = Entry, last = Exit, one interior room (weighted toward the far half) = Objective/Boss, remainder = Combat, with a small chance of a Loot side-branch if the chain has slack.
4. Reject-and-retry generation within a bounded attempt count (prevents infinite loops on a bad seed); fall back to the known-good hand-authored v1 layout if retries exhaust, rather than crashing.
5. Rooms spawn as actors at door-aligned transforms computed by walking the chain (sub-levels/streaming is a later optimization, not needed at 6-10 rooms).
6. Navigation rebuilds at runtime via a `NavMeshBoundsVolume` sized to the generated bounding box, moving to Nav Mesh Invokers only if profiling says the bounds-volume rebuild is too expensive.
7. Same seed always producing the same dungeon needs an explicit repeatable-seed automation test in `Source/DungeonCat/Tests/`, not an assumption.

**Trigger to start this phase**: once the room-module art catalog has enough real content beyond the v1 layout's rooms that generation would actually produce meaningful variety — not on a fixed calendar date.

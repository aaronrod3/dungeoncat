# Systems Design

Technical design for every major system, expanding `GameDevPlan.md`'s architecture section (§4) into concrete, buildable specs. This is the doc engineering should build against; `GameDevPlan.md` stays the pitch/decisions/roadmap layer, this is the "how." Update in place as systems get built and reality corrects the plan — don't let this drift from what's actually implemented (see `SessionHandoff.md`'s rule: status lives there, not here; this doc describes intended/current design, not "is it done").

---

## 1. Naming convention

One project prefix, **`DC`**, for every new class/asset belonging to the real game — this distinguishes real-game code from the stock template's unprefixed scaffold classes (`DungeonCatCharacter`, `DungeonCatGameMode`, etc.) which stay as-is (either reused directly or superseded, per class). Pick this once and don't change it — Blueprint asset renames are painful mid-project and can corrupt references.

- C++: `ADCPlayerCharacter`, `ADCGameMode`, `ADCPlayerState`, `ADCPlayerController`, `UDCAbilitySystemComponent`, `UDCAttributeSet`, `UDCGameplayAbility`, `ADCEnemyCharacter`, `ADCDungeonGenerator`.
- Content: `BP_DC_*`, `DA_DC_*` (DataAssets), `WBP_DC_*` (widgets), `GA_DC_*` (Gameplay Abilities), `GE_DC_*` (Gameplay Effects), `SK_DC_*` (skeletal meshes), `SM_DC_*` (static meshes), `T_DC_*` (textures), `M_DC_*`/`MI_DC_*` (materials/instances), `A_DC_*` (animation sequences), `AM_DC_*` (anim montages).
- Input Actions unprefixed (`IA_Attack`), Input Mapping Contexts prefixed (`IMC_DC_Default`) — matches the existing template's own convention (`IMC_Default`, `IMC_MouseLook`), just adding the project prefix for new ones so they're not confused with the stock template's IMCs if both stay in the project.
- GameplayTags: dot-hierarchy, `Ability.Knight.*`, `State.*` (e.g. `State.Stunned`, `State.Downed`), `Damage.*`, `Cue.*`. Root tag list lives in `Config/Tags/` once GAS work starts (DataTable-based, not hardcoded native tags, so design can add tags without a recompile).

## 2. Combat & ability system (GAS)

### 2.1 AttributeSet
One `UDCAttributeSet` for the beta (single class). Fields:

| Attribute | Notes |
|---|---|
| `Health` / `MaxHealth` | Clamped 0..Max via a `PreAttributeChange`/`ClampAttribute` pattern, never set directly. |
| `Stamina` / `MaxStamina` | Knight's resource — spent on abilities, regens idle (mirrors `zombieshooter`'s `UZSNeedsComponent` stamina-drain/regen pattern, which is proven and reusable). |
| `Armor` | Flat or percentage damage reduction — pick flat-then-scale for the beta, simplest to tune. |
| `MoveSpeed` | Feeds `CharacterMovementComponent::MaxWalkSpeed` via a GameplayEffect-driven multiplier, never set directly on the movement component. |
| `CritChance` / `CritMultiplier` | Present from the start even if the beta's single class doesn't tune them much — avoids a schema change when class 2 wants crit-focused kits. |

### 2.2 ASC placement: PlayerState, not Character
Attach `UDCAbilitySystemComponent` + `UDCAttributeSet` to `ADCPlayerState`, not `ADCPlayerCharacter`. Reasoning: the character is expected to be destroyed/respawned on death (§7 below), and a PlayerState-hosted ASC survives that, avoiding an ASC re-init + ability re-grant dance on every respawn. Standard pattern for GAS games without pawn-possession-swapping mid-match. `ADCPlayerCharacter` still implements `IAbilitySystemInterface::GetAbilitySystemComponent()` by forwarding to its `PlayerState`.

### 2.3 Abilities
Actual class/ability content (identities, names, kits, weapons, mastery-line skills) now lives in `Docs/Classes.md` — this section stays technical-architecture only. Every ability is a `UDCGameplayAbility` subclass (a thin base adding convenience getters mirroring `zombieshooter`'s `UZSUserWidgetBase` convenience-getter pattern — `GetDCPlayerCharacter()`, `GetDCAttributeSet()`, plus the shared melee-trace/effect-application helpers added in P1). Activation is server-authoritative (`NetExecutionPolicy = ServerInitiated` or `LocalPredicted` once prediction is worth the complexity — start `ServerInitiated` for the beta, revisit prediction only if input latency is actually a felt problem in co-op testing, not preemptively).

Internal GAS identifiers for the Knight's 4 abilities (unchanged regardless of player-facing flavor names, per the naming convention's "pick the identifier once" rule): `GA_DC_Knight_BasicAttack` (Claw Flurry/Pounce), `GA_DC_Knight_ShieldBash` (Headbutt), `GA_DC_Knight_Dash` (Zoomies), `GA_DC_Knight_Whirlwind` (Bunny Kick). Implementation status: `ProductionPlan.md` P1.

### 2.4 Damage pipeline
Single entry point, no exceptions: all damage flows through one `UDCAttributeSet`-side `ExecutionCalculation` (`UDCDamageExecCalculation`), invoked only via a `GameplayEffect` applied through `ASC->ApplyGameplayEffectSpecToTarget`. Nothing else mutates `Health` directly — mirrors `zombieshooter`'s `UZSHealthComponent` convention (`AZSPlayerCharacter::TakeDamage` is the *only* path that reaches `Server_ApplyDamage`), which is a proven pattern worth carrying over verbatim rather than re-deriving.

### 2.5 GameplayCues
VFX/SFX are `GameplayCue` handlers (`GC_DC_*`), never triggered directly from ability C++/BP logic — keeps presentation swappable by the person doing art/audio pass without touching gameplay code, and cues replicate correctly to all co-op clients for free.

### 2.6 Post-beta classes

Moved to `Docs/Classes.md` — Rogue/Wizard/Healer's identities, weapons, ability names, and mastery-line skills all live there now, along with the skill/progression system brainstorm. Per `GameDevPlan.md` §6, the full-release vision is 4 classes / 8 specializations; the beta ships Knight only. None of the other three classes are implemented — don't let `Docs/Classes.md`'s level of detail imply otherwise.

**Specialization system** (the "8 total" promise): each spec = the existing 4-ability kit plus one binary modifier choice on a single existing ability — not a talent tree, not new ability slots. Keeps "class expression, not class bloat" (`GameDevPlan.md` §3) intact; needs only one alternate-effect branch + one choice screen in Loadout per spec, not a sprawling new system.

## 3. Co-op & replication

### 3.1 Session model
Listen-server only for the beta (matches `GameDevPlan.md` §8 scope) — one player hosts, joined via **direct-IP** (resolved 2026-08-13: no Steam/OnlineSubsystem integration exists anywhere in this project or in `zombieshooter`, confirmed by checking both codebases directly — `zombieshooter`'s own production docs call Steam/EOS integration a still-open item there too and state "direct-IP only is much simpler and is what the project has assumed throughout." Nothing to reuse; direct-IP is still the right near-term build even now that the release platform is decided — see §3.1.1). No dedicated-server packaging, no host migration. If the host disconnects, the session ends and remaining players return to the Main Menu — acceptable for a 2-player, 8-12-minute run; document this as a known beta limitation, not a bug to chase. Non-host mid-run disconnect: despawn their pawn/clean up their ASC, remaining player(s) continue rather than the run auto-failing.

### 3.1.1 Steam release plan (added 2026-08-14)

**Decided**: releasing on Steam first (Epic Games Store no longer being weighed as an alternative). This doesn't change §3.1's direct-IP recommendation for the beta build — Steamworks integration is a distinct, separable layer on top of the same replicated-gameplay code, not a networking-transport prerequisite. What actually needs to happen, split by who does it:

**The dev's own steps (real account/payment actions, not something to automate or delegate)**:
1. Register a Steamworks Partner account and pay Valve's one-time $100 app fee.
2. Get a Steam **App ID** issued (this is what everything below actually needs).
3. Eventually: build the store page, capsule art, trailer, etc. — a marketing/business track, not an engineering one, and far downstream of the beta.

**What gets built once an App ID exists (not yet — nothing to do here until step 2 above happens)**:
- Enable the `OnlineSubsystemSteam` plugin in `DungeonCat.uproject` (currently commented-out boilerplate in `DungeonCat.Build.cs`, confirmed not enabled) and set the App ID in `DefaultEngine.ini`'s `[OnlineSubsystemSteam]` section.
- A `steam_appid.txt` file (App ID only) at the project root for dev-machine testing without going through the real Steam client launch path.
- Migrate the join flow from direct-IP to Steam Sessions/Steam invite (`SystemsDesign.md` §3.1's existing direct-IP code doesn't need to be thrown away — Steam Sessions can sit alongside it, or replace it, depending on how much direct-IP testing convenience is still wanted once Steam is live).
- Later still (post-beta-adjacent, not urgent): Steam achievements, rich presence, cloud saves as an alternative/supplement to the local save system (§7).

**Recommended timing**: register the Steamworks account and get the App ID early if there's no reason not to (it's a one-time step that unblocks everything else whenever it happens, and Valve's review/setup can take time) — but the actual `OnlineSubsystemSteam` code integration is naturally a P3-adjacent task (that's when the real session/join flow gets hardened anyway, per `Docs/P3_CoopHardening.md`), not something to front-load into P1/P2's grey-box work.

### 3.2 Server authority
Every ability, health change, enemy action, and loot pickup is server-authoritative from the first line — this is decision §2.1's non-negotiable, not a retrofit target. Concretely: `Server_` RPC prefix convention for every mutator (matches `zombieshooter`'s convention exactly — carry it over), `HasAuthority()` guards on every state-changing function, `ReplicatedUsing=OnRep_X` + `OnRep_X` broadcasts a delegate for every replicated property UI/animation cares about. **Never poll replicated state directly** from Tick or Blueprint — this specific mistake is called out because it's the easiest one to make under time pressure and the hardest to debug once several systems depend on the polled value being fresh.

### 3.3 Replication risk window
Per `GameDevPlan.md` §8/§9, weeks 5-6 are the highest-risk window on the whole 2-month clock. Concretely de-risk it by proving one ability replicates correctly across 2 PIE clients in **week 1**, not after the rest of combat is built — see `ProductionPlan.md` P1's exit criteria.

## 4. Procedural dungeon generation

**⚑ REOPENED 2026-08-13** — the dev is reconsidering the dungeon-generation technique/approach; the "prefab-room graph stitching" decision below (and `GameDevPlan.md` §4.2's matching DECISION) is **not currently settled**, despite the confident language throughout this section. Do not start P2 dungeon-generation implementation work (room catalog, door-socket convention, the generation algorithm) against this section until the dev confirms a technique. Everything else in this doc (AI, itemization, saves, UI, etc.) is unaffected and can proceed. See `Docs/P2_DungeonAI.md` for the live status of this blocker.

### 4.1 Room module grid
- Base grid unit: **400 uu** (matches common UE modular-kit convention, and is a clean multiple of the Mannequin-derived cat's capsule radius).
- Room modules are rectangular, sized in whole grid multiples — e.g. a standard combat room is 10x10 tiles (4000x4000uu), a corridor segment is 2x4 tiles. Exact catalog of room sizes is an art-pipeline decision (`AssetPipeline.md` §3), not a code decision — the generator just needs modules tagged with their footprint.
- Door sockets sit at fixed grid-aligned positions on a room's N/S/E/W edges (one door per edge minimum, corridor pieces are 2-door pass-throughs). A door socket is a `USceneComponent` tagged `Socket.Door` at a consistent local-space offset so any two modules' doors can snap together regardless of which modules they are.

### 4.2 Generation algorithm
Seed-driven graph stitching (not wave-function-collapse — explicitly out of scope per `GameDevPlan.md` §4.2):

1. Seed an `FRandomStream` from the run seed.
2. Start at a fixed entry room. Random-walk a chain of 6-10 rooms (per `GameDevPlan.md` §7's MVP target), at each step picking an unvisited door-compatible module from the catalog.
3. Tag rooms by position in the chain: first = Entry, last = Exit, one interior room (weighted toward the far half of the chain) = Objective/Boss, remainder = Combat, with a small chance of a Loot side-branch (a 1-room dead-end off the main chain) if the chain has slack.
4. Reject-and-retry generation if the chain can't place within a bounded number of attempts (prevents infinite loops on a bad seed) — log the failed seed for debugging, fall back to a known-good hand-authored layout if retries exhaust, rather than crashing or spawning a broken dungeon.

### 4.3 Runtime instantiation & nav
- Rooms are spawned as actors (not sub-levels, for the beta — level streaming is a post-beta optimization, not needed at 6-10 rooms) at door-aligned transforms computed by walking the chain.
- Navigation rebuilds at runtime via a `NavMeshBoundsVolume` sized to the generated dungeon's bounding box, or Nav Mesh Invokers per player if the bounding-volume approach proves too expensive to rebuild — start with the simpler bounds-volume approach and only move to invokers if profiling says so.
- Same seed always produces the same dungeon — this is load-bearing for debugging and must be verified (not assumed) once the generator exists: a repeatable-seed automation test belongs in `Source/DungeonCat/Tests/` per `ProductionPlan.md` P2.

## 5. Enemy AI

Architecture decided in `GameDevPlan.md` §4.5 (StateTree default, BT embedded escape hatch for the boss only). Per-archetype behavior spec:

| Archetype | Core loop | EQS need |
|---|---|---|
| Melee chaser | Idle → detect (AIPerception sight/hearing) → close-distance rush → attack when in range → recover | Closest-valid-approach-point query, mirrors `zombieshooter`'s `AZombieAIController` perception wiring |
| Ranged spitter | Idle → detect → maintain preferred range (retreat if too close, approach if too far) → attack | Ring query at preferred range, line-of-sight filtered |
| Swarm/leaper | Idle → detect → flank/surround (multiple enemies bias toward different approach angles) → leap-attack | Multi-point query scored for spacing from other swarm members, not just from the player |
| Boss | Idle → aggro → phase 1..N (telegraph → attack → recover, repeat, phase transition on health thresholds) → dead | Per-attack-pattern positioning queries, defined per phase |

All four share one base `ADCEnemyCharacter` + one base StateTree schema (extends the existing `CombatStateTreeUtility` conditions), parameterized per archetype rather than forked — per the multi-config rule below.

**Multi-config rule** (carried over from `zombieshooter`, worth stating explicitly here too): a new enemy is a new `DA_DC_EnemyConfig_*` data asset instance (speed/health/senses/damage/mesh/StateTree params), never a new C++ subclass. New enemies should be mostly art + a new config asset once the four archetypes exist.

### 5.1 Concrete AI specs (added 2026-08-13, replaces the prose-only table above with real numbers)

| Archetype | States/transitions | EQS |
|---|---|---|
| Melee chaser | Idle→Investigate (AIPerception sight/hearing)→Chase (until within AttackRange)→Attack (reused P0 sweep)→brief recover→loop | Closest-valid-approach-point, 150-250uu radius, nav-reachable + LOS filtered |
| Ranged spitter | Idle→Investigate→Chase (until within a preferred-range band)→Attack; Retreat state if player <400uu, Approach state if >900uu | Ring query, 750uu radius, 12-16 candidates, LOS filtered, scored to avoid stacking with other enemies |
| Swarm/leaper | Idle→Investigate→Flank (biased approach angle per member)→Leap-Attack (300-500uu trigger range)→recover | Multi-point, 6-8 candidates, scored for ≥150uu spacing from other swarm members + distinct angle bucket |

**Tuning numbers**, anchored to the player's actual 100 HP / 100 Stamina (`UDCAttributeSet` defaults) — the stock `ACombatEnemy` template's numbers (1.0 dmg etc.) are toy values sized to a trivial dummy pool and are explicitly not portable:

| Archetype | Health | Damage/hit | Notes |
|---|---|---|---|
| Melee chaser | 30 | 8-10 | ~3 Claw Flurry swipes to kill |
| Ranged spitter | 20 | 6-8 | glassier, punishes melee engagement |
| Swarm/leaper | 12-15 each | 5-6 each | weak alone, dangerous in numbers of 3-5 |

**Difficulty curve** (density/mix progression across a run, deliberately room-graph-independent since the dungeon-generation technique is reopened, §4): early encounters are 2-3 melee chasers only; mid encounters add a ranged spitter to the mix; late encounters go 3-4 mixed including swarm; the encounter immediately before the boss-gated objective is the largest "spike" (4-5 mixed) before committing to the boss. ~15-25 enemies total per run, sized to the 8-12 min pillar at roughly 20-40s per encounter.

## 6. Itemization & loot

Actual item/weapon/clothing/loot content now lives in `Docs/Items.md` — this section stays technical-architecture only.

- `UDCItemConfig` DataAsset: display name, icon, rarity tier, equip slot (if any), granted `GameplayEffect`(s) for stat modifiers. Mirrors `zombieshooter`'s `UZSItemConfig` shape closely enough to port the pattern, not the content.
- No equip/inventory UI for the beta — stat trinkets auto-apply their `GameplayEffect` on pickup, consumables are usable from a simple carried-list. Avoids scope creep into a full inventory screen the beta doesn't need.
- Pickup via `UDCInteractableComponent` (P0-audit-confirmed portable) — interact-prompt, not walk-over auto-pickup, so "who grabs it" stays server-authoritative-simple in co-op.
- Rarity tiers for the beta: Common / Uncommon / Rare / Epic (4 tiers is enough to feel ARPG-ish without needing a full affix system yet — procedural affixes are explicitly post-beta per `GameDevPlan.md` §4.6).
- Drop tables: one `DA_DC_LootTable_*` per enemy archetype + one per room type (Combat/Loot/Boss), rolled server-side only.
- **Skill-gated containers** (new, per the skill/progression brainstorm in `Docs/Classes.md`): a lockable container carries `RequiredSkill` (`FGameplayTag`) + `RequiredSkillLevel` (`int32`); locked until the opening player's tracked skill level meets the requirement. Skill levels live in a new replicated `TMap<FGameplayTag, int32>` on `ADCPlayerState` — profile-scope progression, persists via §7's save system, not run-scope. The actual skill list is a living, ongoing design effort (`Docs/Classes.md`), not fixed here.

## 7. Progression & saves

Two separate, non-overlapping save scopes — don't let them blur:

- **Profile save** (persistent across runs): cosmetic unlocks, meta-currency, class/spec unlocks (post-beta), skill levels (§6). One `USaveGame` per player profile, with a `SaveVersion` (`int32`) field from day one — the beta needs no migration logic yet, but adding the field now is free and expensive to retrofit later. Autosave trigger: the Run-End screen only, since nothing changes profile-state mid-run in beta scope.
- **Run state** (ephemeral, in-memory only for the beta): the current dungeon's seed, room graph, and per-player loot-carried-this-run. **No mid-run save/resume for the beta** — this is a deliberate design choice, not a cut corner: it preserves the extraction *tension* pillar (§3 in `GameDevPlan.md`) that a failed run has real stakes. Revisit only if playtesting says runs need to be pause-and-resume-able, which would be a pillar-level conversation, not a quiet architecture change.

Death handling (co-op, no permadeath for the beta): a downed player can be revived by a teammate within the run; a solo-downed player with no teammate available fails the run (matches the co-op-PvE decision — no permanent character loss, the *run's* loot is what's at stake, not the character).

## 8. UI/UX flow

`Main Menu (title screen: New/Continue/Quit) → Hub World (persistent shared space, see Docs/GameplayLoops.md — loadout/vendor/dummies/portal are physical interactables, not menu buttons) → [interact Portal: Host/Join direct-IP] → Dungeon HUD (per-player health/stamina bars, objective tracker, full-screen map toggle — see Docs/GameplayLoops.md's fog-of-war design, replaces the earlier "minimap cut" note) → Run-End screen (extract success/fail, loot summary — success keeps all run-carried loot, failure loses it; profile-scope unlocks/skill-levels persist regardless of run outcome, a separate save scope from run loot) → back to Hub.`

**Settings/Options menu** (previously absent from the flow entirely): reachable from Main Menu and as a non-pausing Dungeon HUD overlay — co-op action combat never pauses in real-time for one player, matching `zombieshooter`'s explicit precedent. Beta contents: volume/audio placeholder sliders, key/gamepad-rebind stub (see §9's controller-support decision), Leave-run action.

**Error/disconnect states** (previously unaddressed): host-disconnect toast + return to Main Menu (§3.1), join-failure message, generic connection-lost fallback.

**Downed/revive UI**: screen-edge desaturation + revive prompt + countdown. Spec: 60s downed duration before a solo player's run fails; revive requires a teammate within ~150uu channeling ~3s; a downed player crawls slowly, has no abilities, and is still finishable by a fresh hit (matches the ported `zombieshooter` state machine's "already downed = finishing blow," `SystemsDesign.md` §10's reuse audit).

Every HUD/menu widget is a dedicated `UDCUserWidgetBase` subclass (native `BindWidget` + `NativeConstruct` wiring, Blueprint side is layout/Class-Defaults only) — mirrors `zombieshooter`'s B1 UI convention, which the dev already validated works well for a solo-dev Claude-Code-assisted pace.

## 9. Camera & controls

Third-person, over-the-shoulder — `GameDevPlan.md`'s header explicitly says "third person," which resolves what could otherwise read as ambiguous against the Diablo-4 comparison (Diablo 4 is isometric; the visual/control reference is Dark and Darker's camera, not Diablo's). Reuse the existing stock template's `DungeonCatCharacter` camera-boom setup as the starting point rather than building a camera system from scratch — it's already third-person and already in the repo.

FOV 90. No hard target-lock — a soft auto-face-toward-nearest-target-in-cone on attack-input instead, readable without lock-on system complexity (and works equally well on controller, see below). Standard spring-arm collision, no custom camera-collision system needed for grey-box.

**Controller support, planned from the start** (2026-08-13 decision — overrides an earlier draft recommendation to cut it for the beta; that was `zombieshooter` precedent, not this project's call). Enhanced Input targets both KBM and gamepad from day one, not gamepad-as-an-afterthought: movement on the left stick, camera on the right stick, the 4 Knight abilities on face buttons (Claw Flurry/Pounce on the primary face button since it's tap-vs-hold, Headbutt/Zoomies/Bunny Kick on the others), interact on a shoulder button. This is a real scope addition to the still-blocked Enhanced Input work — `IMC_DC_Default` needs a gamepad-equivalent context alongside the KBM one, not just KBM. Tracked in `Docs/P2_DungeonAI.md`.

## Audio

Not addressed anywhere in the docs until now. Policy: every `GameplayCue` (§2.5) gets its hook point created — even silent/placeholder — as each ability/system is built, so wiring isn't deferred to a late scramble. Real sound design (actual SFX/music/VO, middleware decision if any) is a post-beta pass, matching the grey-box-art precedent already applied everywhere else.

## Boss design (Swarm-mother — the beta's only boss)

Identity: an enlarged vermin matriarch that spawns adds using the same swarm/leaper archetype (§5) already planned for P2/P3 — reuses assets/AI instead of needing bespoke boss-only content, and is the most on-hook choice for the "cat vs vermin" marketable hook (`GameDevPlan.md` §1). Decided 2026-08-13, the one live creative question asked this design pass.

- **Phases**: 2, threshold split at 50% HP.
- **Telegraphs**: every attack gets a visible ~0.5-1s wind-up (animation + optional cue) before it lands — matches the readable pillar and gives co-op players a fair dodge window.
- **Phase 2 change**: a new attack pattern, plus 2-3 swarm adds spawn, reusing swarm-archetype content directly.
- **Arena**: mechanically flat for the beta stub (no hazards) — matches "boss stub," not "boss vertical slice." Environmental mechanics are a good post-beta expansion.
- **This resolves `GameDevPlan.md` §4.5's deferred StateTree-vs-BT call**: 2 phases / ~3-4 total attack patterns is squarely inside what §4.5 already says pure StateTree handles cleanly ("don't reach for BT" for a handful of states). Default to pure StateTree for the beta boss — only reach for the BT escape hatch if implementation actually proves it unwieldy, which is unlikely at this scope. The call was framed as "let complexity decide it"; now that the complexity is scoped, it decides in favor of pure StateTree.
- **Boss-gates-objective**: the boss is a mandatory encounter immediately before the final objective, never optional — see `Docs/GameplayLoops.md` for the full structural rule (applies to every future game mode, not just this one).

## 10. P0 reuse audit findings (2026-08-12)

Verified against the actual code (not memory) per `ProductionPlan.md` P0. Corrects a few assumptions made earlier in this doc — this section is the ground truth where it disagrees with prose above.

**Confirmed reusable (port the pattern, not the class):**
- `AnimNotify_DoAttackTrace`'s sweep (`ACombatEnemy::DoAttackTrace`): a sphere sweep along the actor's forward vector from a named bone socket, filtered to `ECC_Pawn`. The geometry is archetype-agnostic and directly reusable for the Knight's melee abilities and the melee-chaser archetype — only the hit-application needs to change (currently calls `ICombatDamageable::ApplyDamage` directly; needs to route through the GAS damage pipeline instead, §2.4).
- The combo *mechanism* (`ComboSectionNames` + `Montage_JumpToSection` from an AnimNotify): reusable as-is. The combo *decision logic* is not — `ACombatEnemy::DoAIComboAttack` picks a random hit count (AI behavior), while the Knight's combo needs input-buffered continuation (did the player press attack again inside the combo window). Same mechanism, different driver.
- `ACombatAIController` (thin `AAIController` + `UStateTreeAIComponent` wrapper) — trivial and solid, use directly as the base for every archetype's AI controller.
- `CombatStateTreeUtility`'s condition/task library (grounded check, face-actor/face-location, set-speed, the in-danger reaction condition) — reusable shape, extend rather than replace. Two exceptions, see below.
- `CombatEnemySpawner`'s activate-on-trigger / track-until-depleted pattern — good starting point for a dungeon room's "combat encounter" logic (spawn on room entry, unlock progression once the room's spawner is depleted).
- `zombieshooter`'s `UZSInteractableComponent` — ports directly as `UDCInteractableComponent`, no GAS entanglement, already shaped as `BlueprintNativeEvent` + server-routed.
- `zombieshooter`'s `UZSHealthComponent` downed/revive **state machine** (health-depleted → enter downed → timer → revive-or-die, "already downed = finishing blow"): the logic is exactly what `SystemsDesign.md` §7 needs for co-op revive. Port the state machine, not the component — DC's health lives in `UDCAttributeSet` (§2.1), not a hand-rolled `ReplicatedUsing`/`OnRep` component.
- `zombieshooter`'s `UZSNeedsComponent` stamina drain-while-active/regen-while-idle tick shape — port the curve logic into a GAS ability/effect, not the component itself.

**Must be rebuilt, not reused — the audit's real findings:**
- **`ACombatEnemy::TakeDamage`/`ApplyDamage`/`CurrentHP` are entirely non-replicated** — no `HasAuthority()` gating anywhere in the class, plain floats, no `Replicated`/`OnRep`. This is a single-player sandbox implementation start to finish. None of `CombatEnemy`'s damage/health code ports; §2.4's GAS damage pipeline is being built from scratch, not adapted from this.
- **Player targeting is hardcoded to player index 0.** `EnvQueryContext_Player::ProvideContext` calls `UGameplayStatics::GetPlayerPawn(Owner, 0)` — literally "the first local player," full stop. `CombatStateTreeUtility`'s `FStateTreeGetPlayerInfoTask` has the same single-target assumption baked into its instance data (`TargetPlayerCharacter`, singular). **In a 2-player co-op session, every enemy would perceive and target only Player 0 and completely ignore Player 1.** This blocks every enemy archetype until fixed — rewrite both the EQS context and the StateTree task to consider all of `GameState->PlayerArray` (nearest, or threat-scored) before `ProductionPlan.md` P2's enemy work starts, not after.
- `zombieshooter`'s `Weapons/` (ranged/magazine/ammo/jam systems) — not relevant to the melee-only Knight beta. Skip entirely; revisit only if/when a ranged player class is added post-beta.

## 11. Open items (not blocking planning, but not yet decided)

- Exact Stamina regen/drain curve (tuning, needs the grey-box arena to feel out).
- Whether ability activation goes `ServerInitiated` or `LocalPredicted` long-term (§2.3 — start server-initiated, revisit only if latency is a felt problem).
- ~~Steam Sessions vs direct-IP~~ — **resolved 2026-08-13**, see §3.1. Direct-IP, no Steam integration exists to reuse.
- **Dungeon generation technique** — reopened 2026-08-13, see §4's flag. The dev is reconsidering the approach; nothing in §4 should be treated as settled until confirmed.

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

### 2.3 Abilities (Knight beta spec — 4 abilities)
Grey-box functional spec; exact numbers are tuning, not architecture:

1. **Claw Flurry / Pounce** (`GA_DC_Knight_BasicAttack` — internal identifier unchanged, these are player-facing flavor names layered on top per the naming convention's "pick the identifier once" rule) — one ability, tap-vs-hold, deliberately kept as a single hotkey rather than a 5th ability slot (`GameDevPlan.md` §3.3's kit-size budget). Tapping chains **Claw Flurry**, a Vermintide-style cleave combo (**Swipe → Rake → Shred**, 3 montage sections in one clip, not 3 separate abilities) that cleaves through multiple weak enemies — reuses the existing `Variant_Combat` `AnimNotify_*` attack-trace pattern (don't reinvent hit detection, port it), but input-buffered (continues only on a fresh press inside the combo window, not `CombatEnemy`'s AI-random hit count). Holding instead charges **Pounce**, a Souls-style heavy variant for a single tough target — reuses `CombatEnemy`'s existing charge-loop montage-section mechanism (`ChargeLoopSection`/`ChargeAttackSection`, confirmed reusable in the P0 audit) rather than inventing a new charge system.
2. **Headbutt** (`GA_DC_Knight_ShieldBash` — identifier unchanged) — short-range gap-closer + stagger (applies `State.Staggered` via a `GE_DC_Stagger` GameplayEffect with a tag-based duration). Stamina cost. A real cat headbutt/"bunt" reflavored as a forward shield-charge — the affection-gesture-as-stagger joke is deliberate.
3. **Zoomies** (`GA_DC_Knight_Dash` — identifier unchanged) — mobility/repositioning, i-frames during the active window (a `GameplayEffect` granting `State.Invulnerable` for the dash's active frames, removed on montage end via a notify, not a timer — avoids drift between animation and gameplay window). Named for the real cat-owner term for a sudden hyperactive running burst — the most on-hook name of the four.
4. **Bunny Kick** (`GA_DC_Knight_Whirlwind` — identifier unchanged) — AoE around the player, the "crowd control" tool for the swarm/leaper archetype. Cooldown-gated via a `GE_DC_Cooldown_Whirlwind` effect with the ability's own tag. Named for the real rapid-fire hind-leg-kick behavior a cat uses on anything it's pinned in a play-fight — a near-literal match for "spin and hit everything around you."

All four are `UDCGameplayAbility` subclasses (a thin base adding convenience getters mirroring `zombieshooter`'s `UZSUserWidgetBase` convenience-getter pattern — `GetDCPlayerCharacter()`, `GetDCAttributeSet()`, etc.). Activation is server-authoritative (`NetExecutionPolicy = ServerInitiated` or `LocalPredicted` once prediction is worth the complexity — start `ServerInitiated` for the beta, revisit prediction only if input latency is actually a felt problem in co-op testing, not preemptively).

### 2.4 Damage pipeline
Single entry point, no exceptions: all damage flows through one `UDCAttributeSet`-side `ExecutionCalculation` (`UDCDamageExecCalculation`), invoked only via a `GameplayEffect` applied through `ASC->ApplyGameplayEffectSpecToTarget`. Nothing else mutates `Health` directly — mirrors `zombieshooter`'s `UZSHealthComponent` convention (`AZSPlayerCharacter::TakeDamage` is the *only* path that reaches `Server_ApplyDamage`), which is a proven pattern worth carrying over verbatim rather than re-deriving.

### 2.5 GameplayCues
VFX/SFX are `GameplayCue` handlers (`GC_DC_*`), never triggered directly from ability C++/BP logic — keeps presentation swappable by the person doing art/audio pass without touching gameplay code, and cues replicate correctly to all co-op clients for free.

### 2.6 Post-beta classes (Rogue, Wizard, Healer) — not in beta scope

Per `GameDevPlan.md` §6, the full-release vision is 4 classes / 8 specializations; the beta ships Knight only. Ability kits for the other three are speculative-but-decided (identity + kit locked 2026-08-12, not yet built) — recorded here so the design work isn't lost, not because any of it is scheduled. Same 4-slot structure as Knight (basic attack, playmaking tool, mobility/control, signature payoff), same real-cat-behavior naming voice as §2.3.

**Rogue — ambush predator.** Opener/burst role, not sustained DPS — the whole identity is stalk-then-strike, deliberately distinct from Knight's sustained-cleave-tank role so the two melee classes don't feel like reskins of each other.
- **Quickclaw** — basic attack, a faster/lighter multi-hit combo than Claw Flurry — chip damage between openers, not the main damage source.
- **Slink** — stealth/setup: reduces detection, enables repositioning behind or away from a target.
- **Ambush** — the payoff: a guaranteed-bonus-damage strike, full power only from Slink or from behind the target.
- **Hiss** — escape/defensive: short-range intimidation burst, nearby enemies flinch/back off.

**Wizard — familiar/hex caster.** Curse/control specialist that sets up kills for the team rather than pure nuking — leans into the witch's-familiar folklore hook instead of generic elemental blasting.
- **Jinx** — basic attack, ranged cursed bolt.
- **Evil Eye** — single-target curse/root/mark.
- **Hairball** — lobbed AoE, explodes into a damage-over-time zone. Deliberately the one joke ability in the whole roster.
- **Nine Lives** — signature: an activated ward absorbing the next lethal hit (self or ally — TBD once this actually gets built).

**Healer — purr and groom support.** Proactive (heal-pulse, shields) rather than reactive click-to-heal, since reactive healing fits fast action combat poorly. Combat-capable, not a pure healbot, per the class-expression pillar (`GameDevPlan.md` §3).
- **Swat** — basic attack, deliberately lighter than the other three classes' — Healer fights, but it isn't the kit's main draw.
- **Purr** — signature: proactive AoE heal-pulse.
- **Groom** — proactive shield/barrier on an ally or self.
- **Biscuits** — party buff/rally (exact effect TBD — haste and regen are the leading candidates).

Full 16-ability roster now named (4 classes × 4 abilities). None of it is implemented — Knight's 4 (§2.3) are the only ones with real GAS classes, and even those are grey-box/not-yet-built per `ProductionPlan.md`. Don't let this section's level of detail imply otherwise.

## 3. Co-op & replication

### 3.1 Session model
Listen-server only for the beta (matches `GameDevPlan.md` §8 scope) — one player hosts via Steam Sessions (if Steam integration is already available from prior projects) or direct-IP/Steam-invite-join, no dedicated-server packaging, no host migration. If the host disconnects, the session ends — acceptable for a 2-player, 8-12-minute run; document this as a known beta limitation, not a bug to chase.

### 3.2 Server authority
Every ability, health change, enemy action, and loot pickup is server-authoritative from the first line — this is decision §2.1's non-negotiable, not a retrofit target. Concretely: `Server_` RPC prefix convention for every mutator (matches `zombieshooter`'s convention exactly — carry it over), `HasAuthority()` guards on every state-changing function, `ReplicatedUsing=OnRep_X` + `OnRep_X` broadcasts a delegate for every replicated property UI/animation cares about. **Never poll replicated state directly** from Tick or Blueprint — this specific mistake is called out because it's the easiest one to make under time pressure and the hardest to debug once several systems depend on the polled value being fresh.

### 3.3 Replication risk window
Per `GameDevPlan.md` §8/§9, weeks 5-6 are the highest-risk window on the whole 2-month clock. Concretely de-risk it by proving one ability replicates correctly across 2 PIE clients in **week 1**, not after the rest of combat is built — see `ProductionPlan.md` P1's exit criteria.

## 4. Procedural dungeon generation

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

## 6. Itemization & loot

- `UDCItemConfig` DataAsset: display name, icon, rarity tier, equip slot (if any), granted `GameplayEffect`(s) for stat modifiers. Mirrors `zombieshooter`'s `UZSItemConfig` shape closely enough to port the pattern, not the content.
- Rarity tiers for the beta: Common / Uncommon / Rare / Epic (4 tiers is enough to feel ARPG-ish without needing a full affix system yet — procedural affixes are explicitly post-beta per `GameDevPlan.md` §4.6).
- Drop tables: one `DA_DC_LootTable_*` per enemy archetype + one per room type (Combat/Loot/Boss), rolled server-side only.

## 7. Progression & saves

Two separate, non-overlapping save scopes — don't let them blur:

- **Profile save** (persistent across runs): cosmetic unlocks, meta-currency, class/spec unlocks (post-beta). One `USaveGame` per player profile.
- **Run state** (ephemeral, in-memory only for the beta): the current dungeon's seed, room graph, and per-player loot-carried-this-run. **No mid-run save/resume for the beta** — this is a deliberate design choice, not a cut corner: it preserves the extraction *tension* pillar (§3 in `GameDevPlan.md`) that a failed run has real stakes. Revisit only if playtesting says runs need to be pause-and-resume-able, which would be a pillar-level conversation, not a quiet architecture change.

Death handling (co-op, no permadeath for the beta): a downed player can be revived by a teammate within the run; a solo-downed player with no teammate available fails the run (matches the co-op-PvE decision — no permanent character loss, the *run's* loot is what's at stake, not the character).

## 8. UI/UX flow

`Main Menu → Lobby (host/join) → Loadout (beta: fixed Knight, no real choice yet, but build the screen so post-beta class selection slots in without a rebuild) → Dungeon HUD (per-player health/stamina bars, objective tracker; minimap is explicitly cut from the beta per GameDevPlan.md §6) → Run-End screen (extract success/fail, loot summary) → back to Hub.`

Every HUD/menu widget is a dedicated `UDCUserWidgetBase` subclass (native `BindWidget` + `NativeConstruct` wiring, Blueprint side is layout/Class-Defaults only) — mirrors `zombieshooter`'s B1 UI convention, which the dev already validated works well for a solo-dev Claude-Code-assisted pace.

## 9. Camera & controls

Third-person, over-the-shoulder — `GameDevPlan.md`'s header explicitly says "third person," which resolves what could otherwise read as ambiguous against the Diablo-4 comparison (Diablo 4 is isometric; the visual/control reference is Dark and Darker's camera, not Diablo's). Reuse the existing stock template's `DungeonCatCharacter` camera-boom setup as the starting point rather than building a camera system from scratch — it's already third-person and already in the repo.

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

## 11. Open items to resolve during P1 (not blocking planning, but not yet decided)

- Exact Stamina regen/drain curve (tuning, needs the grey-box arena to feel out).
- Whether ability activation goes `ServerInitiated` or `LocalPredicted` long-term (§2.3 — start server-initiated, revisit only if latency is a felt problem).
- Steam Sessions vs direct-IP for the beta's join flow (§3.1) — depends on whether Steam integration already exists from a prior project to reuse.

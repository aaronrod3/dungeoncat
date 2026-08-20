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

**Resource model across future classes** (2026-08-16 design pass): `Stamina`/`MaxStamina` stays the one shared resource attribute for every class, not a per-class fork (no separate `Mana` for Wizard, `Energy` for Rogue, etc.) — same regen/drain curve, same replication, same future itemization hooks (a "+Max Stamina" trinket works for every class without a schema branch). Each class reflavors it cosmetically only (UI display name/color — e.g. Wizard shows "Focus") once class-specific UI exists, not as a new attribute. Flagged clearly since it's a real fork: the alternative (true per-class resource attributes) reads more authentically ARPG but forks the AttributeSet, the regen/drain logic, and every future itemization trinket per class for what's otherwise a cosmetic difference — revisit if that ends up mattering more than expected once a second class is actually in players' hands. Full per-class ability specs (including which ability costs this resource for each class): `Docs/Classes.md`.

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

## 4. Dungeon layout (fixed v1, procedural deferred)

**RESOLVED 2026-08-19** — closes the 2026-08-13 reopening. Real procedural generation is genuine implementation scope (~weeks 3-4 per `GameDevPlan.md` §8) and produces nothing meaningful without a room-module art catalog that doesn't exist yet. So **v1 ships as one hand-authored dungeon layout**, not procedural generation — this fully unblocks P2 today. The module-grid/door-socket convention below is unchanged and still enforced even though nothing is procedurally stitched yet, specifically so upgrading to real generation later is cheap (swap "one hand-placed layout" for "many interchangeable modules + an algorithm," not a re-architecture). The seed-driven graph-stitching design that used to live in this section is preserved, not deleted — see `ProductionPlan.md`'s new "Real procedural dungeon generation" entry under After the beta.

### 4.1 Room module grid
- Base grid unit: **400 uu** (matches common UE modular-kit convention, and is a clean multiple of the Mannequin-derived cat's capsule radius).
- Room modules are rectangular, sized in whole grid multiples — e.g. a standard combat room is 10x10 tiles (4000x4000uu), a corridor segment is 2x4 tiles. Exact catalog of room sizes is an art-pipeline decision (`AssetPipeline.md` §3), not a code decision.
- Door sockets sit at fixed grid-aligned positions on a room's N/S/E/W edges (one door per edge minimum, corridor pieces are 2-door pass-throughs). A door socket is a `USceneComponent` tagged `Socket.Door` at a consistent local-space offset so any two modules' doors can snap together regardless of which modules they are — enforced now even for hand-placement, so the modules themselves are already generator-ready.

### 4.2 v1 layout: one hand-authored dungeon
- Built in-editor from the same 400uu-grid modules, snapped door-to-door exactly the way the eventual generator would place them — just placed by hand instead of algorithmically.
- Structure: Entry room → 6-10 rooms total (unchanged target from `GameDevPlan.md` §7) → an Objective/Boss room (mandatory gate, `GameplayLoops.md`'s boss-gates-objective rule) → Exit, plus one optional Loot side-branch dead-end — same shape the original chain design intended, just fixed instead of walked.
- **Per-run variance stays in encounter/loot rolling, not layout.** Room shapes/positions are fixed, but enemy composition per combat room (§5.1's difficulty curve) and loot rolls (`Items.md`'s drop tables) are still rolled server-side per run from a seed — that logic doesn't depend on the layout being generated, so there's no reason to make it static too. A run through the fixed layout still plays differently each time even though the rooms don't move. This also means the seed-repeatability test P2 wanted still has a real target: same seed → same encounter/loot rolls (not same layout, which is fixed by construction).
- Nav mesh: baked normally in-editor (standard `NavMeshBoundsVolume`, no runtime rebuild) — removes an entire category of P2 risk, since runtime nav rebuild was one of the trickier parts of the original generator plan.

### 4.3 Post-beta: real procedural generation
Moved to `ProductionPlan.md`'s After-the-beta section — the full seed-driven graph-stitching design (random-walk chain, reject-and-retry, runtime instantiation/nav rebuild) is preserved there rather than deleted. Trigger to revisit: once the room-module art catalog has enough real content beyond v1's single layout that generation would actually produce meaningful variety.

## 5. Enemy AI

Architecture decided in `GameDevPlan.md` §4.5 (StateTree default, BT embedded escape hatch for the boss only). Per-archetype behavior spec:

| Archetype | Core loop | EQS need |
|---|---|---|
| Melee chaser | Idle → detect (AIPerception sight/hearing) → close-distance rush → attack when in range → recover | Closest-valid-approach-point query, mirrors `zombieshooter`'s `AZombieAIController` perception wiring |
| Ranged spitter | Idle → detect → maintain preferred range (retreat if too close, approach if too far) → attack | Ring query at preferred range, line-of-sight filtered |
| Swarm/leaper | Idle → detect → flank/surround (multiple enemies bias toward different approach angles) → leap-attack | Multi-point query scored for spacing from other swarm members, not just from the player |
| Brute/Tank (new, added 2026-08-19) | Idle → detect → relentless slow approach (no retreat/reposition band, unlike the ranged spitter) → long-telegraph heavy attack → long exploitable recovery → loop | None — direct move-to-target, no positioning query needed. Cheaper to implement than the other 3 archetypes, which all need real EQS-driven positioning. |
| Boss | Idle → aggro → phase 1..N (telegraph → attack → recover, repeat, phase transition on health thresholds) → dead | Per-attack-pattern positioning queries, defined per phase |

Creature identities/theming for all 4 archetypes and the full boss roster: `Docs/Bestiary.md`.

All four share one base `ADCEnemyCharacter` + one base StateTree schema (extends the existing `CombatStateTreeUtility` conditions), parameterized per archetype rather than forked — per the multi-config rule below.

**Multi-config rule** (carried over from `zombieshooter`, worth stating explicitly here too): a new enemy is a new `DA_DC_EnemyConfig_*` data asset instance (speed/health/senses/damage/mesh/StateTree params), never a new C++ subclass. New enemies should be mostly art + a new config asset once the four archetypes exist.

### 5.1 Concrete AI specs (added 2026-08-13, replaces the prose-only table above with real numbers)

| Archetype | States/transitions | EQS |
|---|---|---|
| Melee chaser | Idle→Investigate (AIPerception sight/hearing)→Chase (until within AttackRange)→Attack (reused P0 sweep)→brief recover→loop | Closest-valid-approach-point, 150-250uu radius, nav-reachable + LOS filtered |
| Ranged spitter | Idle→Investigate→Chase (until within a preferred-range band)→Attack; Retreat state if player <400uu, Approach state if >900uu | Ring query, 750uu radius, 12-16 candidates, LOS filtered, scored to avoid stacking with other enemies |
| Swarm/leaper | Idle→Investigate→Flank (biased approach angle per member)→Leap-Attack (300-500uu trigger range)→recover | Multi-point, 6-8 candidates, scored for ≥150uu spacing from other swarm members + distinct angle bucket |
| Brute/Tank (new, added 2026-08-19) | Idle→Investigate→Chase (relentless, no retreat/approach bands)→Attack (~1-1.2s telegraph, long wind-up)→long recover (exploitable)→loop | None — direct move-to-target |

**Tuning numbers**, anchored to the player's actual 100 HP / 100 Stamina (`UDCAttributeSet` defaults) — the stock `ACombatEnemy` template's numbers (1.0 dmg etc.) are toy values sized to a trivial dummy pool and are explicitly not portable:

| Archetype | Health | Damage/hit | Notes |
|---|---|---|---|
| Melee chaser | 30 | 8-10 | ~3 Claw Flurry swipes to kill |
| Ranged spitter | 20 | 6-8 | glassier, punishes melee engagement |
| Swarm/leaper | 12-15 each | 5-6 each | weak alone, dangerous in numbers of 3-5 |
| Brute/Tank (new, added 2026-08-19) | 70 | 18-22 | ~5-6 Claw Flurry swipes to kill; the long telegraph is what makes this fair rather than a DPS check |

**Difficulty curve** (density/mix progression across a run, deliberately layout-independent per §4's fixed v1 approach): early encounters are 2-3 melee chasers only; mid encounters add a ranged spitter to the mix; late encounters go 3-4 mixed including swarm, with a Brute/Tank as an occasional late-run "big threat" spike rather than a regular; the encounter immediately before the boss-gated objective is the largest "spike" (4-5 mixed) before committing to the boss. ~15-25 enemies total per run, sized to the 8-12 min pillar at roughly 20-40s per encounter. Full creature identities: `Docs/Bestiary.md`.

## 6. Itemization & loot

Actual item/weapon/clothing/loot content now lives in `Docs/Items.md` — this section stays technical-architecture only.

- `UDCItemConfig` DataAsset: display name, icon, rarity tier, equip slot (if any), granted `GameplayEffect`(s) for stat modifiers. Mirrors `zombieshooter`'s `UZSItemConfig` shape closely enough to port the pattern, not the content.
- No equip/inventory UI for the beta — stat trinkets auto-apply their `GameplayEffect` on pickup, consumables are usable from a simple carried-list. Avoids scope creep into a full inventory screen the beta doesn't need.
- Pickup via `UDCInteractableComponent` (P0-audit-confirmed portable) — interact-prompt, not walk-over auto-pickup, so "who grabs it" stays server-authoritative-simple in co-op.
- Rarity tiers for the beta: Common / Uncommon / Rare / Epic (4 tiers is enough to feel ARPG-ish without needing a full affix system yet for launch — the beta itself still ships with no affixes; the affix system below is post-beta content, designed now but not built until its own phase).
- Drop tables: one `DA_DC_LootTable_*` per enemy archetype + one per room type (Combat/Loot/Boss), rolled server-side only.
- **Boss loot tables (post-beta, added 2026-08-19)**: content/numbers live in `Docs/Items.md`; technical shape here. `DA_DC_LootTable_Boss_*` extends the regular loot-table asset with two tunable fields — `NumGuaranteedRolls` (defaults to current party size, read at kill time from `GameState->PlayerArray`, same multiplayer-aware pattern as the P2 targeting fix) and `BonusRollChance` (flat chance at +1 extra roll) — rather than hardcoding "roll once" like the regular tables.
- **Affix system (post-beta, added 2026-08-19)**: content/pool lives in `Docs/Items.md`; technical shape here. An `FDCItemAffix` struct (`FGameplayTag AffixType` + `float RolledValue`) applied as a `GameplayEffect` modifier — same mechanism `UDCItemConfig` already uses for stat modifiers above, just parameterized instead of fixed per item. The affix pool is a `DataTable` (`DT_DC_ItemAffixes`), not hardcoded, matching `GameDevPlan.md` §4.4's data-driven-everything philosophy — design can add/tune affixes without a recompile. Affixes roll server-side at drop-table-roll time (when the loot is generated), not on pickup — keeps rolls deterministic and debuggable per-seed, same reasoning as the existing seeded drop-table design. Uniques (`Docs/Items.md`) are a `bIsUnique` flag on `UDCItemConfig` that skips the normal affix roll and applies a hardcoded bespoke `GameplayEffect`/behavior instead.
- **Skill-gated containers** (new, per the skill/progression brainstorm in `Docs/Classes.md`): a lockable container carries `RequiredSkill` (`FGameplayTag`) + `RequiredSkillLevel` (`int32`); locked until the opening player's tracked skill level meets the requirement. Skill levels live in a new replicated `TMap<FGameplayTag, int32>` on `ADCPlayerState` — profile-scope progression, persists via §7's save system, not run-scope. The actual skill list is a living, ongoing design effort (`Docs/Classes.md`), not fixed here.

## 7. Progression & saves

Two separate, non-overlapping save scopes — don't let them blur:

- **Profile save** (persistent across runs): cosmetic unlocks, meta-currency, class/spec unlocks (post-beta), skill levels (§6). One `USaveGame` per player profile, with a `SaveVersion` (`int32`) field from day one — the beta needs no migration logic yet, but adding the field now is free and expensive to retrofit later. Autosave trigger: the Run-End screen only, since nothing changes profile-state mid-run in beta scope.
- **Run state** (ephemeral, in-memory only for the beta): the current dungeon's seed, room graph, and per-player loot-carried-this-run. **No mid-run save/resume for the beta** — this is a deliberate design choice, not a cut corner: it preserves the extraction *tension* pillar (§3 in `GameDevPlan.md`) that a failed run has real stakes. Revisit only if playtesting says runs need to be pause-and-resume-able, which would be a pillar-level conversation, not a quiet architecture change.

Death handling (co-op, no permadeath for the beta): a downed player can be revived by a teammate within the run; a solo-downed player with no teammate available fails the run (matches the co-op-PvE decision — no permanent character loss, the *run's* loot is what's at stake, not the character).

## 8. UI/UX flow

Wireframe-level layout for every screen (exact components, positions, Blueprint-build steps): `Docs/UIUX.md`. This section stays flow-level — which screen leads to which, and save-scope behavior.

`Main Menu (title screen: New/Continue/Quit) → Hub World (persistent shared space, see Docs/GameplayLoops.md — loadout/vendor/dummies/portal are physical interactables, not menu buttons) → [interact Portal: Host/Join direct-IP] → Dungeon HUD (per-player health/stamina bars, objective tracker, full-screen map toggle — see Docs/GameplayLoops.md's fog-of-war design, replaces the earlier "minimap cut" note) → Run-End screen (extract success/fail, loot summary — success keeps all run-carried loot, failure loses it; profile-scope unlocks/skill-levels persist regardless of run outcome, a separate save scope from run loot) → back to Hub.`

**Settings/Options menu** (previously absent from the flow entirely): reachable from Main Menu and as a non-pausing Dungeon HUD overlay — co-op action combat never pauses in real-time for one player, matching `zombieshooter`'s explicit precedent. Beta contents: volume/audio placeholder sliders, key/gamepad-rebind stub (see §9's controller-support decision), Leave-run action.

**Error/disconnect states** (previously unaddressed): host-disconnect toast + return to Main Menu (§3.1), join-failure message, generic connection-lost fallback.

**Downed/revive UI**: screen-edge desaturation + revive prompt + countdown. Spec: 60s downed duration before a solo player's run fails; revive requires a teammate within ~150uu channeling ~3s; a downed player crawls slowly, has no abilities, and is still finishable by a fresh hit (matches the ported `zombieshooter` state machine's "already downed = finishing blow," `SystemsDesign.md` §10's reuse audit).

Every HUD/menu widget is a dedicated `UDCUserWidgetBase` subclass (native `BindWidget` + `NativeConstruct` wiring, Blueprint side is layout/Class-Defaults only) — mirrors `zombieshooter`'s B1 UI convention, which the dev already validated works well for a solo-dev Claude-Code-assisted pace.

## 9. Camera & controls

Third-person, over-the-shoulder — `GameDevPlan.md`'s header explicitly says "third person," which resolves what could otherwise read as ambiguous against the Diablo-4 comparison (Diablo 4 is isometric; the visual/control reference is Dark and Darker's camera, not Diablo's). Reuse the existing stock template's `DungeonCatCharacter` camera-boom setup as the starting point rather than building a camera system from scratch — it's already third-person and already in the repo.

FOV 90. No hard target-lock — a soft auto-face-toward-nearest-target-in-cone on attack-input instead, readable without lock-on system complexity (and works equally well on controller, see below). Standard spring-arm collision, no custom camera-collision system needed for grey-box. **Extended 2026-08-16 to every class, not just Knight's melee**: no ability on any class ever uses manual aiming or target-cycling — ranged/curse/lob abilities auto-resolve a target the same way (nearest valid enemy in a forward cone, nearest ally or self, or a fixed lobbed point ahead). Full rule and per-ability targeting: `Docs/Classes.md`'s "Ability-slot philosophy" section.

**Controller support, planned from the start** (2026-08-13 decision — overrides an earlier draft recommendation to cut it for the beta; that was `zombieshooter` precedent, not this project's call). Enhanced Input targets both KBM and gamepad from day one, not gamepad-as-an-afterthought. **Full button map pinned 2026-08-16** (previously just "abilities on face buttons, interact on a shoulder button" with no exact assignments):

| Input | Function | Notes |
|---|---|---|
| Left stick | Move | |
| Right stick | Camera | |
| Face South | Slot 1 — basic attack | Free, spammable, tap-vs-hold where the kit uses it (Claw Flurry/Pounce). Same slot-role on every class — always the reflexive "mash" button no matter which class you're playing. |
| Face East | Slot 2 — defensive/panic ability | Zoomies, Hiss, Nine Lives, Groom (`Docs/Classes.md`). Same slot-role on every class on purpose — the panic-button reflex shouldn't depend on which class you picked. |
| Face West | Slot 3 — cost-gated utility ability | Headbutt, Slink, Evil Eye, Purr. |
| Face North | Slot 4 — signature/AoE ability | Whirlwind, Ambush, Hairball, Biscuits. |
| Right bumper | Interact | Same decision as before, now pinned to a specific button. |
| Left bumper | Use consumable | New assignment — `Docs/Items.md`'s beta consumable had no bound input before this pass. |
| Left/right trigger | Reserved, unbound in the beta | The sanctioned home for future expansion (a hold-trigger-plus-face-button chord for an alternate cast, or eventually a 5th slot) — see `Docs/Classes.md`'s "Ability-slot philosophy." Not built now. |
| D-pad | Reserved (Up = map-toggle candidate) | Low-urgency tier, matches the D-pad's poor mid-combat reachability. |
| Start/Menu | Pause / Settings overlay | Matches §8's non-pausing overlay decision. |
| Left stick click | Jump | Inherited from the stock template. **Flagged for the dev**: confirm whether Jump is a real verb in the actual game (a dungeon crawler with no platforming in scope per `GameDevPlan.md`) or should be dropped from the scheme entirely — parked on the least-precious input either way until that's decided. |

This is a real scope addition to the still-blocked Enhanced Input work — `IMC_DC_Default` needs a gamepad-equivalent context alongside the KBM one, not just KBM. Tracked in `Docs/P2_DungeonAI.md`.

## Audio

Policy (unchanged): every `GameplayCue` (§2.5) gets its hook point created — even silent/placeholder — as each ability/system is built, so wiring isn't deferred to a late scramble. Real sound design (actual SFX/music/VO, middleware decision if any) is still a post-beta pass, matching the grey-box-art precedent applied everywhere else. **Creative direction added 2026-08-19** (still no actual SFX/music decided, just the direction future work should aim at):

- **Tone**: playful, adventurous, cartoon-bright — not horror, not grim/dread. Matches the "readable cat power fantasy" pillar (`GameDevPlan.md` §3) and the cartoon art style; sound should reinforce clarity and snap, not mood.
- **Music** (post-beta, direction only): two functional beats needed eventually — a calm "home base" Hub theme, and a Dungeon/combat theme that's tense in the extraction sense, not oppressive/horror-tense.
- **GameplayCue category checklist** (hook points to create now, even silent, per the existing policy):
  - Attack-impact — per weapon type (`AssetPipeline.md`'s weapon-vs-ability animation split applies here too: one impact cue per weapon type, not per class).
  - Ability-cast — per specific ability.
  - Enemy-hit/death — per archetype (`Docs/Bestiary.md`).
  - Boss telegraph "sting" — the audio half of every boss's ~0.5-1s wind-up telegraph.
  - UI — menu navigation, vendor purchase, error/disconnect toasts.
  - Ambient loops — per room type/dungeon zone.
  - Footsteps — per surface material.

## Customization (post-beta, added 2026-08-19)

Turns `GameDevPlan.md` §4.3's brief mention into a buildable spec. Content (fur colors/patterns, accessory list, size-slider bounds): `Docs/Items.md`.

- **Fur color/pattern**: material-instance-parameter driven — one base material with color/pattern-mask parameters, swapped per `MI_DC_Cat_<Variant>` instance. Straightforward, not a forked decision.
- **Accessories**: modular meshes on existing or new small sockets (head, neck) — same socket-attachment pattern already used for weapons (`AssetPipeline.md` §3). Straightforward.
- **Size — FLAGGED FOR DEV, not resolved**: `GameDevPlan.md` §4.3 already posed this as an either/or without deciding — **blend-shape morph target** vs. **controlled bone-scaling**. Real technical fork: affects animation retargeting and how tolerant the collision capsule needs to be across the size range. Needs the dev's call before implementation, not decided here.
- **Gear-fitting — FLAGGED FOR DEV, not resolved**: same doc, same either/or — **skeletal mesh merge** (gear meshes combined into the base mesh at runtime) vs. **Leader Pose Component** (gear stays separate meshes following the base's animation). A perf/complexity tradeoff, not a creative one — needs the dev's call.

## Boss design

Full boss content (identity, phases, telegraphs, arena — Swarm-mother plus the post-beta roster) moved to `Docs/Bestiary.md`, matching this doc's existing content-vs-architecture split (§2.6/§6 do the same for classes/items). This section stays technical: every boss defaults to pure StateTree, no BT escape hatch, per `GameDevPlan.md` §4.5 — reach for the embedded-BT escape hatch only if a specific boss's implementation actually proves unwieldy at pure-StateTree, which hasn't happened for any boss designed so far (all are 2 phases / a handful of attack patterns, squarely inside what §4.5 already says StateTree handles cleanly). Boss-gates-objective structural rule (every boss is a mandatory, never-optional gate immediately before its mode's final objective): `Docs/GameplayLoops.md`.

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
- ~~Dungeon generation technique~~ — **resolved 2026-08-19**, see §4. Fixed hand-authored v1 layout for the beta; real procedural generation deferred post-beta (`ProductionPlan.md`).

# Asset Pipeline (Blender → UE5.8)

Living reference for self-authored art, per `GameDevPlan.md` §5. Update this as the pipeline gets exercised for real — the first few exports will surface gotchas worth recording here (mirrors `zombieshooter`'s `Docs/BlenderNotes.md` role: a memory bridge since Blender-side MCP sessions don't share context with this project's sessions). The goal of this doc is to let art and systems work happen **in parallel** — these specs exist so a model built to spec today still fits cleanly once the corresponding system is built later.

---

## 1. Unit scale & export convention

- Blender scene units: **Metric, Unit Scale = 1.0** (model at real-world meters — Blender's default).
- UE5 expects 1 unit = 1cm. Let FBX export handle the conversion: **FBX export scale = 100**, not a manual scale-up of the mesh in Blender. Do not bake a 100x scale into the mesh data itself — keep Blender-side dimensions real-world (a ~1m-tall cat is `~1.0` in Blender units), apply the FBX scale factor at export time only.
- FBX export settings: Forward = `-Y`, Up = `Z` (UE5 defaults), **Apply Transforms** checked, **Apply Scalings = FBX All**. Freeze/apply all object transforms (Ctrl+A → All Transforms) before export so rotation/scale aren't hiding in the object data.
- Lock this convention before the first real export and don't deviate per-asset — a mixed-scale asset library is expensive to untangle later (this is the exact lesson `zombieshooter`'s `Docs/BlenderNotes.md` exists to avoid re-learning).

## 2. Master cat skeleton

Per `GameDevPlan.md` §2.2/§4.3: **bipedal, UE5-Mannequin-compatible** so the humanoid animation library (Lyra, marketplace ARPG packs, Mixamo) retargets onto it via IK Rig. Build the skeleton to mirror the UE5 Manny/Quinn bone names and hierarchy as closely as the cat's proportions allow:

```
root
└─ pelvis
   ├─ spine_01 → spine_02 → spine_03
   │  └─ neck_01 → head
   │     └─ (ears: ear_l, ear_r — cat-specific, not in the human rig)
   │  ├─ clavicle_l → upperarm_l → lowerarm_l → hand_l (paw — see note below)
   │  └─ clavicle_r → upperarm_r → lowerarm_r → hand_r
   │  └─ tail_01 → tail_02 → tail_03 → tail_04 (cat-specific — root off spine_01 or pelvis, pick whichever gives better secondary-motion sway; verify in-engine before finalizing)
   ├─ thigh_l → calf_l → foot_l → ball_l
   └─ thigh_r → calf_r → foot_r → ball_r
```

- **Paws, not hands**: cartoon cat paws don't need a 5-finger rig. **Recommended: 2 digit bones per paw** (one opposable-ish digit + one fused-pad bone) — the sword/shield grip-pose test is the reason: 1 bone risks a flat, wrong-reading grip at third-person camera distance, and 2 is enough without needing full finger articulation. Still provisional — confirm against a real grip-pose test once the skeleton exists, this is a recommendation, not a verified fact.
- **Ears and tail are the two cat-specific additions** to an otherwise-standard humanoid hierarchy — everything else should map 1:1 to Manny bone names so IK Retargeter setup is close to plug-and-play. Ears can be simple 1-bone swing joints; they're a secondary-motion nicety, not gameplay-critical. **Tail root: recommended `spine_01`** (not `pelvis`) — lets the tail inherit spine twist/lean during combat (Pounce's leaping thrust, Claw Flurry's rotational swings), which should read as better combat flourish than a pelvis-rooted tail. Same caveat: provisional, verify in-engine once there's a skeleton to test against.
- Target proportions: gameplay-readable silhouette at third-person camera distance, roughly **100-110cm** at head height (upright). Exact number is an art call — the constraint is "reads clearly as a small predator" (pillar 1 in `GameDevPlan.md` §3), not a specific number.

## 3. Socket conventions

| Socket | Parent bone | Purpose |
|---|---|---|
| `Socket.WeaponHandR` | `hand_r` | Primary weapon (sword, staff) |
| `Socket.WeaponHandL` | `hand_l` | Off-hand — shield for Knight, but generalizes to a second offensive weapon for dual-wield classes (Rogue's daggers, `Docs/Classes.md`/`Docs/Items.md`) — don't assume "shield" is the only use once post-beta classes get built. |
| `Socket.WeaponBack` | `spine_03` | Sheathed/holstered weapon when not equipped |
| `Socket.Camera` (reference only, not a spring-arm attach point) | `head` or `spine_03` | Reference for camera-boom target height during setup, not a hard attach |

Match these names exactly once C++ starts referencing sockets by name (`GetSocketLocation(TEXT("Socket.WeaponHandR"))` etc.) — a socket rename after code depends on it is a find-and-replace across every reference, avoidable by fixing names now.

## 4. Animation list (Knight vertical slice)

Per `SystemsDesign.md` §2.3's four abilities plus core locomotion. This is the concrete list to animate (or source/retarget) first — everything else is post-slice:

**Weapon-consistent styling, not literal animal-behavior replication.** The Knight fights with a real sword and shield (§3's socket conventions), so the cat-flavored ability names are expressed through *how* an otherwise-normal weapon technique is posed and styled — stance, wind-up, secondary motion (ears/tail), slash-trail shape — not by dropping the weapon for bare-paw animations on every move. **Bunny Kick is the one deliberate exception** (2026-08-12, dev-confirmed): a genuine character beat where the cat's instinct breaks through its knight training — weapon held loose or briefly sheathed, claws out, a scrappy all-limbs flurry. That's intentional characterization, not an inconsistency — don't extend the same "drops the weapon" treatment to the other three abilities without deciding to on purpose.

- **Locomotion**: Idle, Walk/Run (8-way directional or a blend space — match whatever the existing `Variant_Combat` locomotion setup already does, don't invent a second convention), Jump start/fall/land.
- **Death**: one death animation is enough for the beta.
- **Hit reacts**: light and heavy, front only is acceptable for the beta (directional hit reacts are a polish pass, not slice-critical).
- **Downed / Revive**: a downed loop + a get-up animation (co-op revive, per `SystemsDesign.md` §7).
- **Claw Flurry combo** (tap): 3 montage sections — **Swipe → Rake → Shred** — a sword-slash combo with diagonal/raking swing arcs (slash-trail VFX can read as claw marks) rather than straight cuts. One clip, sectioned, not 3 separate clips, matches how AnimNotify-driven combos are typically authored in the existing `Variant_Combat` pattern.
- **Pounce** (hold-to-charge heavy, same ability/hotkey as Claw Flurry, not a separate one): a leaping sword-lunge — charge-loop section (crouch/coil wind-up, loops while held) + a release section (the leaping thrust), reusing `CombatEnemy`'s existing `ChargeLoopSection`/`ChargeAttackSection` montage-section pattern (P0 audit). The cat-specific part is the coiled crouch-wind-up, not the strike itself — weapon stays drawn throughout.
- **Headbutt** (Shield Bash): a shield-bash — weapon and shield stay equipped and doing the work, the head just dips forward through the motion, layered on top of a normal shield-charge rather than replacing it. One montage, notify-tagged for the stagger-application frame.
- **Zoomies** (Dash): pure mobility, no weapon interaction — weapon rides along sheathed or held. One montage, notify-tagged for the invulnerability window start/end (see `SystemsDesign.md` §2.3 — the i-frame window is driven by a notify, not a timer, so the notify placement is gameplay-load-bearing, not just cosmetic).
- **Bunny Kick** (Whirlwind): the deliberate "instinct breaks through" exception above — weapon held loose or momentarily sheathed, claws out, an all-limbs flurry kicking in every direction, not a spinning weapon attack. One montage, AoE-radius **250uu** (confirmed in code, `UDCGameplayAbility_Whirlwind::Radius`, 2026-08-13 — this was previously listed as "coordinate the number," now synced).

Retargeted humanoid locomotion (from Lyra/marketplace/Mixamo per the plan) can fill in anything not listed above (turn-in-place, strafing, etc.) — only the list above needs hand-authored, cat-specific animation for the vertical slice.

### Weapon-attack vs. ability-attack animation split (added 2026-08-19)

Clarifies scope for the Rogue/Wizard/Healer lists below and going forward: **basic/weapon attacks are authored per weapon type, not per class.** A dual-dagger swing belongs to "dual daggers" as an asset, reusable by any class that ever wields dual daggers — it isn't hand-tied to Rogue specifically, even though Rogue is currently the only class that uses them. Each class is locked to exactly one weapon type (`Docs/Items.md`'s weapon table — no class ever swaps weapon types), so in practice each weapon-type animation set maps to one class today, but scoping the *asset* by weapon rather than by class keeps it inherently shareable if that ever changes. The other 3 ability slots per class (plus Slot 1's charged-heavy half, e.g. Pounce) are always bespoke, hand-authored per specific ability, never shared across classes.

Naming reflects the split: weapon-attack montages are `AM_DC_<WeaponType>_BasicAttack` (`AM_DC_SwordShield_BasicAttack`, `AM_DC_DualDagger_BasicAttack`, `AM_DC_Staff_BasicAttack`, `AM_DC_ClawGauntlet_BasicAttack`); ability montages stay `AM_DC_<Class>_<Ability>` (`AM_DC_Knight_Headbutt`, `AM_DC_Rogue_Ambush`, etc.) per §6's convention. Knight's existing animation list above predates this split (authored under the old per-class-only convention) — rename its basic-attack montage to `AM_DC_SwordShield_BasicAttack` when animation work actually starts; nothing else about Knight's list changes.

### Rogue animation list (dual daggers)

Locomotion, death, hit reacts, and downed/revive reuse the same shared base set as Knight (no re-authoring) — only the ability-specific list below is new:

- **Quickclaw** (basic/weapon attack, tap combo) — `AM_DC_DualDagger_BasicAttack`: a 2-hit dagger flurry (Slash1 → Slash2 sections), 0.8s re-tap window — tighter than Claw Flurry's 1.2s, reinforcing "faster, lighter."
- **Slink** (ability) — `AM_DC_Rogue_Slink`: a stance-shift into a low prowling loop while active, cleanly interruptible back to normal stance on attack or duration end.
- **Ambush** (ability) — `AM_DC_Rogue_Ambush`: a lunging dagger thrust. One montage is enough for the beta; a distinct rear-strike flourish (vs. a Slink-triggered pounce-lunge) is a natural later polish split, not required now.
- **Hiss** (ability) — `AM_DC_Rogue_Hiss`: a quick defensive dual-dagger flourish — a wide slashing flinch-burst that reads as pure technique. Rogue doesn't get a Bunny-Kick-style "instinct breaks through" exception; the daggers stay the identity throughout, fully weapon-consistent.

### Wizard animation list (staff/orb)

Locomotion, death, hit reacts, and downed/revive reuse the shared base set — only the ability-specific list below is new:

- **Jinx** (basic/weapon attack) — `AM_DC_Staff_BasicAttack`: a quick staff-point/orb-thrust cast gesture, fast enough to match its ~0.5s innate swing timer.
- **Evil Eye** (ability) — `AM_DC_Wizard_EvilEye`: a more deliberate two-handed staff-plant/orb-raise gesture with a visible telegraph, matching its control-ability read.
- **Hairball** (ability) — `AM_DC_Wizard_Hairball`: the deliberate joke exception (mirrors Bunny Kick's role for Knight, per `Docs/Classes.md`'s "the one joke ability" framing) — an exaggerated coughing/hacking wind-up into a lob-throw release.
- **Nine Lives** (ability) — `AM_DC_Wizard_NineLives`: a brief self-focused channel pose, staff/orb raised protectively — reads as "warded" instantly, no long wind-up.

### Healer animation list (claw-gauntlets or light mace)

Locomotion, death, hit reacts, and downed/revive reuse the shared base set — only the ability-specific list below is new:

- **Swat** (basic/weapon attack) — `AM_DC_ClawGauntlet_BasicAttack`: a quick, low-committal paw-swat/gauntlet-jab — deliberately the lightest-reading swing in the roster, matching `Docs/Classes.md`'s "deliberately the lightest basic" note.
- **Purr** (ability) — `AM_DC_Healer_Purr`: a self-centered settle-and-pulse pose, a brief purring-idle beat with a radiating pulse cue.
- **Groom** (ability) — `AM_DC_Healer_Groom`: a grooming/licking-paw gesture directed at the target ally (or self). Deliberately leans into literal cat behavior rather than weapon-consistent styling — Healer's identity ("purr and groom support") calls for this more than Knight's weapon-fighter identity does, unlike Rogue/Wizard above.
- **Biscuits** (ability) — `AM_DC_Healer_Biscuits`: a kneading-paw rally stomp — literal to the ability's real-world cat-behavior name, same reasoning as Groom.

## 5. Dungeon trim-kit spec

**⚑ Provisional — dungeon generation technique reopened 2026-08-13** (`SystemsDesign.md` §4, `Docs/P2_DungeonAI.md`). The grid/room-catalog content below assumes the previously-planned prefab-room graph-stitching approach, which is no longer a settled decision. Trim-sheet/material guidance (texture atlas, wall height as a general modeling convention) is likely still useful regardless of technique; the specific room catalog is not — don't build against it yet.

Per `SystemsDesign.md` §4.1's 400uu grid:

- **Base grid unit**: 400uu (4m). All room modules and props snap to this grid.
- **Wall height**: 300-400uu — pick one and hold it across every room module (a room that's taller than its neighbors breaks the door-socket-snapping assumption in §4.1).
- **Door socket**: consistent width and floor-level position on every room edge that has one, so any two modules' doors align regardless of which specific modules they are. Standardize the door opening size once (e.g. 200uu wide x 300uu tall) and reuse it everywhere.
- **Trim sheet**: one shared texture atlas (2048x2048 to start) for wall/floor/trim materials across the whole dungeon kit — this is what keeps both modeling effort and draw calls down at art-pipeline scale (`GameDevPlan.md` §5). Don't make a bespoke material per room piece.
- **Room catalog**: start with a small set — one 10x10-tile combat room, one 2-tile-wide x N-tile corridor segment (straight + one turn piece), one entry room, one exit room, one boss arena (larger, say 14x14). This is enough variety for the 6-10 room MVP chain (`GameDevPlan.md` §7) — expand the catalog only after the generator itself works with a minimal set.

## 6. Asset naming convention

Matches `SystemsDesign.md` §1's `DC` prefix:

| Type | Pattern | Example |
|---|---|---|
| Skeletal mesh | `SK_DC_<Name>` | `SK_DC_Cat_Body` |
| Static mesh | `SM_DC_<Category>_<Name>` | `SM_DC_Wall_Straight01` |
| Texture | `T_DC_<Name>_<Channel>` | `T_DC_TrimSheet_Dungeon01_BC` |
| Material / instance | `M_DC_<Name>` / `MI_DC_<Name>` | `M_DC_TrimSheet` / `MI_DC_TrimSheet_Damp` |
| Animation sequence | `A_DC_<Subject>_<Action>` | `A_DC_Cat_Idle` |
| Anim montage — ability | `AM_DC_<Class>_<Ability>` | `AM_DC_Knight_Headbutt` |
| Anim montage — basic/weapon attack | `AM_DC_<WeaponType>_BasicAttack` | `AM_DC_SwordShield_BasicAttack` |
| Data asset | `DA_DC_<Category>_<Name>` | `DA_DC_EnemyConfig_MeleeChaser` |
| Blueprint | `BP_DC_<Name>` | `BP_DC_Container_Chest` |
| Widget blueprint | `WBP_DC_<Name>` | `WBP_DC_HUD_Main` |

Anim montages split into two rows above per §4's "Weapon-attack vs. ability-attack animation split" — basic attacks are scoped/named by weapon type (shareable across any class using that weapon), specific abilities stay scoped/named by class.

## 7. Off-limits / gotchas (fill in as they're hit)

Empty for now — this section exists so the first real Blender→UE5 export session has somewhere to record what actually went wrong, the same way `zombieshooter`'s equivalent doc accumulated real lessons over time. Don't leave it empty out of neglect once exports start happening.

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

- **Paws, not hands**: cartoon cat paws don't need a 5-finger rig. One or two simplified digit bones per paw (`hand_l` → `thumb_01_l`-equivalent at minimum) is enough for gear/weapon grip poses to look right — full finger articulation is effort the beta doesn't need. Confirm against a grip-pose test (holding a sword/shield) before deciding how many digit bones are actually necessary.
- **Ears and tail are the two cat-specific additions** to an otherwise-standard humanoid hierarchy — everything else should map 1:1 to Manny bone names so IK Retargeter setup is close to plug-and-play. Ears can be simple 1-bone swing joints; they're a secondary-motion nicety, not gameplay-critical.
- Target proportions: gameplay-readable silhouette at third-person camera distance, roughly **100-110cm** at head height (upright). Exact number is an art call — the constraint is "reads clearly as a small predator" (pillar 1 in `GameDevPlan.md` §3), not a specific number.

## 3. Socket conventions

| Socket | Parent bone | Purpose |
|---|---|---|
| `Socket.WeaponHandR` | `hand_r` | Primary weapon (sword, staff) |
| `Socket.WeaponHandL` | `hand_l` | Off-hand (shield) |
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
- **Bunny Kick** (Whirlwind): the deliberate "instinct breaks through" exception above — weapon held loose or momentarily sheathed, claws out, an all-limbs flurry kicking in every direction, not a spinning weapon attack. One montage, AoE-radius should visually match whatever radius the ability's GameplayEffect actually uses — coordinate the number with whoever tunes the ability data asset.

Retargeted humanoid locomotion (from Lyra/marketplace/Mixamo per the plan) can fill in anything not listed above (turn-in-place, strafing, etc.) — only the list above needs hand-authored, cat-specific animation for the vertical slice.

## 5. Dungeon trim-kit spec

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
| Anim montage | `AM_DC_<Subject>_<Action>` | `AM_DC_Knight_BasicAttack` |
| Data asset | `DA_DC_<Category>_<Name>` | `DA_DC_EnemyConfig_MeleeChaser` |
| Blueprint | `BP_DC_<Name>` | `BP_DC_Container_Chest` |
| Widget blueprint | `WBP_DC_<Name>` | `WBP_DC_HUD_Main` |

## 7. Off-limits / gotchas (fill in as they're hit)

Empty for now — this section exists so the first real Blender→UE5 export session has somewhere to record what actually went wrong, the same way `zombieshooter`'s equivalent doc accumulated real lessons over time. Don't leave it empty out of neglect once exports start happening.

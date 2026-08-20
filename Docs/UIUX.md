# UI / UX

Living reference for every screen's wireframe-level layout — exact components, exact positions, exact Blueprint-build steps. For the overall flow between screens and save-scope behavior, see `SystemsDesign.md` §8 — that section stays flow-level, this one owns per-screen layout.

Ongoing doc, first-draft, same status as everything else here — positions/sizes below are a concrete starting point to build against, not pixel-locked final values.

---

## Shared rule: functionality is native, Blueprint is layout/styling only

Every screen extends a dedicated native `UDCUserWidgetBase` subclass (`SystemsDesign.md` §8) — **all logic lives in C++** (attribute-delegate bindings, button click handlers, data population, `NativeConstruct`/`NativeTick`). The Widget Blueprint's Event Graph stays empty beyond the default. This isn't a per-screen choice, it's the rule for all of them, so the build steps below are the same shape every time:

1. **Native side** declares one `BindWidget` (or `BindWidgetOptional` for conditionally-visible elements) property per component listed in that screen's table, and does all wiring/logic in C++.
2. **Editor**: Content Browser → User Interface → Widget Blueprint, parent class = that screen's native class (named per screen below), save as the listed `WBP_DC_*` name (`AssetPipeline.md` §6 convention).
3. Add a root **Canvas Panel**.
4. Add one child widget per row in the table, **named to exactly match its native `BindWidget` property** — UMG's name-matching is the mechanism that lets native code find and drive the widget without any BP graph wiring.
5. Set each child's **Anchor / Alignment / Position / Size** per the table (Details panel → Slot (Canvas Panel Slot)).
6. **Style only** in the Appearance section (fonts, colors, bar/icon textures) — never add Event Graph nodes beyond the inherited default.
7. Compile/Save.

Each screen below gives only what's specific to it: the diagram, the component table, the native class name, and the WBP name — the 7 steps above are identical every time and aren't repeated per screen.

---

## Main Menu

```
┌──────────────────────────────────────┐
│                                        │
│              [ DUNGEON CAT ]          │
│                                        │
│              [ New Game ]             │
│              [ Continue ]             │
│              [ Settings ]             │
│              [ Quit ]                 │
│                                        │
│ v0.1.0                                │
└──────────────────────────────────────┘
```

| Element | Anchor | Alignment | Position | Size |
|---|---|---|---|---|
| Title/logo | (0.5, 0.3) | (0.5, 0.5) | (0, 0) | auto |
| Button stack (New/Continue/Settings/Quit) | (0.5, 0.55) | (0.5, 0) | (0, 0) | 280×220 |
| Version text | (0, 1.0) | (0, 1.0) | (16, -16) | auto |

Native class: `UDCUserWidgetBase_MainMenu`. WBP: `WBP_DC_MainMenu`.

---

## Hub HUD

Minimal — the Hub is a safe, non-combat space, so no health/stamina bars. Just a context-sensitive interact prompt when near an interactable (Loadout/Vendor/Portal/Trophy Board/Cosmetics Mirror, `GameplayLoops.md`).

```
┌──────────────────────────────────────┐
│                                        │
│                                        │
│              [E] Interact             │
│           "Open Loadout"              │
│                                        │
└──────────────────────────────────────┘
```

| Element | Anchor | Alignment | Position | Size |
|---|---|---|---|---|
| Interact prompt (icon + label) | (0.5, 0.75) | (0.5, 0.5) | (0, 0) | auto, visible only in range |

Native class: `UDCUserWidgetBase_HubHUD`. WBP: `WBP_DC_HUD_Hub`.

---

## Loadout

```
┌──────────────────────────────────────┐
│  [Class Portrait]     Knight          │
│                        "Tank-and-cleave"│
│                                        │
│  Abilities: [S][E][W][N]              │
│  Mastery: Shield Mastery [T1|T2|T3]   │
│  Spec: ( ) Bulwark   ( ) Blitz        │
│                                        │
│                       [Confirm] [Back]│
└──────────────────────────────────────┘
```

| Element | Anchor | Alignment | Position | Size |
|---|---|---|---|---|
| Class portrait/preview | (0, 0.5) | (0, 0.5) | (48, 0) | 400×600 |
| Class name/identity text | (0.5, 0.1) | (0.5, 0) | (0, 0) | auto |
| Ability slot row (4 icons + labels) | (0.5, 0.45) | (0.5, 0.5) | (0, 0) | 500×100 |
| Mastery-line tier display | (0.5, 0.6) | (0.5, 0) | (0, 0) | auto |
| Spec fork toggle | (0.5, 0.7) | (0.5, 0) | (0, 0) | auto |
| Confirm/Back buttons | (1.0, 1.0) | (1.0, 1.0) | (-32, -32) | auto |

Native class: `UDCUserWidgetBase_Loadout`. WBP: `WBP_DC_Loadout`. Beta note: only Knight exists, so class-select itself is a stub — the screen is still built to this full spec so post-beta class selection slots in without a rebuild (`GameplayLoops.md`'s existing note).

---

## Vendor

```
┌──────────────────────────────────────┐
│ [Vendor Portrait]      Currency: 120  │
│                                        │
│  [Item][Item][Item][Item]             │
│                                        │
│  Selected: "Heal Potion" - 20g        │
│  [Buy]                                │
│                                        │
│                              [Close]  │
└──────────────────────────────────────┘
```

| Element | Anchor | Alignment | Position | Size |
|---|---|---|---|---|
| Vendor portrait | (0, 0) | (0, 0) | (32, 32) | 220×220 |
| Currency display | (1, 0) | (1, 0) | (-32, 32) | auto |
| Item grid (4-wide row) | (0.5, 0.4) | (0.5, 0.5) | (0, 0) | 480×120 |
| Item detail panel | (0.5, 0.65) | (0.5, 0) | (0, 0) | 400×100 |
| Buy button | (0.5, 0.8) | (0.5, 0) | (0, 0) | auto |
| Close button | (1, 1) | (1, 1) | (-32, -32) | auto |

Native class: `UDCUserWidgetBase_Vendor`. WBP: `WBP_DC_Vendor`.

---

## Settings / Options

Reachable from Main Menu and as a non-pausing Dungeon HUD overlay (`SystemsDesign.md` §8) — the Leave-run button only shows in the overlay context.

```
┌──────────────────────────────────────┐
│              Settings                 │
│                                        │
│  Master Volume  [======----]          │
│  Music Volume   [====------]          │
│  SFX Volume     [========--]          │
│                                        │
│  Keybinds: [Rebind...] (stub)         │
│                                        │
│  [Leave Run]  (only shown mid-run)    │
│                              [Back]   │
└──────────────────────────────────────┘
```

| Element | Anchor | Alignment | Position | Size |
|---|---|---|---|---|
| Title text | (0.5, 0.08) | (0.5, 0) | (0, 0) | auto |
| Volume sliders ×3 (stacked) | (0.5, 0.3) | (0.5, 0) | (0, 0) | 400×140 |
| Keybind-rebind stub | (0.5, 0.55) | (0.5, 0) | (0, 0) | auto |
| Leave-run button | (0.5, 0.7) | (0.5, 0) | (0, 0) | auto |
| Back button | (1, 1) | (1, 1) | (-32, -32) | auto |

Native class: `UDCUserWidgetBase_Settings`. WBP: `WBP_DC_Settings`. The Leave-run button's visibility is a native `bIsMidRun`-driven binding — still positioning/styling only in BP, a Visibility data-binding isn't Event Graph logic.

---

## Dungeon HUD

Ability cluster reads as a paw print pressed into the bottom-left corner: one larger heel pad (the North/signature "ult") closest to the corner, 4 smaller toe pads (South/East/West + the consumable) arcing tightly above it.

```
┌──────────────────────────────────────┐
│ [HP/Stamina: Self]                [Objective]   [Map hint] │
│ [HP/Stamina: Ally 1]                  │
│ [HP/Stamina: Ally 2]                  │
│                                        │
│              (play space)             │
│                                        │
│   o   o                               │
│    o o                                │
│     O          <- heel pad (ult), toes tight above │
└──────────────────────────────────────┘
```

| Element | Anchor | Alignment | Position | Size |
|---|---|---|---|---|
| HealthBar/StaminaBar — Self | (0, 0) | (0, 0) | (24, 24) | 280×48 |
| HealthBar/StaminaBar — Ally 1 | (0, 0) | (0, 0) | (24, 90) | 220×36 |
| HealthBar/StaminaBar — Ally 2 | (0, 0) | (0, 0) | (24, 130) | 220×36 |
| Objective tracker | (0.5, 0) | (0.5, 0) | (0, 24) | auto |
| Map-toggle hint | (1, 0) | (1, 0) | (-24, 24) | auto |
| Ability icon — Ult (North/signature, heel pad) | (0, 1.0) | (0, 1.0) | (110, -55) | 96×96 |
| Ability icon — consumable (outer-left toe) | (0, 1.0) | (0, 1.0) | (30, -120) | 68×68 |
| Ability icon — West/utility (inner-left toe) | (0, 1.0) | (0, 1.0) | (75, -150) | 68×68 |
| Ability icon — East/defensive (inner-right toe) | (0, 1.0) | (0, 1.0) | (150, -150) | 68×68 |
| Ability icon — South/basic (outer-right toe) | (0, 1.0) | (0, 1.0) | (195, -120) | 68×68 |
| Downed/revive overlay | (0.5, 0.5) | (0.5, 0.5) | (0, 0) | full-screen |

Native class: `UDCUserWidgetBase_DungeonHUD`. WBP: `WBP_DC_HUD_Dungeon`.

---

## Full-screen Map

Fog-of-war render (`GameplayLoops.md`) — discovered areas at real layout, undiscovered hidden. Toggled on-demand, not always-on (`SystemsDesign.md` §8).

```
┌──────────────────────────────────────┐
│                                        │
│         [ Dungeon Layout ]            │
│           ● (you)   ▲ (ally)          │
│              ★ (objective)            │
│                                        │
│                          [Close: Tab] │
└──────────────────────────────────────┘
```

| Element | Anchor | Alignment | Position | Size |
|---|---|---|---|---|
| Map render (full layout) | (0.5, 0.5) | (0.5, 0.5) | (0, 0) | 80% of screen, centered |
| Player marker(s) | native world-to-map projection | — | dynamic | 24×24 each |
| Objective/boss marker | native world-to-map projection | — | dynamic | 28×28 |
| Close hint | (0.5, 1.0) | (0.5, 1.0) | (0, -24) | auto |

Native class: `UDCUserWidgetBase_Map`. WBP: `WBP_DC_Map`. Marker positions are computed natively (world-to-screen projection against the discovered-room data) and applied via a native per-marker translation call — BP still only styles the marker icons, never computes placement.

---

## Run-End

```
┌──────────────────────────────────────┐
│         RUN COMPLETE — SUCCESS        │
│                                        │
│  Loot kept:                           │
│   - Heal Potion                       │
│   - +Armor Trinket                    │
│                                        │
│  Skill progress: Nimble Paws +1       │
│                                        │
│                    [Return to Hub]    │
└──────────────────────────────────────┘
```

| Element | Anchor | Alignment | Position | Size |
|---|---|---|---|---|
| Success/Fail banner | (0.5, 0.15) | (0.5, 0.5) | (0, 0) | auto, large text |
| Loot summary list | (0.5, 0.45) | (0.5, 0) | (0, 0) | 400×200 |
| Profile-progress summary | (0.5, 0.7) | (0.5, 0) | (0, 0) | 400×80 |
| Return-to-Hub button | (0.5, 0.92) | (0.5, 1.0) | (0, 0) | auto |

Native class: `UDCUserWidgetBase_RunEnd`. WBP: `WBP_DC_RunEnd`. Content spec (what's shown on success vs. failure): `SystemsDesign.md` §8.

---

## Error / disconnect states

One shared toast widget covers all 3 cases (host-disconnect, join-failure, connection-lost) — native code sets the message text and whether an acknowledge button is required (host-disconnect) vs. auto-dismiss (the other two).

```
┌──────────────────────────────────────┐
│                                        │
│     ┌────────────────────────┐       │
│     │  Host disconnected.     │       │
│     │  Returning to Main Menu │       │
│     │           [OK]           │       │
│     └────────────────────────┘       │
│                                        │
└──────────────────────────────────────┘
```

| Element | Anchor | Alignment | Position | Size |
|---|---|---|---|---|
| Toast/banner container | (0.5, 0.5) | (0.5, 0.5) | (0, 0) | 400×140 |
| Message text | (0.5, 0.5) | (0.5, 0.5) | (0, -20) | auto |
| Acknowledge button (host-disconnect only) | (0.5, 0.5) | (0.5, 0.5) | (0, 30) | auto |

Native class: `UDCUserWidgetBase_ConnectionToast`. WBP: `WBP_DC_ConnectionToast`.

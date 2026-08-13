# Gameplay Loops

Living reference for the game's core loop(s) and the Hub World that houses them — the creative/content "what." For technical architecture (session model, replication, save scopes), see `SystemsDesign.md` §3/§7/§8.

Ongoing doc — start here, keep adding as more modes get designed post-beta.

---

## The Hub World — "The Den" (working name, trivially renameable)

A persistent, shared, hand-authored 3D space players spawn into and return to between runs — not a menu screen. Replaces the earlier "Main Menu → Lobby" concept: the Hub *is* the lobby, the same way Destiny's Tower or Warframe's ship work. Players see each other walking around in it before committing to a run, rather than a separate menu-only lobby step. Grey-box for the beta, same as everything else right now.

**Contents:**
- **Spawn point** — where players land after the title screen.
- **Loadout station** — an interactable (`UDCInteractableComponent`) that opens the Loadout UI screen. Beta: fixed Knight, no real choice yet, but the screen is built so post-beta class selection slots in without a rebuild.
- **Vendor** — one for the beta, selling the beta's small consumable/trinket list (`Docs/Items.md`). Additional vendor types are a post-beta expansion.
- **Training dummies** — reuse `CombatDummy` (`Source/DungeonCat/Variant_Combat/Gameplay/CombatDummy.h`, already exists in the stock scaffold, confirmed reusable in the P0 audit) directly. Lets players test Claw Flurry/Pounce/Headbutt/Zoomies/Bunny Kick risk-free before a run.
- **Portal / run-start point** — interacting opens Host/Join (direct-IP text field, `SystemsDesign.md` §3.1). Starting the run transitions the party from the shared Hub into the Dungeon level together.

---

## Get-item-escape (the beta's only loop)

The full moment-to-moment loop:

**Hub** → interact Portal (Host/Join) → **Dungeon** (explore, fight through enemy encounters, pick up loot along the way, map reveals only what's been explored — fog-of-war, see below) → reach the **final objective**, guarded by the **final boss** (mandatory gate, never optional or avoidable — see "Boss-gates-objective," below) → defeat the boss → **Extract** via the exit while carrying the objective → **Run-End screen** (success: loot summary of what's kept; failure: loot summary of what's lost) → back to **Hub**.

Pillar target: 8-12 minutes per run (`GameDevPlan.md` §3).

### Boss-gates-objective (structural rule, not just this mode)

The final boss is a mandatory gate immediately before the final objective, in **every** game mode, not just Get-item-escape — "almost like Raids in WoW." This replaces the earlier (incorrect) "survive the boss or avoid it" framing in `ProductionPlan.md` P4, which contradicted a combined Objective/Boss room design. Recorded here as a general mode-design rule so it doesn't need re-deciding per future mode: Kill-boss's whole point *is* the boss; Rescue and Combo presumably gate their own objectives behind a boss too, once designed.

### Map / fog-of-war

A full-screen map (beta: no always-on minimap, keeps scope tight) showing only areas the party has actually explored — everything else stays blacked out. Technical concept, deliberately independent of the still-undecided dungeon-generation technique: each room/area carries a replicated "discovered" flag, set true the first time any party member enters it; the map renders discovered areas at their real layout and undiscovered areas as hidden shapes. Works whether the eventual dungeon is procedurally generated or hand-authored, so it's safe to design now.

---

## Post-beta modes (named only — `GameDevPlan.md` §6, not designed here)

- **Kill-boss**
- **Rescue**
- **Combo**

Zero design content exists for these yet beyond their names and the boss-gates-objective structural rule above, which will apply to whichever of them has a final-objective structure. Design these when their turn comes, not speculatively now.

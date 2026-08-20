# Blender Notes

Living reference for hand-authored Blender work on DungeonCat's cat character (and whatever else gets modeled by hand later). Mirrors `zombieshooter`'s `Docs/BlenderNotes.md` role, per `AssetPipeline.md` §0: a memory bridge, since Blender-side sessions (watching tutorials, modeling in the Blender app itself) don't share context with this repo's coding sessions. Update this as tutorials get watched and techniques get exercised for real — this is the place to dump hotkeys, gotchas, and "here's what I did differently and why" notes so they survive between sessions.

This doc is technique/reference notes. The locked-down export contract (unit scale, skeleton, sockets, naming) lives in `AssetPipeline.md` — don't duplicate spec decisions here, cross-link instead.

**Working principle (dev-stated, 2026-08-17):** tutorials are learning/building blocks, not steps to follow to the letter — deviate on purpose (different proportions, different topology choices, different color/texture approach, etc.) so the result is an original interpretation, not a clone. Each video log entry below should eventually note what was actually kept vs. changed once the modeling session happens.

---

## 1. Source video log

### "3D Cat - Blender Character Modeling for Beginners | Real-Time Tutorial" — SouthernShotty

- **URL**: https://www.youtube.com/watch?v=C1CFWDWTamo
- **Channel**: SouthernShotty (318K subs)
- **Runtime**: 28:27 — published 2024-01-30, 528K views
- **Logged**: 2026-08-17
- **Description's own framing**: reference image setup → box modeling → detail pass for cuteness → simple texturing for color/personality.
- **Assets the channel points at**: reference image (imgur.com/a/9JpKSDT), free sample packs on Blender Market ("Dynamic VFX Pack", "Crafty Asset Pack" — likely where applied materials in the texturing segment come from), and a ZBrush alpha download (pixologic.com) — likely used as a texture-paint alpha/brush.

**Correction, same session**: the first pass at this entry read the runtime as 3:02 and assumed the video was near-silent — both wrong. 3:02 was the length of the pre-roll ad, not the video; the real video is 28:27 and does have real spoken narration (confirmed via a live caption line that rendered on screen: "...and use that grab brush yet again just to move that there so..."). A full transcript/segment breakdown wasn't achieved this session — YouTube blocks direct caption-file download (empty response from both the legacy `timedtext` endpoint and the internal `get_transcript` API), and live-playback capture wasn't reliable either: the automation session's Browser pane wasn't actually visible on-screen, and YouTube correctly pauses/throttles playback for a backgrounded tab (confirmed via `document.hidden`), which no amount of muting or forcing `.play()` from script got around. The specific per-phase timestamp table that used to be here was built on the wrong 3-minute framing and has been removed rather than left around as misleading data.

**If this video gets revisited**: the reliable path is either (a) the dev pastes the transcript text directly (YouTube's own "Show transcript" panel works fine in a normal signed-in browser — it's specifically the automated/backgrounded session that couldn't get it), or (b) working the Browser pane visibly/focused so real playback isn't throttled. Don't re-attempt the fast-forward-and-poll-captions trick — confirmed unreliable even before the visibility issue (captions desync badly above ~2x and don't recover without a full reload).

**What DungeonCat will do differently**: not yet decided — fill in once an actual modeling session happens (per the working principle above, the point is to deviate on purpose, not to reproduce this cat 1:1).

---

## 2. Hotkey reference (Blender default keymap)

General-purpose reference, not tied to any one tutorial — stock Blender 4.x keymap unless noted. Organized by what you're doing, not alphabetically, so it's scannable mid-task.

### Viewport navigation

| Hotkey | Action |
|---|---|
| Middle Mouse drag | Orbit view |
| Shift+MMB drag | Pan view |
| Ctrl+MMB drag / Scroll wheel | Zoom |
| Numpad 1 / 3 / 7 | Front / Right / Top orthographic view |
| Ctrl+Numpad 1 / 3 / 7 | Back / Left / Bottom orthographic view |
| Numpad 0 | Camera view |
| Numpad 5 | Toggle Perspective ↔ Orthographic |
| Numpad `.` | Frame selected (zoom to fit selection) |
| Home | Frame all |
| Numpad `/` | Toggle local view (isolate selected object) |
| `~` (tilde) | View pie menu (quick ortho-view switch) |

No numpad (laptop)? `Edit → Preferences → Input → Emulate Numpad` maps the top-row number keys to the shortcuts above.

### Selection

| Hotkey | Action |
|---|---|
| Left Click | Select |
| Shift+Click | Add/remove from selection |
| `A` | Select all |
| Alt+A | Deselect all |
| Ctrl+I | Invert selection |
| `B` | Box select |
| `C` | Circle select (scroll to resize brush) |
| `L` (hover, no click) | Select linked — everything connected under the cursor |
| Ctrl+L | Select linked, from current selection |
| Alt+Click on an edge | Select the full edge loop |
| Ctrl+Alt+Click on an edge | Select the full edge ring |
| Shift+Alt+Click | Extend an existing loop/ring selection |
| Shift+G | Select similar (by normal, length, material, etc.) |

### Object Mode

| Hotkey | Action |
|---|---|
| Tab | Toggle Object ↔ Edit Mode |
| `G` / `R` / `S` | Grab (move) / Rotate / Scale — press X/Y/Z after to lock an axis, or type a number for an exact value |
| Shift+D | Duplicate |
| Alt+D | Duplicate **linked** — copies share mesh data, editing one edits all of them |
| Ctrl+A | Apply transform (Location/Rotation/Scale/All) — bakes the transform into the mesh data itself |
| Ctrl+J | Join selected objects into the active one |
| `M` | Move to collection |
| Shift+A | Add menu (mesh, curve, empty, light, etc.) |
| X / Delete | Delete |
| F2 | Rename active object |
| Ctrl+C / Ctrl+V | Copy / paste objects |

### Edit Mode — mesh modeling

| Hotkey | Action |
|---|---|
| `1` / `2` / `3` | Vertex / Edge / Face select mode |
| `E` | Extrude |
| Alt+E | Extrude menu (along normals, individual faces, etc.) |
| `I` | Inset faces |
| Ctrl+R | Loop cut — scroll for more cuts, click to place, drag to slide |
| `K` | Knife tool |
| `J` | Connect selected vertices with a new edge |
| `F` | Fill — makes an edge between 2 verts, or a face from an edge loop |
| `V` | **Rip** — tears the mesh apart at the selected verts so the torn piece can be pulled away. Not the same operation as Separate, even though it visually "separates" geometry. |
| `P` | **Separate** — splits the selection into a new *object* (By Selection / By Material / By Loose Parts) |
| `M` | Merge menu (At Center, At Last, By Distance…) |
| Ctrl+B | Bevel |
| Alt+N | Normals menu — **Recalculate Outside** is the fix for inverted/flipped normals |
| Alt+Z | Toggle X-ray (see/select through the mesh — combine with Box Select to grab both sides of a mirrored mesh at once) |

### Modifiers

| Hotkey | Action |
|---|---|
| Ctrl+1 … Ctrl+5 (hover viewport) | Instantly add/set a **Subdivision Surface** modifier to that level |
| — | Everything else (Mirror, Bevel, Solidify, Armature…) is added via the wrench icon (Modifier Properties) → Add Modifier — no universal hotkey for those |

### UV

| Hotkey | Action |
|---|---|
| Ctrl+E → Mark Seam | Mark selected edges as a UV seam |
| `U` | Unwrap menu — Unwrap, Smart UV Project, Cube/Cylinder/Sphere Projection |

### Shading / materials / texture paint

| Hotkey | Action |
|---|---|
| `Z` | Shading pie menu (Wireframe / Solid / Material Preview / Rendered) |
| `F` (drag, Texture Paint mode) | Adjust brush size |
| Shift+F (drag, Texture Paint mode) | Adjust brush strength |

### Precision & snapping

| Hotkey | Action |
|---|---|
| Type a number after `G`/`R`/`S` | Exact-value transform — e.g. `G` `X` `2` `Enter` moves 2 units on X |
| Shift+X / Shift+Y / Shift+Z after `G` | Move on every axis **except** the one pressed |
| Ctrl (while transforming) | Snap to increments |
| Shift (while transforming) | Precision/slow mode — finer control on mouse movement |
| Shift+S | Snap pie menu (cursor to selected, selection to cursor, etc.) |
| Shift+C | Reset 3D cursor to world origin + frame all |
| `.` (period) | Pivot point pie menu |
| `,` (comma) | Transform orientation pie menu |

### File / general

| Hotkey | Action |
|---|---|
| Ctrl+S | Save |
| Ctrl+Z / Ctrl+Shift+Z | Undo / redo |
| `N` | Toggle the side panel (Item/Transform tab) |
| `T` | Toggle the left toolbar |
| `F3` | Operator search (stock default — some tutorials rebind Spacebar to this instead) |
| Spacebar | Play/pause the timeline, stock default (frequently rebound to Search by tutorials/addons — check `Edit → Preferences → Keymap → Spacebar Action` if a video's Spacebar behavior doesn't match) |

---

## 3. Tools & workflow techniques

- **Box modeling**: start from a primitive (cube), extrude/scale/loop-cut it into the target silhouette rather than sculpting from scratch. Standard for stylized, game-ready low/mid-poly characters — pairs well with the Mirror + Subdivision Surface modifier combo below.
- **Mirror modifier**: model one half of a symmetric character, mirror modifier handles the other half live. Apply once symmetry-breaking detail work starts (asymmetric damage, accessories, etc.) — not needed for the cat's base body/head.
- **Subdivision Surface modifier**: smooths a low-poly cage into a rounder result at render/preview time without permanently increasing the base mesh's vertex count. `Ctrl+1`…`Ctrl+5` is the fast way to dial the level in without hunting through the modifier stack.
- **Multi-viewport reference setup**: manual panel-splitting (right-click border → Vertical/Horizontal Split) to keep viewport, reference image, and outliner/properties visible simultaneously. A saved custom workspace would do the same thing more durably if this setup gets reused often.
- **Material-per-part vs. single texture paint**: assigning a material slot per selected face-group (`Assign` in Edit Mode) is faster than texture-painting a single shared UV layout when parts just need flat/simple coloring — save texture painting for where actual painted detail (fur shading, markings) is worth the UV/paint overhead.
- **Appending assets from another `.blend`**: `File → Append` → pick file → pick the data-block category (e.g. Material) → select → Append. Non-destructive copy-in, doesn't link/depend on the source file staying in place (that would be `File → Link` instead).

---

## 4. Tips & tricks

- **Freeze transforms before export.** `Ctrl+A → All Transforms` bakes rotation/scale/location into the mesh data — required before FBX export per `AssetPipeline.md` §1 ("Apply Transforms" checked, nothing hiding in the object's transform channels). Do this as a matter of habit at the end of a modeling pass, not just right before export.
- Pack painted textures into the `.blend` (`File → External Data → Pack Resources`) before closing, or re-export the painted image externally — otherwise texture-paint work silently reverts on reopen.
- `L` (select linked) and `Alt+Click`/`Shift+Alt+Click` (select loop) are fast alternatives to box-selecting when working on a mesh with reference geometry in the way.
- **Statistics overlay** (`Viewport Overlays → Statistics`, top-right of the viewport) shows a live vert/face/tri count — worth having on while adding detail, to keep an eye on the cat's poly budget.
- **Auto Save** (`Edit → Preferences → Save & Load → Auto Save`) is on by default (2 min interval) and writes to a temp location — it's a crash safety net, not a substitute for deliberate `Ctrl+S` saves to the real file.
- **Numpad emulation** (`Edit → Preferences → Input → Emulate Numpad`) if working on a laptop without a physical numpad — remaps the top-row number keys to the ortho-view shortcuts in §2.
- A tutorial's on-screen hotkeys not matching what's listed here almost always means a non-default keymap or a changed preference (Spacebar-as-Search and Tab-for-Pie-Menu are common ones creators set) — check `Edit → Preferences → Keymap` rather than assuming the shortcut itself is wrong.

---

## 5. Gotchas / off-limits (fill in as they're hit)

Empty for now beyond the texture-packing note above — same role as `AssetPipeline.md` §7, but for Blender technique/workflow rather than the UE5 export contract. Add real lessons here once actual modeling sessions happen, not just tutorial-watching notes.

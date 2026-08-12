# Dungeon Cat: Game Dev Plan (design plan of record)

Working title: *Dungeon Cat* (placeholder)
Engine: Unreal Engine 5.8 | Style: cartoon 3D, third person | Modeling: Blender (self-authored)
Reference feel: extraction tension of Dark and Darker + class/ability depth of an ARPG like Diablo 4

Source: authored 2026-08-12 as `dungeon-cat-planning.md`, promoted here as the plan of record. Update in place as decisions change — this is the document engineering/design work should stay consistent with, not the stock template code currently checked in (see `CLAUDE.md`'s Project overview for current repo state vs. this plan).

---

## 1. The hook

A charming, upright cartoon cat crawling procedural dungeons and hunting the things cats love to hunt (spiders, snakes, mice, lizards) is a strong, marketable hook. It reads instantly in a screenshot or a 6-second clip, and the "cat vs vermin" framing gives you a coherent bestiary theme most dungeon crawlers lack. Protect that hook: every art and design decision should make the "cat power fantasy" clearer, not muddier.

---

## 2. Pivotal decisions (locked)

These four decisions drive everything downstream. All four are now decided.

### 2.1 Multiplayer: PvP extraction, co-op, or solo?
This is the single biggest cost driver in the whole project.

- **PvP extraction (true Dark and Darker):** dedicated servers, full replication of movement/combat/loot, anti-cheat, matchmaking, and a live population to extract *from*. For a solo dev this is the classic reason extraction games take 3+ years or die on the vine. High risk, high infra cost, and the fun collapses if nobody's online.
- **Co-op PvE (2 to 4 players vs the dungeon):** still networked, but listen-server/relatively forgiving. Adds real work but no "empty lobby" failure mode.
- **Solo PvE:** dramatically cheaper. No replication tax, faster iteration, ship years sooner.

**DECISION: Co-op PvE (2 to 4 players vs the dungeon). No PvP.**

This changes the architecture. Because co-op is a v1 requirement, not a later add-on, build replication in from the first line of combat and state code. Every ability, health change, enemy, and loot pickup is authored server-authoritative from the start. Retrofitting networking later is the expensive path; building it in now is the cheap one. Target 2-player co-op (listen-server, one player hosts) for the beta, scaling to 4 later. You keep the extraction *tension* (grab the loot, reach the exit before you're overwhelmed) as the co-op loop, which is your "get item and escape" mode.

### 2.2 Rig topology: bipedal or quadruped cat?
This quietly decides hundreds of hours of animation work.

- **Bipedal cartoon cat** (stands and fights like a knight/wizard, Khajiit-style): you can build on a UE5 Mannequin-compatible skeleton and retarget the enormous library of existing humanoid animations (Lyra, marketplace ARPG packs, Mixamo). You only hand-author the cat-specific flourishes. Gear, weapons, and spellcast poses all "just work."
- **Quadruped realistic cat:** every locomotion cycle and ability is bespoke, quadruped gaits are hard to make look good, and armor/weapon fitting gets weird.

**DECISION: Bipedal, on a humanoid-compatible skeleton.** This is the biggest single animation time-saver available: you retarget existing humanoid libraries and hand-author only the cat-specific flourishes, and gear/weapons fit cleanly.

### 2.3 Beta scope: 2-month grey-box slice
Your 8 kits, 4 modes, and full bestiary are the *full-release* target, not the beta.

**DECISION: 2-month beta, ruthlessly cut.** A 2-month co-op beta is reachable with heavy Claude Code assistance and your experience, but the compression comes from cutting art fidelity and content breadth, not from cramming the same scope faster. Claude Code accelerates code; it does not model, rig, or animate your cat. So the beta is a grey-box (or marketplace-placeholder) vertical slice with **one class, one mode, two enemies, a boss stub, and 2-player co-op.** Your self-authored art and the second class move to just after the beta. Full detail in section 8.

### 2.4 Engine: Unity or UE5?

**DECISION (recommended): UE5.**

The two hardest, highest-value systems this game needs are a real ability system and mature co-op networking, and UE5 gives you both nearly for free:

- **GAS (Gameplay Ability System)** is purpose-built for class-based, ability-heavy, buff/DoT combat with replication baked in. It is exactly your 4-classes / 8-specs problem. Unity has no equivalent out of the box; you would build or buy one and maintain it.
- **Replication** in UE5 is battle-tested for co-op action games of precisely this shape. Unity's options (Netcode for GameObjects, Mirror, Photon Fusion) are workable but less turnkey for this genre, and you own more of the plumbing.
- **You already have UE5 C++ dungeon-crawler code** in zombieshooter (character base, AI, loot patterns) to mine, plus the UE5 experience to lean on.
- Blender import, animation retargeting (IK Rig), and Fab marketplace content all support the plan.

The cost: GAS and UE5 replication have a real onboarding ramp, and that ramp is the main threat to a 2-month clock. Mitigate by pointing Claude Code hardest at the GAS and replication boilerplate, and by grey-boxing everything.

**When I'd flip to Unity instead:** if the 2-month slice is the true goal and full scope is secondary. Your Unity MCP + Claude Code pipeline is already productive, C# is faster to write and agentic tools handle it more cleanly than UE5 C++ (no UPROPERTY/UCLASS reflection quirks to trip up codegen), and for a single beta class you can hand-roll simple abilities without needing anything like GAS. The trade is that you rebuild in Unity what UE gives free, and that debt compounds as you approach the full 8-spec co-op vision.

Net: for the game as written (full-scope, ability-heavy, co-op ARPG), UE5. Given your concept ambition and existing UE5 assets, eat the GAS ramp with heavy Claude Code assistance.

---

## 3. Design pillars

Use these to arbitrate feature disputes. If a feature doesn't serve a pillar, it waits.

1. **Readable cat power fantasy.** Combat and animation sell "capable little predator." Snappy, weighty hits over sim-realism.
2. **A run you can feel in 8 to 12 minutes.** Procedural dungeons should produce short, replayable runs with rising tension toward the exit.
3. **Class expression, not class bloat.** Each class *feels* distinct within its first three abilities. Depth comes from a small set of well-tuned kits, not a huge count.
4. **Reusable content.** Every art and system decision favors reuse (shared skeleton, modular kit, enemy archetypes reskinned) so one person can fill a dungeon.

---

## 4. Technical architecture (UE5)

### 4.1 Ability & combat backbone: Gameplay Ability System (GAS)
Commit to GAS early. It is the correct foundation for 4 classes and 8 eventual specializations, and retrofitting it later is painful.

- **Attributes:** health, primary resource (mana/energy), armor, move speed, crit, etc., via an AttributeSet.
- **Abilities:** each skill is a GameplayAbility (dash, fireball, shield bash, heal-over-time).
- **Effects:** buffs, debuffs, DoTs, stuns as GameplayEffects with tags.
- **Cues:** VFX/SFX driven by GameplayCues so presentation is decoupled from logic.
- GAS is also replication-friendly, which keeps the co-op door open per decision 2.1.

There's a learning curve. Budget time in preproduction to get comfortable; it pays back across every class.

### 4.2 Procedural dungeon generation
Go **prefab-room graph stitching**, not full geometry synthesis (skip wave-function-collapse for v1).

- Build a library of hand-authored room modules from a shared modular kit.
- A generator picks rooms by seed, connects them via door sockets, and tags rooms as combat / loot / boss / exit.
- Populate encounters and loot by seed so runs are reproducible for debugging and shareable later.
- Rebuild navigation at runtime (Dynamic Nav Mesh / Nav Mesh Invokers) so enemy AI works in generated layouts.
- Seed everything: same seed = same dungeon. Invaluable for testing.

### 4.3 Modular character & customization
- **One master cat skeleton.** All customization, gear, and animation retarget to it. This is non-negotiable for keeping animation sane.
- **Appearance:** material/texture swaps (fur color, patterns) and a small set of modular head/accessory meshes.
- **Size:** blend shapes or controlled bone-scaling on the base mesh. Keep the collision capsule tolerant of a size range.
- **Gear:** skeletal mesh merge or Leader Pose Component so armor/weapon pieces share the base skeleton.

### 4.4 Data-driven everything
Drive classes, abilities, enemies, and loot from DataAssets / DataTables so you can tune without recompiling. Given your DevOps instincts, lean into this hard; it makes balancing a data problem, not a code problem.

### 4.5 Enemy AI
Behavior Trees + EQS, built as a small set of **archetypes** you reskin:

- Melee chaser (mouse, lizard)
- Ranged spitter (snake, spitting spider)
- Swarm/leaper (small spiders)
- Boss (multi-phase, telegraphed attacks)

Four archetypes cover most of the bestiary; new enemies become mostly art + stat tweaks.

Note: this repo's stock template variants already use **StateTree / GameplayStateTree** for their AI (see `CLAUDE.md`), not classic Behavior Trees. Reconcile this before AI work starts on the real plan — either standardize on StateTree (matches the existing project convention) or bring in classic BT/EQS as this doc originally specified. Not yet decided; flag as an open question in `SessionHandoff.md` when AI work begins.

### 4.6 Input & other systems
- Enhanced Input for bindings.
- Inventory/loot: start with fixed item DataAssets and rarity tiers; add Diablo-style procedural affixes later.
- Save system: profile (cat customization, unlocks) separate from run state.

### 4.7 Reuse what you already have
Your zombieshooter (UE5 C++) project likely already contains a character base, input setup, loot/inventory scaffolding, and AI patterns worth mining or at least referencing. Don't rebuild what you've already solved once.

---

## 5. Art & Blender pipeline (self-authored)

Since you're doing most modeling yourself, the pipeline *is* your timeline. Optimize it aggressively.

- **Master cat mesh:** one clean, UE5-mannequin-compatible topology, bipedal. Everything else fits this.
- **Modular gear:** model armor/weapons to the base proportions; rig to the shared skeleton.
- **Dungeon kit with trim sheets:** a modular wall/floor/prop kit sharing a few texture atlases. This keeps both modeling load and draw calls down, and makes procedural rooms cheap to author.
- **Enemy variants:** model one strong base per archetype, then recolor/scale/accessorize for variety before making all-new meshes.
- **Blender to UE5 hygiene:** lock a consistent scale/units convention early, standard export settings, and a naming convention. Fix the pipeline once so re-exports are painless.
- **Animation strategy:** shared cat locomotion + retargeted humanoid libraries for the common cases; hand-author only signature ability montages.

---

## 6. Content scope reference (full release vs beta)

| Content | Full-release vision | 2-month beta |
|---|---|---|
| Classes | 4 (Knight, Rogue, Wizard, Healer) | 1 (Knight) |
| Specializations | 8 total | 1 |
| Game modes | Get-item-escape, Kill-boss, Rescue, Combo | 1 (Get-item-escape) |
| Enemy archetypes | 4 reskinned across ~8 to 12 enemies | 2 archetypes |
| Bosses | Several, multi-phase | 1 stub |
| Customization | Size + appearance + gear cosmetics | Placeholder only |
| Art | Self-authored, finished | Grey-box / marketplace placeholder |
| Multiplayer | 4-player co-op | 2-player co-op |

---

## 7. MVP / vertical slice (the thing to build first)

One playable, fun run that proves the concept:

- One class (suggest **Knight**, simplest combat to feel good), one spec.
- Procedural dungeon: 6 to 10 stitched rooms, one exit, seed-driven.
- Two enemy archetypes (melee chaser + one ranged), plus the boss stub.
- Core loop: enter -> fight through -> grab the objective item -> reach the exit before the dungeon overwhelms you.
- Basic loot pickup, health/resource, death and run-end screens.
- Grey-box art is fine here. You are testing *fun*, not polish.

If this slice is fun with programmer art, the game is real. If it isn't, fix the loop before making a single finished asset.

---

## 8. Roadmap & timeline (2-month beta)

Assumptions: solo, heavy Claude Code assistance on systems code, your UE5 experience, and grey-box / marketplace-placeholder art for the beta milestone. The compression comes from cutting art fidelity and content breadth, not from cramming the full scope faster. Claude Code accelerates code; it does not model, rig, or animate your cat, so self-authored art is deliberately deferred past the beta line.

**What the 2-month beta is:** one playable, co-op vertical slice that proves the loop is fun.

- 1 class (Knight), 1 spec, ~4 core abilities via GAS
- 2-player co-op (listen-server), server-authoritative from the start
- Procedural dungeon: 6 to 10 stitched grey-box rooms, seed-driven, runtime nav rebuild
- 2 enemy archetypes (melee chaser + ranged) plus a boss stub
- Get-item-escape mode end to end: enter, fight, grab objective, reach the exit alive
- Grey-box or marketplace art, basic loot, health/resource, run-end screens

**What the beta is not:** your finished cat model, a second class, four modes, or polished 4-player. Those are post-beta.

**8-week shape (heavily assisted, aggressive):**

- Weeks 1 to 2: UE5 project + GAS sandbox; Knight's core abilities working; Enhanced Input; grey-box arena. Get one ability replicating across two clients now so networking pain shows up in week 1, not week 6.
- Weeks 3 to 4: procedural room-graph generation v1 with runtime nav; two enemy archetypes on Behavior Trees; loot pickup.
- Weeks 5 to 6: co-op replication pass. Make abilities, enemies, and loot server-authoritative and correct for two players. Budget the most debugging time here; networking is where aggressive timelines slip.
- Weeks 7 to 8: Get-item-escape objective + exit, boss stub, run-end flow, balance and bug pass. Playable co-op slice.

This is reachable but tight, and weeks 5 to 6 (co-op) are the real risk window. If it slips, the fallback is to ship the beta solo-playable with co-op wired but rough, and harden co-op in the following weeks. Protect the "is the loop fun" gate above all.

**After the beta (art + expansion):**

- Self-authored art pass: master cat mesh + skeleton, first dungeon kit with trim sheets, real enemy models. (~1 to 2 months)
- Second class + spec, more enemies, first real boss, customization v1, 4-player co-op hardening. (~2 to 4 months)
- Remaining classes/specs, remaining modes (Kill-boss, Rescue, Combo), deeper loot, polish, audio.

**Full release: roughly 12 to 24 months** from start, depending on final scope. The 2-month milestone is a fun-validation and co-op-proof gate, not a shippable product.

A realistic note: you have two other games in flight and an active job hunt. The 8-week shape assumes Dungeon Cat gets serious, near-daily hours during those weeks. If it's a third parallel project splitting your time, the calendar stretches accordingly.

---

## 9. Risk register

| Risk | Impact | Mitigation |
|---|---|---|
| GAS + replication ramp threatens the 2-month clock | High | Point Claude Code at the boilerplate; prove replication in week 1 |
| Co-op debugging overruns weeks 5 to 6 | High | Build server-authoritative from line 1; fallback to solo-playable beta |
| Self-authored art becomes the bottleneck | High | Defer it past beta; grey-box / marketplace art for the slice |
| Scope creep back to "all 8 specs + 4 modes" | High | Hold the 1-class beta line; expansion is post-beta only |
| Procedural gen produces boring layouts | Medium | Room-graph rules + tuning, hand-authored room quality |
| Competing projects starve this one | Medium | Decide its priority tier explicitly |

---

## 10. Immediate next steps

1. Decisions locked: co-op PvE, bipedal, UE5, 2-month grey-box beta.
2. Stand up the UE5 project and a GAS sandbox this week; get the Knight's first ability replicating across two clients early, so networking pain surfaces in week 1, not week 6.
3. Build server-authoritative from the first line. Do not defer replication.
4. Grey-box the whole vertical slice loop before making any finished art.
5. Only after the beta proves the loop fun: model the master cat mesh + skeleton, then the first dungeon kit with trim sheets.

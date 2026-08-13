# Session Handoff

> Rewritten every session — current status only, not appended to. Full history lives in the git commit log.
>
> **Away session?** Read `Docs/AsyncSessionProtocol.md` once now and follow it for the rest of the session without re-reading it.

## Status (2026-08-13)

**Deep design pass complete** — a documentation-only session (dev away, 3D modeling) resolved a large chunk of the open-question backlog and restructured how the docs are organized. No code changed.

**New doc structure**: `Docs/Classes.md`, `Docs/GameplayLoops.md`, `Docs/Items.md` are new, ongoing "living" topic docs (creative/content "what") — `SystemsDesign.md` keeps only the technical/architecture "how" and cross-references them. Production phases from P2 on now have dedicated detail docs (`Docs/P2_DungeonAI.md`, `Docs/P3_CoopHardening.md`, `Docs/P4_LoopAndBoss.md`), each with a 3-stage structure (plan finalization → do-yourself → testing/manual). `ProductionPlan.md` is now a lightweight index pointing at those.

**Resolved this pass**: a Hub World ("The Den" — persistent shared space with a loadout station, vendor, training dummies, and a run-start portal, replacing a menu-only Lobby); the boss-gates-objective structural rule (mandatory, WoW-raid-style, for every future game mode, not just this one); the beta boss's identity (Swarm-mother) and full mechanical skeleton (2 phases, telegraphed attacks, pure StateTree); a skill/leveling progression framework (hybrid learn-by-doing + milestone unlocks, deliberately not gating core content) with a starting skill list; controller support planned from the start (not cut for beta); a fog-of-war map system; concrete AI archetype StateTree/EQS specs and tuning numbers for all 3 enemy archetypes; the itemization/loot beta content list; the session model (confirmed no Steam integration exists anywhere — direct-IP for the beta, storefront choice still open); saves, audio policy, and the specialization-system framework.

**Reopened, not resolved**: dungeon generation technique. The dev is reconsidering the previously-planned "prefab-room graph stitching" approach — flagged clearly in `SystemsDesign.md` §4, `GameDevPlan.md` §4.2, and `AssetPipeline.md` §5 rather than left silently "decided." This blocks P2's dungeon-generation-specific work (room catalog, generation algorithm) but not P2's AI/targeting-fix/loot-pickup work, which can proceed now.

**Still explicitly the dev's own call**: final dungeon-generation technique; the full skill list (dev wants to co-design this further, `Docs/Classes.md` has a starting draft); Steam vs. Epic as the eventual storefront.

**Unrelated to this pass, still true**: all 4 Knight abilities are compiled but not yet PIE-tested; Enhanced Input still isn't wired (now scoped to include gamepad, not just KBM).

## Next step

Dev's choice between two independent tracks:

1. **Resume P1 verification** — PIE-test the 4 Knight abilities with 2 clients, wire Enhanced Input (KBM + gamepad).
2. **Start P2's unblocked work** — the multiplayer-targeting fix and the two enemy archetypes don't depend on the dungeon-generation decision; see `Docs/P2_DungeonAI.md` Stage 2.

Either way, the dungeon-generation technique decision is the one thing actively needed from the dev to unblock the rest of P2.

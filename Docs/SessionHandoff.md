# Session Handoff

Rewritten every session — current status only, not appended to. Full history lives in the git commit log.

## Status (2026-08-12)

Dev environment + full design planning complete, no game code written yet:

- Git/Gitea/GitHub/LFS/backup infra fully wired (see `CLAUDE.md`'s Local Git Infrastructure section). The Gitea repo was generated from the dev's "Game Dev Template" repo (unrelated history) and merged into this project's real history — resolved in favor of the real content, template placeholder files (`README.md`/`SETUP.md`/`Docs/CommandReference.md`) removed. Push mirror to GitHub is configured and confirmed working.
- Design planning complete: `Docs/GameDevPlan.md` (pillars/decisions/roadmap), `Docs/SystemsDesign.md` (technical spec for every system), `Docs/AssetPipeline.md` (Blender-facing specs — skeleton, sockets, animation list, dungeon trim-kit grid, naming), `Docs/ProductionPlan.md` (P0-P4 phase breakdown of the 8-week beta roadmap).
- AI architecture question resolved: StateTree owns the top level for every enemy archetype, Behavior Tree only as an optional embedded sub-behavior for the boss if StateTree alone proves unwieldy there (`GameDevPlan.md` §4.5).

**The repo's actual code/content is still just Epic's stock Third Person template + the three starter variants (Combat/Platforming/SideScrolling)** — none of the planning above has been built yet. Don't assume any of it exists in code just because the docs do.

## Next step

Start `Docs/ProductionPlan.md` **P0 (reuse audit)** — read through `zombieshooter`'s `UZSHealthComponent`/`Survival/`/`Interaction/UZSInteractableComponent`/`Weapons/` and confirm what's actually worth porting, and verify the existing `Variant_Combat` StateTree/AnimNotify patterns still look reusable now that `SystemsDesign.md` has committed to building on them. Then move into **P1**: stand up the GAS foundation and prove one ability replicates across 2 PIE clients before building anything else on top — this is the single highest-risk item on the whole 2-month clock, per `GameDevPlan.md` §9.

Meanwhile, art (per `Docs/AssetPipeline.md`) can proceed in parallel — the master cat skeleton and the dungeon trim-kit are the two highest-value things to start modeling first, since P1/P2 will eventually need them and they don't block on any code existing yet.

# Session Handoff

Rewritten every session — current status only, not appended to. Full history lives in the git commit log.

## Status (2026-08-12)

Dev environment stood up: git initialized, LFS configured, remotes wired (`origin` = Gitea `http://localhost:3000/aaronrod/dungeoncat.git`, `github` = https://github.com/aaronrod3/dungeoncat mirror), nightly backup task registered ("DungeonCat Nightly Backup", 3:10 AM), `.claude`/`.mcp.json` set up for build-command permissions and the `unreal-mcp` (Epic ModelContextProtocol) connection. `Docs/GameDevPlan.md` now holds the real design plan of record.

**The repo's actual code/content is still just Epic's stock Third Person template + the three starter variants (Combat/Platforming/SideScrolling)** — nothing from `GameDevPlan.md` (GAS, procedural dungeons, co-op replication, the cat character) has been built yet. Don't assume any plan content exists in code just because the plan doc does.

**Gitea repo may still need to be created** — the local `git push -u origin main` failed non-interactively (Git Credential Manager couldn't authenticate without a UI prompt). Confirm `http://localhost:3000/aaronrod/dungeoncat` exists and the push has succeeded before relying on Gitea as a real remote.

## Next step

Per `Docs/GameDevPlan.md` §8 Weeks 1-2: stand up a GAS sandbox and get the Knight's first ability (any ability) replicating across two PIE clients as early as possible — this is the highest-risk unknown on the 2-month clock, so surface the pain in week 1, not week 6.

Also unresolved: §4.5 flags that this repo's existing variants use StateTree for AI, while the plan assumes classic Behavior Trees + EQS — decide which convention the real enemy AI uses before building the first enemy archetype.

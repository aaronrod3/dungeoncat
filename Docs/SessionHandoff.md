# Session Handoff

Rewritten every session — current status only, not appended to. Full history lives in the git commit log.

## Status (2026-08-12)

Dev environment + full design planning + P0 reuse audit complete. No real game code written yet.

- Git/Gitea/GitHub/LFS/backup infra fully wired (see `CLAUDE.md`'s Local Git Infrastructure section), push mirror confirmed working.
- Design planning complete: `Docs/GameDevPlan.md`, `Docs/SystemsDesign.md`, `Docs/AssetPipeline.md`, `Docs/ProductionPlan.md` (P0-P4). AI architecture resolved (StateTree default, BT only as an embedded escape hatch for the boss — `GameDevPlan.md` §4.5).
- **P0 reuse audit done** (`ProductionPlan.md` P0, findings in `SystemsDesign.md` §10). Reused-as-pattern: the `AnimNotify_DoAttackTrace` sweep, the combo montage-jump mechanism, `ACombatAIController`'s StateTree wiring, `CombatStateTreeUtility`'s condition/task library, `CombatEnemySpawner`'s activate/track-until-depleted shape, `zombieshooter`'s `UZSInteractableComponent` (ports directly) and `UZSHealthComponent`'s downed/revive state machine (port the logic, not the component — GAS owns health now). **Two things must be rebuilt, not reused**: `ACombatEnemy`'s damage/health is entirely non-replicated (no `HasAuthority()` anywhere in it) so none of that code carries over to the GAS damage pipeline; and enemy player-targeting (`EnvQueryContext_Player`, `FStateTreeGetPlayerInfoTask`) is hardcoded to player index 0 — every enemy currently would only ever see one specific co-op player and ignore the other(s). That targeting fix is now called out explicitly as a P2 blocker.

**The repo's actual code/content is still just Epic's stock Third Person template + the three starter variants (Combat/Platforming/SideScrolling)** — none of the planning above has been built yet. Don't assume any of it exists in code just because the docs do.

## Next step

Start `Docs/ProductionPlan.md` **P1**: stand up the GAS foundation (`UDCAttributeSet`, `UDCAbilitySystemComponent` on `ADCPlayerState`, the 4 Knight abilities, the single-entry-point damage execution) and prove one ability replicates correctly across 2 PIE clients before building anything else on top — this is the single highest-risk item on the whole 2-month clock, per `GameDevPlan.md` §9.

Meanwhile, art (per `Docs/AssetPipeline.md`) can proceed in parallel — the master cat skeleton and the dungeon trim-kit are the two highest-value things to start modeling first, since P1/P2 will eventually need them and they don't block on any code existing yet.

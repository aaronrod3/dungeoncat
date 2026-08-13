# P4 — Get-item-escape loop + boss (Weeks 7-8)

Detail doc for `ProductionPlan.md`'s P4 entry — the beta-complete gate. Structure: Stage 1 (plan finalization/open questions) → Stage 2 (do-yourself/autonomous) → Stage 3 (testing/manual steps).

---

## Stage 1 — Plan finalization / open questions

- [x] Boss identity — resolved 2026-08-13, **Swarm-mother**. Full mechanical skeleton (phases, telegraphs, arena): `SystemsDesign.md`'s Boss design section.
- [x] Boss-gates-objective structure — resolved. The boss is a mandatory gate immediately before the final objective, never avoidable — corrects the old "survive the boss or avoid it" exit-criteria language below. Structural rule for all future modes too: `GameDevPlan.md` §6.1.
- [x] StateTree-vs-BT for the boss — resolved, pure StateTree (`GameDevPlan.md` §4.5).
- [x] Objective item mechanics — resolved: stays on a downed carrier's body until revived (reinforces the existing revive mechanic's stakes), lost entirely on run failure (`SystemsDesign.md` §7's "run's loot is at stake" framing). Flavor/name is a placeholder-fine beta detail, not decided here.
- [x] Run-end screen content — resolved, `SystemsDesign.md` §8: success keeps all run-carried loot + extraction confirmation; failure shows what's lost; profile-scope unlocks/skill-levels persist regardless of outcome.
- [ ] Balance-pass acceptance criteria — drafted below, confirm before Stage 3 starts.

## Stage 2 — Do-yourself (autonomous)

- [ ] Objective item spawn + pickup, exit volume + extraction logic (reach exit while carrying the objective = success).
- [ ] Boss StateTree implementation — Swarm-mother, 2 phases at 50% HP threshold, ~0.5-1s telegraphs per attack, phase-2 spawns 2-3 swarm adds reusing swarm-archetype content (`SystemsDesign.md`'s Boss design section for the full spec).
- [ ] Run-end flow (success/fail screen, loot summary) per `SystemsDesign.md` §8's content spec.

## Stage 3 — Testing & manual steps (needs the dev's hands)

- [ ] Full MVP loop end-to-end, in co-op, PIE-verified: Hub → enter dungeon → fight through both enemy archetypes → grab the objective → defeat the boss (mandatory, not avoidable) → reach the exit → run-end screen shows correct loot.
- [ ] Balance/bug pass — acceptance criteria: median run length across ~5 test runs lands in the 8-12 min pillar window; no ability is a clear must-pick/never-pick outlier; zero crash-tier/run-blocking bugs; the loop completes cleanly both solo and 2-player.

**Exit criteria**: the full loop works end-to-end in co-op as described above. This is the beta-complete gate.

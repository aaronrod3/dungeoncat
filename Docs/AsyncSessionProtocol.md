# Async Session Protocol

> Read this once, at the start of a session the user has flagged as an "away" session (e.g. "this is an away session," "go ahead and work on X while I'm out"). Follow it for the rest of that session without re-reading it. This doc is not auto-loaded — `SessionHandoff.md` points here when it applies.

## Is this an away session?

Trigger: the user opens with something like "this is an away session," or otherwise signals they're stepping away rather than working alongside you in real time. If it's genuinely ambiguous, ask once at the very start which mode this is — don't ask again after that.

## Scope: one cluster, not one whole phase

"Cluster" here means one coherent chunk of work sized to finish (implement + verify, or implement + fully document) inside a single session — roughly one checklist item (or a tightly related handful) from the active phase in `Docs/ProductionPlan.md` (P0-P4), not the whole phase. A full phase covers 1-2 weeks of work, so an away session means one cluster from wherever `SessionHandoff.md`'s "Next step" points, not the whole phase.

**Stop after that one cluster.** Do not cascade into the next cluster without the user explicitly saying to proceed, even later in the same sitting.

## Mode A — compile-gated (default)

1. Confirm the editor is closed: `Get-Process | Where-Object ProcessName -match "Unreal|LiveCoding"`. If it's running, don't build — treat this like a normal working-alongside session instead (or wait).
2. Implement the cluster.
3. **Compile gate** — full `Build.bat` rebuild (not Live Coding). Fix and re-run on failure, capped at ~4 attempts against the same error. If still failing, drop to Mode B for the rest of the session rather than keep burning turns on it.
4. **Tier-1 smoke test only** — launch headless, scan the log for crashes/asserts, and for `is not a child class of`/`invalid target type` (a known Unreal Live Coding module-corruption signature on other projects this dev runs — not yet confirmed hit here, but worth watching for). Never attempt simulated interactive PIE input (mouse/keyboard into the viewport) — confirmed unreliable on `zombieshooter`'s identical MCP tooling (2026-07-20), don't retry it here either.
5. Commit. Tag the message `[compiled]` once it's passed steps 3-4 clean.
6. Push to `origin main` (never force-push). No CI is configured on this repo (no `.github/workflows/`), so frequent pushes carry no Actions-spend risk.
7. Append results/gaps to the relevant phase section of `Docs/ProductionPlan.md` — not to `SessionHandoff.md`.

## Mode B — fallback, no compile gate

Drop into this only when Mode A's compile loop stalls (step 3) or is clearly costing more time/context than the work it's gating.

- Keep implementing carefully against established codebase patterns (`SystemsDesign.md`, `AssetPipeline.md`, the P0 audit findings) — don't improvise a new convention mid-cluster.
- Commit at logical task boundaries, message tagged `[uncompiled]` so it's unambiguous later.
- Still push periodically so progress is checkable remotely — just honestly labeled, since there's no CI badge to signal state either way.
- No smoke test (nothing compiled to launch).

## Always, either mode

- Never touch `main` destructively, never force-push, never skip hooks.
- Content/Blueprint/Data-Asset authoring is out of reach — needs a live editor + `unreal-mcp`, unavailable while you're away. Log it as a content gap (e.g. the still-open Input Action/IMC asset gap from P1); don't attempt a workaround.
- A genuine design-shaping fork gets asked in-chat, terse (one question, short options), no push notification — keep working other unblocked items while waiting rather than stalling.
- Implementation-detail judgment calls: pick a sensible default, log it in `ProductionPlan.md`, keep going.
- **`Source/DungeonCat/Tests/` is explicitly out of scope for an away session, once it exists** — never build, run, or extend an automation test suite unsupervised, in either mode. No test suite exists yet as of P1; this rule is forward-looking, apply it the moment one gets created. Same reasoning as `zombieshooter`'s identical rule: it requires closing the dev's editor, which the dev wants to stay in control of the timing of — see `Docs/CommandReference.md`'s "Editor close/rebuild for test runs" section.

## Not yet set up for this project

`zombieshooter`'s away-session protocol has a third "Queue mode" for scheduled/auto-selected clusters pulled from a GitHub Issues backlog (labels, parallel git worktrees, PR-based merging). None of that infrastructure exists for DungeonCat — no Issues labels, no scheduled automation, no worktree convention. Don't assume it applies here; if the dev wants it, that's a real infrastructure-setup task first, not something to improvise mid-session.

## End of session

1. One brief `SessionHandoff.md` rewrite (~15-25 lines — phase, last completed, next step, verification status. Not a repeat of this doc.)
2. Final push.
3. Stop. Wait for "proceed" before starting another cluster.

## Known limits (don't relitigate these mid-session)

- PIE gameplay/logic verification and Blueprint/content work require a human at the editor. The ceiling for an away session is "compiles clean" (Mode A) or "implemented, awaiting verification" (Mode B) — never "verified working."
- There's no tool available to self-trigger context compaction at a precise threshold. Practical substitute: treat a long/heavy session as a signal to close out the current cluster cleanly (handoff + doc updates) rather than mid-task, so if the harness's automatic compaction fires, it lands at a clean boundary rather than mid-edit.

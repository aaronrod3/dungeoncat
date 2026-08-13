# P3 — Co-op replication hardening (Weeks 5-6)

Detail doc for `ProductionPlan.md`'s P3 entry. Budget the most debugging time here — per `GameDevPlan.md` §9 this is the real risk window on the whole timeline. Structure: Stage 1 (plan finalization/open questions) → Stage 2 (do-yourself/autonomous) → Stage 3 (testing/manual steps).

---

## Stage 1 — Plan finalization / open questions

- [x] Session/join flow — resolved 2026-08-13, direct-IP (`SystemsDesign.md` §3.1). No longer open.
- [x] Lobby/session UI flow — resolved, folded into the Hub World design (`Docs/GameplayLoops.md`) — the Hub *is* the shared pre-run space, no separate Lobby screen.
- [x] Downed/revive spec (duration, radius, capabilities, UI) — resolved, `SystemsDesign.md` §8.
- [x] Server-authority test scenarios — enumerated below.
- [ ] Confirm at design time (not yet checked): is the multiplayer-targeting fix from P2 genuinely `PlayerArray`-based, or does it just hardcode indices 0 and 1? Needs a real audit, not an assumption, before this phase's own server-authority pass is trusted.

## Stage 2 — Do-yourself (autonomous)

- [ ] Audit every ability, enemy action, and loot pickup from P1/P2 for server-authority correctness with 2 players, not just 1.
- [ ] Swarm/leaper archetype (third enemy), per `SystemsDesign.md` §5.1's spec and tuning numbers.
- [ ] Co-op downed/revive implementation, per `SystemsDesign.md` §8's spec.
- [ ] 4-player forward-compat pass: confirm the Hub's player-list/party concept is dynamic (not hardcoded to 2), swarm EQS spacing is scored against all enemies present (not a fixed player count), and downed/revive uses nearest-teammate lookup over `PlayerArray` rather than "the other player."

## Stage 3 — Testing & manual steps (needs the dev's hands)

Server-authority test checklist — 2 real PIE/networked clients, not 1:

- [ ] Static `HasAuthority()` audit of every `Server_`-prefixed function (code review, not just testing).
- [ ] Each of the 4 Knight abilities activated by the **non-host** client replicates correctly to the host's view.
- [ ] Enemy damage from either client stays in sync (no desync between what each client sees).
- [ ] Loot pickup doesn't duplicate or desync between clients.
- [ ] Revive works in both directions (either player can revive the other).
- [ ] Ability activation stays correct under ~100-200ms simulated latency.
- [ ] Host-disconnect behavior is clean — no hang/crash, remaining client(s) return to Main Menu per `SystemsDesign.md` §3.1.
- [ ] Full loop (enter, fight, loot, revive) works with all 3 enemy archetypes present simultaneously.

**Exit criteria**: a full solo-playable loop also works correctly with 2 real networked clients, including a revive and a simultaneous 3-archetype encounter. Documented fallback if this phase slips (`GameDevPlan.md` §8): ship the beta solo-playable with co-op wired-but-rough, harden after — don't silently cut co-op scope without updating `GameDevPlan.md` §2.1's decision if this fallback gets used.

# Session Handoff

> Rewritten every session — current status only, not appended to. Full history lives in the git commit log.
>
> **Away session?** Read `Docs/AsyncSessionProtocol.md` once now and follow it for the rest of the session without re-reading it.

## Status (2026-08-13)

**All 4 Knight abilities are implemented and compiled** (away session, Mode A per `AsyncSessionProtocol.md`) — Claw Flurry/Pounce, Headbutt, Zoomies, Bunny Kick, replacing the throwaway test ability from the replication proof. Compiled clean on the 4th attempt (Mode A's cap) and smoke-tested clean (headless launch, no crash/assert). Full detail on what changed and why: `ProductionPlan.md` P1.

**One real architecture correction worth knowing before touching this code**: `UGameplayEffect::FindOrAddComponent<UTargetTagsGameplayEffectComponent>()` (granting a GameplayTag from a GameplayEffect, UE5.8's post-5.3 mechanism) crashes if called from the GE's own constructor — confirmed via a smoke-test crash, not a guess. This project's GameplayEffects don't grant tags that way anymore: state tags use `AddLooseGameplayTag`/`RemoveLooseGameplayTag` called from ability code at runtime, and cooldowns use manual timestamp tracking on `UDCGameplayAbility` instead of GAS's `CooldownGameplayEffectClass`. Don't reintroduce the component-in-constructor pattern.

**One deliberate, flagged scope cut**: Headbutt doesn't apply `State.Staggered` yet — nothing reads that tag (no enemy AI exists, P2 work), so it wasn't worth working around the crash blind for something unconsumed. Revisit with the loose-tag pattern once an enemy needs to react to it.

**Not yet PIE-tested.** The away session's ceiling is "compiles clean + smoke-tested" — real gameplay feel (combo timing, Pounce's reach, Zoomies canceling into Basic Attack) needs a human at 2 PIE clients, same as the original replication proof.

## Next step

1. **PIE-test the 4 abilities** with 2 clients (editor is open) — this is real verification work the away session couldn't do. Nothing's bound to input yet, so this needs either a quick temporary console-command hook (mirroring the deleted test ability's approach) or going straight to Enhanced Input.
2. **Enhanced Input** (`IMC_DC_Default`) — still needs hands-on editor work (Input Action assets can't be created via automation). Once it exists, wire it to `ADCPlayerCharacter::AbilityInputPressed`/`AbilityInputReleased` using `EDCAbilityInputID` — no ability code needs to change.
3. **Grey-box test arena** once input is wired, to actually feel the combat out in.

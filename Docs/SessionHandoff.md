# Session Handoff

> Rewritten every session — current status only, not appended to. Full history lives in the git commit log.
>
> **Away session?** Read `Docs/AsyncSessionProtocol.md` once now and follow it for the rest of the session without re-reading it.

## Status (2026-08-12)

**P1's core risk is retired.** GAS foundation code compiles clean and — dev-confirmed in a real 2-player listen-server PIE session — replicates correctly. `DCGameMode` is now the project's actual default GameMode (set via Project Settings during that test). This was the single highest-risk unknown on the whole 2-month clock (`GameDevPlan.md` §9); it's now proven, not just written.

What exists in code: `UDCAttributeSet`, `UDCAbilitySystemComponent` on `ADCPlayerState`, `UDCDamageExecCalculation` (single damage entry point), `UDCGameplayAbility` base class, `ADCPlayerCharacter`/`ADCGameMode`, a native `Config/Tags/DungeonCatGameplayTags.ini` tag list, and the throwaway `UDCGameplayAbility_TestDamage`/`UDCGameplayEffect_TestDamage` test pair (**delete both once the real 4 abilities exist** — they've served their purpose).

**The Knight's 4 beta abilities are named** (`SystemsDesign.md` §2.3) but **not yet implemented as real ability classes** — only the throwaway test ability exists in code:
- **Claw Flurry / Pounce** — tap for a 3-hit cleave combo (Swipe → Rake → Shred), hold to charge a Souls-style heavy (Pounce). One ability/hotkey, not two.
- **Headbutt** — gap-closer + stagger.
- **Zoomies** — mobility dash, i-frames, cancels Claw Flurry specifically.
- **Bunny Kick** — AoE crowd-control.

**The full post-beta 16-ability roster is also named** (`SystemsDesign.md` §2.6) — Rogue (Quickclaw/Slink/Ambush/Hiss), Wizard (Jinx/Evil Eye/Hairball/Nine Lives), Healer (Swat/Purr/Groom/Biscuits). Pure design/naming, not scheduled, no code — don't confuse this with anything being built.

Also in place this session: `Docs/CommandReference.md` and `Docs/AsyncSessionProtocol.md` (adapted from `zombieshooter`).

## Next step

Implement the Knight's 4 real abilities as `UDCGameplayAbility` subclasses, replacing the throwaway test ability (delete it once these exist):

1. **Claw Flurry / Pounce** — port the audited `AnimNotify_DoAttackTrace` sweep + combo montage-jump mechanism (P0 audit, `SystemsDesign.md` §10) into `GA_DC_Knight_BasicAttack`, input-buffered for the tap-combo (not `CombatEnemy`'s AI-random hit count), plus the hold-to-charge Pounce variant reusing `CombatEnemy`'s existing charge-loop pattern (`ChargeLoopSection`/`ChargeAttackSection`).
2. **Headbutt, Zoomies, Bunny Kick** — per their `SystemsDesign.md` §2.3 specs and the P1 design decisions (attacks slow-not-root via a `State.Attacking` MoveSpeed modifier; Zoomies cancels Claw Flurry specifically via `CancelAbilitiesWithTag`, Headbutt/Bunny Kick commit fully; crit plumbed at 0%).
3. Enhanced Input (`IMC_DC_Default`) — the editor is open, so the Input Action assets that were blocked all session can finally get created.
4. Grey-box test arena to actually feel the combat out in.

None of this needs Blender/art assets yet — grey-box first, per the beta scope.

The editor is currently open.

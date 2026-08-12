# {{PROJECT_NAME}}

{{One-paragraph pitch: engine version, genre, solo/team, C++/Blueprint split, any pivot history worth flagging up front.}}

**Read `Docs/SessionHandoff.md` first, every session — it is kept short on purpose and only covers the last completed task and the immediate next step. Full history lives in git commit log, not in this repo's docs.**

Docs: `Docs/GameDevPlan.md` = design plan of record (create this once you have pillars/scope/decisions worth recording — don't stub it out empty). `Docs/SessionHandoff.md` = current status only (rewritten every session, not appended to). `Docs/CommandReference.md` = ready-to-paste commands (build, git, editor close/rebuild).

## Naming (critical)
- `{{PREFIX}}` prefix for all project-specific classes/assets: `A{{PREFIX}}PlayerCharacter`, `A{{PREFIX}}GameMode`, `U{{PREFIX}}HealthComponent`, `BP_{{PREFIX}}_*`, `DA_{{PREFIX}}_*`, `WBP_{{PREFIX}}_*`. Pick a short (2-4 letter) prefix once, early, and don't change it — Blueprint asset renames are painful mid-project and can corrupt references (see the Live Coding lesson below for a related failure mode).
- Input Actions unprefixed (`IA_Fire`); Input Mapping Contexts prefixed (`IMC_{{PREFIX}}_Default`).
- If the project has an enemy/opposing-faction class family that needs to stay visually/verbally distinct from the player (e.g. this template's origin project kept "Zombie" and "ZS" strictly separate), write that rule here explicitly — it's cheap to state up front and expensive to untangle later.

## Commands
- Build (Editor): Rider/VS → `{{PROJECT_NAME}}Editor Win64 Development`, or `Build.bat {{PROJECT_NAME}}Editor Win64 Development -project=...uproject -waitmutex`
- Gen project files: `Build.bat -projectfiles -project=...uproject -game -engine`
- Test: PIE for gameplay feel/visuals/multiplayer — still needs a human, no way around it (no reliable PIE-input automation path exists via MCP tooling, see MCP/Editor Tooling below). For pure server-logic/state/math, put headless Unreal Automation Tests under `Source/{{PROJECT_NAME}}/Tests/`. **Always pass an explicit `-log=`** when running tests from the command line — the default log path can silently fail to run at all while the GUI editor is open.
- Header change → regen project files if needed. Live Coding (Ctrl+Alt+F11) OK for .cpp-only changes.
- Git + LFS, branch `main`. Commit after each sub-task. Never force-push main.

## Architecture
{{Fill in per-module bullets as the project grows: one bullet per top-level Source/ folder (Framework/, Player/, Combat/, Inventory/, UI/, etc.), updated as systems land — not written speculatively ahead of the code. Describe WHAT EXISTS, not internal mechanics; once a module's "how/why" gets long enough to need its own explanation, split it into a dedicated systems-reference doc rather than growing this section indefinitely.}}

## Replication convention (adjust to your project's actual pattern, then make it mandatory)
`UPROPERTY(ReplicatedUsing=OnRep_X)` + `Server_X()` gated by `HasAuthority()` + `GetLifetimeReplicatedProps()`/`DOREPLIFETIME()` + `OnRep_X()` broadcasts a delegate. Never poll replicated state directly.

## Conventions
- Epic naming standard (`AMyActor`, `UMyComponent`, `FMyStruct`, `EMyEnum`, `IMyInterface`) + this project's prefix rule above.
- Booleans: `b` prefix. Pointers: `TObjectPtr<T>` in UPROPERTY, raw `T*` locally.
- `GENERATED_BODY()` on all reflected classes.
- Server mutators prefixed `Server_`. No magic numbers — tunables in config/data assets, not hardcoded. Route state changes (damage, etc.) through a single funnel function, never scattered direct mutation.
- No commented-out code — use branches. Check `Build.cs` before adding heavy modules.
- **Tech split**: C++ = base classes/data contracts/perf-sensitive/shared machinery. Blueprint = gameplay config/tuning. Consider making player/weapon action functions `BlueprintNativeEvent` so BP children can override without recompiling.

## Workflow Efficiency (lessons — keep this section short, prune what stops being true)
- **Don't auto-attempt `Build.bat` while the editor's likely open.** It fails on the Live Coding lock nearly every time — a wasted round-trip for zero information. State that a rebuild is needed and let the dev trigger it, rather than spending a call finding out again.
- **Live Coding (Ctrl+Alt+F11) can silently corrupt Blueprint references to native classes, project-wide, on any patch — broader than just "the class that changed."** Live Coding reinstances every class in the module on every patch, so any Blueprint anywhere with a cast/reference to a native class in that module is at risk, whether or not that specific class was touched. No crash, no visible editor error — just silently wrong behavior. When a dev reports something that "should just work" behaving wrong after a Live Coding recompile, check the Output Log for `is not a child class of` or `invalid target type` before anything else. Prefer a full editor Compile or `Build.bat` over Ctrl+Alt+F11 whenever practical.
- **Check for a running editor with PowerShell's `Get-Process`, not Bash's `tasklist //FI`**, on Windows — Bash's quoting can silently fail to match a running process. `Get-Process | Where-Object ProcessName -match "Unreal|LiveCoding|Rider"` is reliable. Also invoke `Build.bat` via PowerShell, not Bash — Bash's quoting can mangle the batch file's internal calls even when the outer command looks correctly quoted.
- **`Docs/SessionHandoff.md` is the sole owner of verification status** ("compiled?", "PIE-tested?", "what's next"). `CLAUDE.md`'s Architecture section describes what exists / current-state design — don't restate compile/test status in more than one place, or every status change becomes a multi-file edit.
- **Use targeted reads for large, mostly-static docs** — `Grep` with context or `Read` with offset/limit — instead of reading the whole file when only one section is relevant.
- **Keep task-tracking granularity coarse** — one task per major deliverable/system, not per file.
- **Never commit `.uasset`/binary content changes silently** — flag what changed and let the dev confirm before staging Content/ changes, same as any other consequential action.

## MCP / Editor Tooling (lessons — starting reference from a prior project using unreal-mcp; prune/extend as you hit your own)
- `.mcp.json` only proposes the MCP server; actual enablement lives in `.claude/settings.local.json` (gitignored) — won't exist on a new machine until recreated.
- Prefer calling a tool blind with a guessed schema first over a broad `describe_toolset`-style dump; the error response usually echoes the exact required schema, which is cheaper than a full describe call.
- Simulated PIE input (keypresses, viewport clicks) via MCP does not reliably reach the pawn, and general desktop GUI automation typically can't even resolve the Unreal Editor window as a controllable app — there is usually no reliable PIE-testing automation path. Don't spend calls trying to force real gameplay-input verification headlessly; that needs the dev's hands.
- Poking a replicated/movement-authoritative property directly via an object-property-set tool can silently revert next tick if the owning component re-derives it every frame — a "successful" write isn't proof the state actually changed.
- **AnimGraph editing**: graph-DSL read/write tools often return empty for AnimGraph-specific node types (`AnimGraphNode_*`) — use node-level find/create/connect tools instead. Normal EventGraphs (Blueprint logic) usually work fine with the DSL tools.
- **`FAnimNode_BlendListByBool`'s branch indexing is often inverted from the naive bool-to-int mapping** — verify true/false assignment against a known-working sibling node's wiring, not against intuition, when wiring one from scratch.
- **`BlendSpacePlayer` X/Y pins don't self-document which locomotion variable belongs where** — verify against the blend space asset's own axis-parameter order, never assume by convention. A "wrong locomotion pose plays" bug can look structurally fine (graph fully wired, compiles clean) while the real bug is just which variable feeds which axis.
- Some skeletal-mesh-tooling calls (e.g. socket creation) can silently fail for specific bones with no thrown error, just an oblique log line — if a read-only verification call afterward doesn't confirm the expected result, fall back to driving the real Editor UI for that specific operation instead of retrying the same tool call.
- A `UserWidget` whose root is a `CanvasPanel` reports `(0,0)` desired size — and renders at 0×0 wherever embedded with auto-size enabled — unless at least one of that root panel's direct children has its own auto-size flag set. Check this first on any Widget Blueprint that "is in the hierarchy, not Collapsed, but doesn't draw."
- Editor/PIE state-query tools may reliably reflect the *editor* world even while a PIE session is running, not the live duplicated instance — property/transform queries against the real PIE actors tend to work, but visual/rendering verification (screenshots, visible-actor queries) may show editor-only content instead. Visual/rendering bugs stay dev-hands-only regardless of which query tools are available.

## Local Git Infrastructure (Gitea) + GitHub
**Gitea is the primary remote** — a self-hosted git server running locally (`D:\Dev\Gitea`, Windows service `Gitea` via NSSM, web UI at `http://localhost:3000`, SQLite DB, repo+LFS storage on a dedicated drive so large binaries don't count against GitHub's free-tier LFS budget). **This Gitea instance is shared across every project on this machine — don't reinstall it per-project.** Generating a new repo from this template just needs a new repo created on the already-running instance; see `SETUP.md` for the exact steps. Local remotes once set up: `origin` = Gitea, `github` = this project's GitHub remote. Day-to-day `git push` targets Gitea only — GitHub stays in sync automatically via a configured Push Mirror (repo Settings → Mirror Settings → "Sync when commits are pushed" + an interval fallback).

Repo: `github.com/{{GITHUB_USER}}/{{REPO_NAME}}` — a public repo gets free secret scanning; keep the LFS spending cap at $0 (fail-safe) regardless, since Gitea is the real LFS store now and GitHub is a secondary/off-site copy, not the primary. Never force-push main.
`gh` CLI: on Windows use full path `/c/Program Files/GitHub CLI/gh.exe` if not on PATH.

**Local safety-net mirror**: `Scripts/Backup-Project.ps1` robocopy-mirrors the live working tree (including anything not yet committed) to a dedicated backup folder, excluding regeneratable build/IDE junk. Run it manually anytime, or wire it to a nightly Task Scheduler job (see `SETUP.md`).

## Off-Limits
{{Project-specific scope boundaries go here — e.g. licensed third-party content that can't be committed and how to reinstall it, features explicitly cut from scope, external reference projects not to draw design decisions from. Keep this list honest and current; it's cheap insurance against re-litigating settled decisions.}}

## Reference Docs
- `Docs/CommandReference.md` — ready-to-paste commands.
- `Docs/SessionHandoff.md` — current status only, rewritten each session.
- {{Add design-plan/production-plan docs here as they're created — this section should only ever list docs that actually exist.}}

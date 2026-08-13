# Command Reference

> Exact, ready-to-paste commands for the things Claude runs most often on this project — build, IDE project-file regen, and the git commit/push cycle. Not a workflow change: run these yourself whenever it's more convenient than asking. PowerShell syntax (this machine's primary shell); same commands work from Rider's own terminal or `cmd.exe` with minor quoting differences. Run from the repo root: `C:\Users\aaron\Documents\Unreal Projects\DungeonCat`. Update the engine path below if the project ever moves to a newer UE version.

## Before building: check nothing's holding it open

The editor or Live Coding holding the module locked is the most common cause of a failed build. Bash's `tasklist` is unreliable for this on this machine — use PowerShell:

```powershell
Get-Process | Where-Object { $_.ProcessName -match "Unreal|LiveCoding|Rider" } | Select-Object ProcessName, Id
```

`Rider`/`Rider.Backend` showing up is fine. `UnrealEditor`/`LiveCoding` showing up means close the editor first.

## Build the editor (Development)

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" DungeonCatEditor Win64 Development "-project=C:\Users\aaron\Documents\Unreal Projects\DungeonCat\DungeonCat.uproject" -waitmutex
```

Use after any change, but it's mandatory (not just Live Coding) after a header change or a new module dependency (e.g. the GameplayAbilities/GameplayTags/GameplayTasks additions in P1) — Live Coding can't pick either up.

## Regenerate IDE project files (Rider/VS)

Needed after adding new `.h`/`.cpp` files, or after a session with a lot of file churn:

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" -projectfiles "-project=C:\Users\aaron\Documents\Unreal Projects\DungeonCat\DungeonCat.uproject" -game -engine
```

Rider picks up the regenerated `.sln`/`.slnx` automatically; if it doesn't prompt to reload, use Rider's own "Reload Project." Safe to run with the editor open — unlike a real build, this only rewrites IDE project files, it doesn't touch the Live Coding lock.

## Git: check state before committing

```powershell
git status
git diff --stat
git log --oneline -10
```

## Git: stage, commit, push

Stage specific files rather than `git add -A`/`git add .` — safer against accidentally picking up something unintended:

```powershell
git add <path1> <path2> ...
```

Multi-line commit message (PowerShell here-string — the closing `'@` must sit at column 0, no leading whitespace):

```powershell
git commit -m @'
Short summary line.

Longer explanation if needed.
'@
```

```powershell
git push
```

## Local Gitea server + backup mirror

Check the local git server is running (auto-starts on boot, NSSM restarts it on crash — this should rarely be needed):

```powershell
Get-Service Gitea
```

Web UI: `http://localhost:3000` (localhost only, not reachable elsewhere on the network).

`git push` (no remote arg) targets Gitea (`origin`) by default — GitHub (`github` remote) stays in sync automatically via Gitea's push mirror. Push to GitHub directly only if there's a specific reason to bypass the mirror:

```powershell
git push github
```

Run the working-tree safety-net backup manually (also runs automatically nightly at 3:10 AM via the "DungeonCat Nightly Backup" scheduled task — catches up on next login if the PC was off):

```powershell
& "C:\Users\aaron\Documents\Unreal Projects\DungeonCat\Scripts\Backup-Project.ps1"
```

## Typical end-of-session sequence

1. Check nothing's locking the build (above).
2. Build.
3. Regenerate project files, if new files were added this session.
4. `git status` / `git diff --stat` — confirm what actually changed before staging anything.
5. `git add` the specific files, `git commit`, `git push`.

## Editor close/rebuild for test runs (present-session only, dev-triggered)

This only runs when the dev is present and explicitly asks for it — never unsupervised.

Graceful close attempt — same signal as clicking the window's X, not a force-kill. If there are unsaved changes, Unreal's own "save changes?" dialog blocks the close; there's no safe way to click through that from here (no reliable GUI-automation access to the editor window), so this waits, then stops rather than guessing or forcing:

```powershell
$proc = Get-Process -Name UnrealEditor -ErrorAction SilentlyContinue
if ($proc) {
    $proc.CloseMainWindow() | Out-Null
    $closed = $proc.WaitForExit(20000)
    if ($closed) { Write-Output "CLOSED" } else { Write-Output "STILL_RUNNING - likely a blocking dialog, e.g. unsaved changes - stop here, hand back to the dev" }
} else {
    Write-Output "NOT_RUNNING"
}
```

If `CLOSED`: proceed with the normal build (above). Once `Source/DungeonCat/Tests/` exists, an automation-test run goes here (mirrors `zombieshooter`'s `Automation RunTests ZS.` pattern with a `DC.` prefix) — nothing to run yet, no tests exist as of P1.

Relaunch the editor when done (non-blocking):

```powershell
Start-Process -FilePath "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList '"C:\Users\aaron\Documents\Unreal Projects\DungeonCat\DungeonCat.uproject"'
```

If `STILL_RUNNING`: stop. Don't retry, don't escalate to `Stop-Process`/force-kill, don't attempt to click through the dialog blind. Tell the dev directly.

## If something doesn't compile

Read the actual error, fix it, rebuild with the same command above — don't reach for Live Coding to patch a header change, and don't force past a failure (`--no-verify`, `-c commit.gpgsign=false`, force-push).

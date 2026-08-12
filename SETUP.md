# Setup — turning a repo generated from this template into a working project

One-time steps, in order. Assumes Gitea is already running as a Windows service on this machine (it's shared across every project — you only install it once, ever, not per-project).

## 1. Generate the new repo in Gitea

In the Gitea web UI (`http://localhost:3000`):

1. Go to this template repo's page → **Use this template** (or **New Repository** → set "Template" to this repo, depending on Gitea version).
2. Set the new repo's **Owner** (your account) and **Name** (the new project's name, e.g. `mynewgame`).
3. Leave **Copy default branch only** checked unless you specifically want template branch history — usually you don't; this is a scaffold, not shared history.
4. Create it.

This gives you a fresh Gitea repo containing `CLAUDE.md`, `Docs/`, `Scripts/`, `.gitignore`, `.gitattributes`, `README.md` — no UE project files yet.

## 2. Attach it to your actual Unreal project folder

If you haven't created the UE project yet, do that first (Unreal Editor → New Project). Then, from the UE project's root folder:

```powershell
git init
git remote add origin http://localhost:3000/<your-username>/<new-repo-name>.git
git pull origin main --allow-unrelated-histories
```

(`--allow-unrelated-histories` is needed because the UE project folder and the freshly generated Gitea repo don't share a common git ancestor.) If you generated the repo first and are building the UE project into that same cloned folder instead, skip straight to opening the project in the Unreal Editor there — no `git init`/`pull` needed.

## 3. Fill in the placeholders

Search the repo for `{{` and replace every placeholder:

- `{{PROJECT_NAME}}` — the project/module name (matches the `.uproject` filename and the `Source/<Name>/` folder).
- `{{PREFIX}}` — your class-naming prefix (2-4 letters, e.g. `ZS`). Pick once, don't change later.
- `{{PROJECT_ROOT}}` — the absolute path to the project folder on this machine.
- `{{UE_VERSION_FOLDER}}` — e.g. `UE_5.8`, matching your actual Epic Games engine install folder.
- `{{TEST_PREFIX}}` — the automation-test category prefix if you set one up (e.g. `ZS.` in the origin project).
- `{{GITHUB_USER}}` / `{{REPO_NAME}}` — once you've created the GitHub secondary repo (step 5).

Also flesh out `CLAUDE.md`'s empty `## Architecture` and `## Off-Limits` sections as the project actually grows — don't leave them as placeholders indefinitely, but don't pre-fill them speculatively either.

## 4. First commit

```powershell
git add CLAUDE.md Docs .gitignore .gitattributes Scripts <your UE project files>
git commit -m "Initial project setup from GameDevTemplate"
git push -u origin main
```

## 5. Add GitHub as a secondary mirror (optional but recommended)

1. Create an empty repo on GitHub (no README/license/gitignore — keep it empty so the push doesn't conflict).
2. In this project's local clone:
   ```powershell
   git remote add github https://github.com/<GITHUB_USER>/<REPO_NAME>.git
   git push github main
   ```
3. In Gitea, go to this repo's **Settings → Repository → Mirror Settings** and add a **Push Mirror**:
   - Git Remote Repository URL: the GitHub URL above.
   - Authorization: your GitHub username + a GitHub Personal Access Token scoped to `repo`.
   - Check **"Sync when commits are pushed"**, and set an interval fallback (e.g. `8h0m0s`).
4. Click **Synchronize Now** once to confirm it actually reaches GitHub.

From here on, `git push` (no remote arg) goes to Gitea only — GitHub stays current automatically.

## 6. Set up the backup script + scheduled task

1. Fill in `Scripts/Backup-Project.ps1`'s three placeholder variables (`$SourcePath`, `$DestPath`, `$LogDir`) — see the comments in the file.
2. Run it once manually to confirm it works:
   ```powershell
   & "{{PROJECT_ROOT}}\Scripts\Backup-Project.ps1"
   ```
3. Register the nightly scheduled task (adjust the `-File` path and task name per project):
   ```powershell
   $action = New-ScheduledTaskAction -Execute "powershell.exe" -Argument '-NoProfile -ExecutionPolicy Bypass -File "{{PROJECT_ROOT}}\Scripts\Backup-Project.ps1"'
   $trigger = New-ScheduledTaskTrigger -Daily -At 3:00AM
   $settings = New-ScheduledTaskSettingsSet -StartWhenAvailable -DontStopOnIdleEnd -ExecutionTimeLimit (New-TimeSpan -Hours 1)
   Register-ScheduledTask -TaskName "{{PROJECT_NAME}} Nightly Backup" -Action $action -Trigger $trigger -Settings $settings -Description "Nightly robocopy mirror of {{PROJECT_NAME}} working tree."
   ```

## 7. Rider / IDE

Nothing special — add the Gitea remote URL, authenticate once (HTTPS + a Gitea personal access token generated under Gitea's `Settings → Applications`, or SSH if you'd rather set that up), and Rider's Git tooling works against it exactly like it does against GitHub.

## Notes carried over from the origin project

- Gitea's frontend can be finicky to drive via non-interactive/scripted browser automation (Vue-based forms sometimes need an explicit `input`/`change` event dispatched after setting a field's value, not just the raw value set, or the submission silently no-ops) — irrelevant for normal human use of the web UI, only matters if you're scripting bulk repo setup again.
- Don't run this project's Gitea service install steps again — check `Get-Service Gitea` first; if it's already running, you only need step 1 above (a new repo), never a new install.

# Game Dev Template

A reusable scaffold for a new Unreal Engine project, carrying over the git/backup/AI-assistant workflow from a prior project rather than the game code itself: local-Gitea-primary + GitHub-secondary version control, a working-tree backup script, and the `CLAUDE.md`/`Docs/` documentation conventions that keep an AI coding assistant oriented session to session.

**This is a workflow scaffold, not a UE project skeleton.** There's no `.uproject`, no `Source/`, no `Content/` here — create your Unreal project normally, then drop this scaffold in on top of it (or generate a repo from this template first and build the UE project into that same folder — either order works).

## What's in here

- `CLAUDE.md` — genericized project-instructions template with `{{PLACEHOLDER}}` fields to fill in, plus a set of Unreal/MCP-tooling lessons and workflow conventions worth keeping from the start rather than re-learning.
- `Docs/CommandReference.md` — ready-to-paste build/git/backup commands, same placeholders.
- `Docs/SessionHandoff.md` — the empty "current status" doc, ready for its first real entry.
- `Scripts/Backup-Project.ps1` — the working-tree safety-net mirror script, parameterized.
- `.gitignore` / `.gitattributes` — Unreal + Git LFS patterns.

## Getting started

See **`SETUP.md`** for the full one-time setup: creating the new project's Gitea repo, filling in the placeholders, wiring up the GitHub push mirror, and registering the nightly backup task.

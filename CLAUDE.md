# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

**Read `Docs/SessionHandoff.md` first, every session** — it is kept short on purpose and only covers the last completed task and the immediate next step. Full history lives in the git commit log, not in this repo's docs.

`Docs/GameDevPlan.md` is the design plan of record for the actual game (co-op ARPG dungeon crawler — GAS, procedural dungeons, bipedal cat, 2-month beta scope) — see the Project overview note below on how that relates to what's currently in this repo. `Docs/SystemsDesign.md` is the technical spec (naming convention, GAS/combat, replication, dungeon generation, AI, itemization, UI) that engineering builds against. `Docs/AssetPipeline.md` is the Blender-facing spec (skeleton, sockets, animation list, dungeon trim-kit grid, naming) for self-authored art. `Docs/ProductionPlan.md` breaks the beta roadmap into phases (P0-P4) with exit criteria. `Docs/CommandReference.md` has ready-to-paste build/git/backup/editor-close commands. `Docs/AsyncSessionProtocol.md` = behavioral protocol for a flagged "away session" (dev says something like "this is an away session") — read once at the start of one, not auto-loaded otherwise.

## Project overview

DungeonCat is an Unreal Engine 5.8 C++ project. The repo currently contains Epic's stock "Third Person" template starter content, extended with three self-contained gameplay variants (Combat, Platforming, SideScrolling) — this is the starting scaffold, not the destination. **The actual target game is described in `Docs/GameDevPlan.md`**: a co-op ARPG dungeon crawler starring a bipedal cartoon cat, built on GAS (Gameplay Ability System) with server-authoritative replication from the start. None of that has been built yet as of 2026-08-12 — treat the stock variants as reference/scaffold to reuse or replace, not as the finished product. Engine install: `C:\Program Files\Epic Games\UE_5.8`.

## Build commands

Day-to-day iteration is normally done by opening `DungeonCat.sln` (or `.slnx`) in Visual Studio and building the `Development Editor` configuration, or by using Live Coding from the running editor. Command-line equivalents:

Build the editor target:
```
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" DungeonCatEditor Win64 Development -Project="C:\Users\aaron\Documents\Unreal Projects\DungeonCat\DungeonCat.uproject" -WaitMutex
```

Build the standalone game target:
```
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" DungeonCat Win64 Development -Project="C:\Users\aaron\Documents\Unreal Projects\DungeonCat\DungeonCat.uproject" -WaitMutex
```

Regenerate Visual Studio project files (after adding/removing/renaming source files):
```
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealBuildTool.exe" -projectfiles -project="C:\Users\aaron\Documents\Unreal Projects\DungeonCat\DungeonCat.uproject" -game -engine
```

Launch the editor:
```
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\aaron\Documents\Unreal Projects\DungeonCat\DungeonCat.uproject"
```

## Architecture

- Single game module `DungeonCat` (Runtime; see `Source/DungeonCat/DungeonCat.Build.cs`), depending on Core/CoreUObject/Engine/InputCore/EnhancedInput/AIModule/StateTreeModule/GameplayStateTreeModule/UMG/Slate.
- Shared base classes at `Source/DungeonCat/`: `DungeonCatCharacter`, `DungeonCatGameMode`, `DungeonCatPlayerController`.
- Three gameplay variants live side by side under `Source/DungeonCat/Variant_*`, each with its own GameMode/Character/PlayerController and a matching level under `Content/Variant_*`:
  - **Variant_Combat** — melee combat sandbox. StateTree-driven enemy AI (`AI/CombatAIController`, `CombatEnemy`, `CombatEnemySpawner`, `CombatStateTreeUtility`, EQS contexts); attack/damage contracts as interfaces (`Interfaces/CombatAttacker`, `CombatDamageable`, `CombatActivatable`); combo/charged-attack/attack-trace driven by `Animation/AnimNotify_*`; hazards and checkpoints in `Gameplay/` (`CombatLavaFloor`, `CombatCheckpointVolume`, `CombatDamageableBox`, `CombatActivationVolume`).
  - **Variant_Platforming** — dash-focused platformer character (`PlatformingCharacter`), dash end signaled via `Animation/AnimNotify_EndDash`.
  - **Variant_SideScrolling** — side-scrolling character with a dedicated `SideScrollingCameraManager`; StateTree NPC AI (`AI/SideScrollingAIController`, `SideScrollingNPC`, `SideScrollingStateTreeUtility`); `Interfaces/SideScrollingInteractable` contract; level gameplay actors in `Gameplay/` (jump pads, moving/soft platforms, pickups); UI in `UI/SideScrollingUI`.
- Where a variant needs AI behavior, it's implemented with the StateTree / GameplayStateTree plugins rather than classic Behavior Trees — look for `*StateTreeUtility` classes exposing the conditions/considerations used by the state trees (authored in `Content/`, not in C++).
- The `.uproject` has Epic's `ModelContextProtocol` plugin enabled, so the running Unreal Editor can expose itself over MCP to external tooling — relevant if a task calls for driving the live editor rather than just editing source/assets.

## MCP / Editor Tooling

`.mcp.json` proposes the `unreal-mcp` server (`http://127.0.0.1:8000/mcp`, Epic's built-in `ModelContextProtocol` plugin — not a third-party bridge); actual enablement lives in `.claude/settings.local.json` (gitignored via a global `~/.config/git/ignore` rule, so it won't exist on a new machine until recreated). Only one Unreal Editor instance can bind port 8000 at a time, so this only works while *this* project's editor is the one open.

## Local Git Infrastructure (Gitea) + GitHub

Same pattern as this dev's other UE5 projects (see `zombieshooter`): Gitea is the primary remote — a self-hosted git server running locally (`D:\Dev\Gitea`, Windows service via NSSM, web UI at `http://localhost:3000`). Local remotes: `origin` = Gitea (`http://localhost:3000/aaronrod/dungeoncat.git`), `github` = public GitHub mirror (`https://github.com/aaronrod3/dungeoncat`). Day-to-day `git push` should target `origin` (Gitea) once it syncs to GitHub automatically via a configured Push Mirror (Gitea repo Settings → Mirror Settings) — that mirror still needs to be configured by hand in the Gitea web UI (requires a GitHub token entered into a form, a step Claude Code won't do on the dev's behalf). Until then, push to both remotes manually, or just `github` if Gitea's own repo isn't set up yet. Git LFS is enabled (`.gitattributes`) for `.uasset`/`.umap`/`.fbx`/textures/audio/etc. Never force-push `main`.

**Local safety-net mirror**: `Scripts/Backup-Project.ps1` robocopy-mirrors the live working tree (including anything not yet committed) to `D:\Dev\Backups\DungeonCat\Live`, excluding regeneratable build/IDE junk. Runs automatically via the "DungeonCat Nightly Backup" Task Scheduler job (3:10 AM daily, catches up on next login if the PC was off). Logs to `D:\Dev\Backups\DungeonCat\Logs`.

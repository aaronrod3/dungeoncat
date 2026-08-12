# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

DungeonCat is an Unreal Engine 5.8 C++ project built from Epic's stock "Third Person" template starter content, extended with three self-contained gameplay variants (Combat, Platforming, SideScrolling). Engine install: `C:\Program Files\Epic Games\UE_5.8`.

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

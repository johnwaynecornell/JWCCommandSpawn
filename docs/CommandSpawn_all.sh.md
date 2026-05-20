# CommandSpawn All-In-One Fresh Workspace Build

`CommandSpawn_all.sh` creates or updates a local NewAge workspace, builds `JWCEssentials`, then builds `JWCCommandSpawn`.

The script is intended for clean-machine testing, temporary workspace proof runs, native collection, and packaging preparation.

## Behavior

*   Creates a local `NewAge` directory if needed.
*   Exports `NewAge` for the current script process.
*   Configures the native runtime/build lane.
*   Clones each repository if missing.
*   Fetches, switches, and fast-forwards each repository if already present.
*   Runs each repository's `configure.sh`.
*   Builds native and managed outputs.

## Usage

```
Dev/CommandSpawn_all.sh
```

## Repositories

```
JWCEssentials
JWCCommandSpawn
```

## Native Lanes

```
Debug/Windows/AMD64/msvc
Debug/Windows/AMD64/clang-cl
Debug/Linux/x86_64/gcc
Debug/Linux/x86_64/clang
```

## Notes

This script does not require the repositories to already exist. If they do exist, they must be Git repositories and must be able to fast-forward on the selected branch.

The script intentionally uses `--newage` so it can build inside a fresh local workspace without requiring system-wide environment changes.

See docs in [JWCEssentials on GitHub](https://github.com/johnwaynecornell/JWCEssentials) for help meeting build prerequisites. If they are fulfilled you may proceed here

## CommandSpawn_all.sh

```
#!/usr/bin/env bash
set -euo pipefail

# This script may be stored and run or updated to suit it does not require NewAge to pre-exist

# Fail gracefully if no directory argument is passed
target_dir="${1:-}"
if [[ -z "$target_dir" ]]; then
    echo "Error: Target directory required. Suggestions, 'Purpose'.NewAge or Home.NewAge"
    exit 1
fi

mkdir -p "$target_dir"
cd "$target_dir"

git clone https://github.com/johnwaynecornell/JWCEssentials
cd JWCEssentials
cd ..

JWCEssentials/configure.sh --newage "$(pwd)"

# 1. Setup conditional arguments
ctx_args=("Debug")
if [[ -n "${NewAge_Lane:-}" ]]; then
    ctx_args+=("$NewAge_Lane")
fi

# 2. Execute a single context wrapper
./in_this_context.sh "${ctx_args[@]}" -- bash -e -c '
    newage_get.sh JWCCommandSpawn https://github.com/johnwaynecornell/JWCCommandSpawn
    newage_get_deps.sh
    newage_all_configure.sh
    newage_all_build_coordinated.sh Debug
'
```
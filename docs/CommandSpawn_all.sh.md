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
./CommandSpawn_all.sh
```

## Repositories

```
JWCEssentials
JWCCommandSpawn
```

## Native Lanes

```
Debug/Windows/AMD64/msvc
Debug/Linux/x86_64/gcc
```

## Notes

This script does not require the repositories to already exist. If they do exist, they must be Git repositories and must be able to fast-forward on the selected branch.

The script intentionally uses `--newage` so it can build inside a fresh local workspace without requiring system-wide environment changes.

## CommandSpawn\_all.sh

```
#!/usr/bin/env bash
set -euo pipefail

newage_is_windows_shell() {
    case "$(uname -s)" in
        CYGWIN*|MINGW*|MSYS*) return 0 ;;
        *) return 1 ;;
    esac
}

sync_repo() {
    local url="$1"
    local dir="$2"
    local branch="$3"

    if [ -d "$dir/.git" ]; then
        echo "[CommandSpawn_all] Updating $dir"
        git -C "$dir" fetch origin
        git -C "$dir" switch "$branch"
        git -C "$dir" pull --ff-only origin "$branch"
    elif [ -e "$dir" ]; then
        echo "[CommandSpawn_all] ERROR: $dir exists but is not a git repo." >&2
        exit 1
    else
        echo "[CommandSpawn_all] Cloning $url -> $dir"
        git clone "$url" "$dir"
        git -C "$dir" switch "$branch"
    fi
}

build_native_and_managed() {
    local repo_dir="$1"
    local managed_project="$2"

    cd "$repo_dir"

    cmake CMakeLists.txt
    cmake --build .

    dotnet build "$managed_project"
}

mkdir -p NewAge
cd NewAge

export NewAge="$(pwd)"
export PATH="$NewAge/bin:$PATH"

lane=""

if newage_is_windows_shell; then
    lane="Debug/Windows/AMD64/msvc"
    export PATH="$PATH:$NewAge/lib/$lane"
else
    lane="Debug/Linux/x86_64/gcc"
    export LD_LIBRARY_PATH="$NewAge/lib/$lane:${LD_LIBRARY_PATH:-}"
fi

export PATH="$PATH:$NewAge/bin/$lane"

sync_repo \
    "https://github.com/johnwaynecornell/JWCEssentials" \
    "$NewAge/JWCEssentials" \
    "main" # "scratch/integration-refactor"

cd "$NewAge/JWCEssentials"
./configure.sh --newage "$NewAge"
build_native_and_managed "$NewAge/JWCEssentials" "Project/JWCEssentials.net/"

sync_repo \
    "https://github.com/johnwaynecornell/JWCCommandSpawn" \
    "$NewAge/JWCCommandSpawn" \
    "main" # "scratch/integration-refactor"

cd "$NewAge/JWCCommandSpawn"
./configure.sh --newage "$NewAge"
build_native_and_managed "$NewAge/JWCCommandSpawn" "Project/JWCCommandSpawn.net/"

echo
echo "[CommandSpawn_all] Complete."
echo
echo "NewAge:"
echo "  $NewAge"
echo
echo "Native lane:"
echo "  $lane"
echo
echo "Repos:"
echo "  $NewAge/JWCEssentials"
echo "  $NewAge/JWCCommandSpawn"
```
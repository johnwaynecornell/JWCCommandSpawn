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

See docs in [JWCEssentials on GitHub](https://github.com/johnwaynecornell/JWCEssentials) for help meeting build prerequisites. If they are fulfilled you may proceed here

## CommandSpawn\_all.sh

```
#!/usr/bin/env bash
set -euo pipefail

# This script is intended for clean-machine testing, temporary workspace proof
#    runs, native collection, and packaging preparation.
# It does not requirw a prior clone to run and will fetch the JWCEssentials
#   and JWCCommandSpawn repos
# See https://github.com/johnwaynecornell.net/JWCEssentials/docs for help meeting
#   build prerequisites

newage_is_windows_shell() {
    case "$(uname -s)" in
        CYGWIN*|MINGW*|MSYS*) return 0 ;;
        *) return 1 ;;
    esacs
}

usage() {
    cat <&2
            usage >&2
            exit 1
            ;;

        *)
            if [ -z "$WORKSPACE_DIR" ]; then
                WORKSPACE_DIR="$1"
            else
                echo "[CommandSpawn_all] ERROR: Unexpected argument: $1" >&2
                usage >&2
                exit 1
            fi
            ;;
    esac
    shift
done

if [ -z "$WORKSPACE_DIR" ]; then
    echo "[CommandSpawn_all] ERROR: WORKSPACE_DIR is required." >&2
    usage >&2
    exit 1
fi

case "$BUILD_MODE" in
    Debug|debug)
        BUILD_CONFIGS=("Debug")
        ;;
    Release|release)
        BUILD_CONFIGS=("Release")
        ;;
    Both|both|All|all)
        BUILD_CONFIGS=("Debug" "Release")

        # In-place CMake builds can preserve stale cache/configuration state
        # across Debug/Release transitions, so make Both/All safe by default.
        FRESH="1"
        ;;
    *)
        echo "[CommandSpawn_all] ERROR: Unknown build mode: $BUILD_MODE" >&2
        usage >&2
        exit 1
        ;;
esac

echo "[CommandSpawn_all] Workspace: $WORKSPACE_DIR"
echo "[CommandSpawn_all] Build mode: $BUILD_MODE"
echo "[CommandSpawn_all] Fresh configure: $FRESH"
echo "[CommandSpawn_all] Target clean: $CLEAN"

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

set_lane_environment() {
    local config="$1"

    if newage_is_windows_shell; then
        lane="$config/Windows/AMD64/msvc"
        export PATH="$NewAge/lib/$lane:$PATH"
    else
        lane="$config/Linux/x86_64/gcc"
        export LD_LIBRARY_PATH="$NewAge/lib/$lane:${LD_LIBRARY_PATH:-}"
    fi

    export PATH="$NewAge/bin/$lane:$NewAge/bin:$PATH"
}

build_native_and_managed() {
    local repo_dir="$1"
    local managed_project="$2"
    local config="$3"

    cd "$repo_dir"

    if [ "$FRESH" = "1" ]; then
        rm -f CMakeCache.txt
        rm -rf CMakeFiles
        rm -f cmake_install.cmake
        rm -f Makefile
        rm -f *.sln
        rm -f *.vcxproj
        rm -f *.vcxproj.filters
        rm -f *.vcxproj.user
    fi

    cmake CMakeLists.txt -DCMAKE_BUILD_TYPE="$config"

    if [ "$CLEAN" = "1" ]; then
        cmake --build . --config "$config" --target clean
    fi

    cmake --build . --config "$config"

    dotnet build "$managed_project" -c "$config"
}

mkdir -p "$WORKSPACE_DIR"
cd "$WORKSPACE_DIR"

export NewAge="$(pwd)"

sync_repo \
    "https://github.com/johnwaynecornell/JWCEssentials" \
    "$NewAge/JWCEssentials" \
    "main" # "scratch/integration-refactor"

sync_repo \
    "https://github.com/johnwaynecornell/JWCCommandSpawn" \
    "$NewAge/JWCCommandSpawn" \
    "main" # "scratch/integration-refactor"

configure_workspace_repos() {
    cd "$NewAge/JWCEssentials"
    ./configure.sh --newage "$NewAge"

    cd "$NewAge/JWCCommandSpawn"
    ./configure.sh --newage "$NewAge"
}

configure_workspace_repos

for config in "${BUILD_CONFIGS[@]}"; do
    echo
    echo "[CommandSpawn_all] Building configuration: $config"
    echo

    set_lane_environment "$config"

    build_native_and_managed "$NewAge/JWCEssentials" "Project/JWCEssentials.net/" "$config"
    build_native_and_managed "$NewAge/JWCCommandSpawn" "Project/JWCCommandSpawn.net/" "$config"
done

echo
echo "[CommandSpawn_all] Complete."
echo
echo "NewAge:"
echo "  $NewAge"
echo
echo "Build configurations:"
for config in "${BUILD_CONFIGS[@]}"; do
    echo "  $config"
done
```
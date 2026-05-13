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

usage() {
    cat <<EOF
Usage:
  ./CommandSpawn_all.sh WORKSPACE_DIR

Example:
  ./CommandSpawn_all.sh NewAge
  ./CommandSpawn_all.sh /tmp/NewAge_CommandSpawn_test

Notes:
  WORKSPACE_DIR will be created if it does not exist.
  The script will use WORKSPACE_DIR as the NewAge workspace root.
  This script pulls if the repos preexist, copy and tailor suite
EOF
}

if [ "$#" -ne 1 ]; then
    usage >&2
    exit 1
fi

WORKSPACE_DIR="$1"

mkdir -p "$WORKSPACE_DIR"
cd "$WORKSPACE_DIR"

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
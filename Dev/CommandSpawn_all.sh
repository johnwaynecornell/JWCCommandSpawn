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
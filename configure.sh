#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"
REPO_REL_PATH="JWCCommandSpawn"

NEWAGE_CONFIGURE_SCOPE="JWCCommandSpawn configure"

if [ -z "${NewAge:-}" ]; then
    echo "[JWCCommandSpawn configure] ERROR: NewAge is not set." >&2
    echo "see https://github.com/johnwaynecornell/JWCEssentials if you haven't yet" >&2
    exit 1
fi

if [ ! -f "$NewAge/JWCEssentials/Dev/NewAge.dev.sh" ]; then
    echo "[JWCCommandSpawn configure] ERROR: JWCEssentials development helpers were not found." >&2
    echo "[JWCCommandSpawn configure] Expected: $NewAge/JWCEssentials/Dev/NewAge.dev.sh" >&2
    echo "[JWCCommandSpawn configure] Run JWCEssentials/configure.sh first." >&2
    echo "[JWCCommandSpawn configure] See: https://github.com/johnwaynecornell/JWCEssentials" >&2
    exit 1
fi

. "$NewAge/JWCEssentials/Dev/NewAge.dev.sh"

newage_register_repo_root "$REPO_ROOT" "$REPO_REL_PATH"

newage_register_directory \
    "$REPO_ROOT/include/JWCCommandSpawn" \
    "$NewAge/include/JWCCommandSpawn" \
    required

newage_log "Workspace setup complete."
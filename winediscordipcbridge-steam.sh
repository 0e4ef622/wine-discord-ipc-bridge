#!/bin/bash

# Run a Steam Play game with wine-discord-ipc-bridge
# Set the game's launch option to: path/to/this-script.sh %command%

# Change BRIDGE to the path of winediscordipcbridge.exe
# Defaults to looking in the same directory as the script.
BRIDGE="$(dirname ${BASH_SOURCE[0]})/winediscordipcbridge.exe"

TEMP_PATH="$XDG_RUNTIME_DIR"
TEMP_PATH=${TEMP_PATH:-"$TMPDIR"}
TEMP_PATH=${TEMP_PATH:-"$TMP"}
TEMP_PATH=${TEMP_PATH:-"$TEMP"}
TEMP_PATH=${TEMP_PATH:-"/tmp"}

for i in {0..9}; do
    if [ -S "$TEMP_PATH/discord-ipc-$i" ]; then
        IPC_PATH="$TEMP_PATH/discord-ipc-$i";
        break;
    fi
done

PROTON_REMOTE_DEBUG_CMD="$BRIDGE" PRESSURE_VESSEL_FILESYSTEMS_RW="$IPC_PATH" "$@"

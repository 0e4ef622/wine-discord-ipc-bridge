#!/bin/bash

# Run a Steam Play game with wine-discord-ipc-bridge
# Set the game's launch option to: path/to/this-script.sh %command%

# Change BRIDGE_PATH to the path of winediscordipcbridge.exe
# Defaults to looking in the same directory as the script.
BRIDGE_PATH="$(dirname ${BASH_SOURCE[0]})"
BRIDGE="$BRIDGE_PATH/winediscordipcbridge.exe"

TEMP_PATH="$XDG_RUNTIME_DIR"
TEMP_PATH=${TEMP_PATH:-"$TMPDIR"}
TEMP_PATH=${TEMP_PATH:-"$TMP"}
TEMP_PATH=${TEMP_PATH:-"$TEMP"}
TEMP_PATH=${TEMP_PATH:-"/tmp"}

# Attempt to include the discord-ipc-* entries and the bridge path
# in the pressure vessel container mount directly.
# https://github.com/0e4ef622/wine-discord-ipc-bridge/issues/22
VESSEL_PATH="$BRIDGE"
DISCORD_IPC_PATHS=(
    "$TEMP_PATH"
    "$TEMP_PATH/app/com.discordapp.Discord"
    "$TEMP_PATH/snap.discord-canary"
    "$TEMP_PATH/snap.discord"
    "/run/user/$UID"
)

for ipc_path in "${DISCORD_IPC_PATHS[@]}"; do
    if [ -S "$ipc_path"/discord-ipc-? ]; then
        VESSEL_PATH="$BRIDGE:$(echo "$ipc_path"/discord-ipc-?)"
        break
    fi
done

PROTON_REMOTE_DEBUG_CMD="$BRIDGE" PRESSURE_VESSEL_FILESYSTEMS_RW="$VESSEL_PATH:$PRESSURE_VESSEL_FILESYSTEMS_RW" "$@"

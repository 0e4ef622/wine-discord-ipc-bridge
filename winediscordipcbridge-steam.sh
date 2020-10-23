#!/bin/sh

# Run a Steam Play game with wine-discord-ipc-bridge
# Set the game's launch options to: /path/to/this-script.sh %command%

BRIDGE=/path/to/winediscordipcbridge.exe.so
DELAY=10 # how many seconds to wait after starting the bridge before starting the game

"$1" run "$BRIDGE" &
sleep "$DELAY"
"$1" run "${@:3}"


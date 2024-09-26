Wine Discord IPC Bridge
=======================

This program enables other programs which are running under Wine to send Rich
Presence data to a Linux Discord client.

This is mainly a proof-of-concept, demonstrating that this is possible, the
code itself is a mish-mash of copy-paste from Microsoft docs and
[linux-discord-rpc.dll](https://github.com/goeo-/discord-rpc/blob/linux-under-wine/src/connection_win.cpp).

The way it works is simply by bridging the gap between Windows named pipes
(`\\.\pipe\discord-ipc-0`) and Unix domain sockets
(`/run/user/{userid}/discord-ipc-0`).

Compiling
=========

    i686-w64-mingw32-gcc -masm=intel main.c -o winediscordipcbridge

Usage (Wine)
============

Start the bridge first, wait for it to start listening to the pipe, and
then launch your program/game. The two programs need to be running under the
same wineprefix.

Usage (Steam Proton)
====================

1. Download `winediscordipcbridge-steam.sh` and place it in the same directory as
`winediscordipcbridge.exe`. Use `chmod +x` to make it executable if necessary.

2. In the game's settings on Steam, edit "Launch Options" to
   `/path/to/winediscordipcbridge-steam.sh %command%`. You will have to do this
   for every game that supports Rich Presence.

This currently starts the bridge in the background to act as a 'debugger' before
the game starts and opens up the folder the Linux Discord socket lives in to be
read/write. This worked for Deep Rock Galactic, hopefully the timing of the
bridge works correctly for other games.

The bridge should automatically stop once the connecting program terminates, although
it'll keep running if nothing ever connects.

I've personally tested this with osu! on both 32-bit and 64-bit wineprefixes,
and Muse Dash using the script to launch from Steam with Proton Experimental.

https://github.com/truckersmp-cli/truckersmp-cli also [reported success](https://github.com/0e4ef622/wine-discord-ipc-bridge/issues/6#issuecomment-712266806).

Related Projects
================
* https://github.com/Techno-coder/macOS-wine-bridge (fork)
* https://github.com/EnderIce2/rpc-bridge (alternative implementation)
* https://github.com/openglfreak/winestreamproxy (alternative implementation)

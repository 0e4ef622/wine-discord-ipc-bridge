Wine Discord IPC Bridge
=======================

This is the `winegcc` branch, where the code isn't nearly as janky.

This program enables other programs which are running under Wine to send Rich
Presence data to a Linux Discord client.

This is simply a proof-of-concept, demonstrating that this is possible, the
code itself is a mish-mash of copy-paste from Microsoft docs and
[linux-discord-rpc.dll](https://github.com/goeo-/discord-rpc/blob/linux-under-wine/src/connection_win.cpp).

The way it works is simply by bridging the gap between Windows named pipes
(`\\.\pipe\discord-ipc-0`) and Unix domain sockets
(`/run/user/{userid}/discord-ipc-0`). Ideally something like this would be
implemented in wine itself, it should be possible to add some sort of "mapped
named pipe" feature where `wineserver` will serve handles to unix sockets
instead pseudo-handles to named pipes, but I don't know how to approach
something like that.

Compiling
=========

    winegcc -m32 main.c -o winediscordipcbridge

Or, for 64-bit:

    winegcc main.c -o winediscordipcbridge

The binary included in the repository is 32-bit.

Usage
=====

To run the bridge, either call `winediscordipcbridge.exe`, or `wine
winediscordipcbridge.exe.so`. The former is just a script that does the latter.

Just run this program first, wait for it to start listening to the pipe, and
then launch your program/game. The two programs need to be running under the
same wineprefix. If you're using Steam Play/Proton, download and follow the
instructions in `winediscordipcbridge-steam.sh`.

This program should automatically stop once the other program stops.

Disclaimer
==========

I've only tested this with osu! on Arch Linux with a 32-bit wineprefix.

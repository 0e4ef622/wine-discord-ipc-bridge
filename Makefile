# optional flags that make the stripped binary smaller
EXTRAFLAGS = -D__USE_MINGW_ANSI_STDIO=0 -Wl,--disable-reloc-section -fno-asynchronous-unwind-tables -O2

winediscordipcbridge.exe: main.c
	i686-w64-mingw32-gcc $(EXTRAFLAGS) -masm=intel main.c -o winediscordipcbridge

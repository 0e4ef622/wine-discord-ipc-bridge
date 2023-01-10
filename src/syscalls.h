#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__

#include <windows.h>

typedef struct sockaddr_un {
	unsigned short sun_family;               /* AF_UNIX */
	char           sun_path[108];            /* pathname */
} sockaddr_un;

#define AF_UNIX     1
#define SOCK_STREAM     1
#define F_SETFL 4
#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_CREAT     00000100
#define O_APPEND    00002000
#define O_NONBLOCK  00004000
#define BUFSIZE 2048 // size of read/write buffers
#define SFUNC __declspec(naked) // inline asm function
#define CFUNC	// C function

// linux syscall wrappers
SFUNC int l_close(int);
SFUNC int l_socketcall(int, void*);
SFUNC int l_open(const char*, int, int);
SFUNC int l_write(unsigned int, const char*, unsigned int);
SFUNC int l_read(unsigned int, char*, unsigned int);
CFUNC int l_socket(int, int, int);
CFUNC int l_connect(int, const struct sockaddr*, unsigned int);
CFUNC char* getenv_(char*);
CFUNC const char* get_temp_path();

#endif

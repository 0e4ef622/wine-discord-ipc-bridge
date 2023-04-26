#include <string.h>
#include "syscalls.h"

// syscall wrappers
SFUNC int l_close(int fd)
{
	__asm__ (
	    "push ebx\n\t"
	    "mov eax, 0x06\n\t"
	    "mov ebx, [esp + 4 + 4]\n\t"
	    "int 0x80\n\t"
	    "pop ebx\n\t"
	    "ret"
	);
}

SFUNC int l_socketcall(int call, void* args)
{
	__asm__ (
	    "push ebx\n\t"
	    "mov eax, 0x66\n\t"
	    "mov ebx, [esp + 4 + 4]\n\t"
	    "mov ecx, [esp + 4 + 8]\n\t"
	    "int 0x80\n\t"
	    "pop ebx\n\t"
	    "ret"
	);
}

SFUNC int l_open(const char* filename, int flags, int mode)
{
	__asm__ (
	    "push ebx\n\t"
	    "mov eax, 0x05\n\t"
	    "mov ebx, [esp + 4 + 4]\n\t"
	    "mov ecx, [esp + 4 + 8]\n\t"
	    "mov edx, [esp + 4 + 12]\n\t"
	    "int 0x80\n\t"
	    "pop ebx\n\t"
	    "ret"
	);
}

SFUNC int l_write(unsigned int fd, const char* buf, unsigned int count)
{
	__asm__ (
	    "push ebx\n\t"
	    "mov eax, 0x04\n\t"
	    "mov ebx, [esp + 4 + 4]\n\t"
	    "mov ecx, [esp + 4 + 8]\n\t"
	    "mov edx, [esp + 4 + 12]\n\t"
	    "int 0x80\n\t"
	    "pop ebx\n\t"
	    "ret"
	);
}

SFUNC int l_read(unsigned int fd, char* buf, unsigned int count)
{
	__asm__ (
	    "push ebx\n\t"
	    "mov eax, 0x03\n\t"
	    "mov ebx, [esp + 4 + 4]\n\t"
	    "mov ecx, [esp + 4 + 8]\n\t"
	    "mov edx, [esp + 4 + 12]\n\t"
	    "int 0x80\n\t"
	    "pop ebx\n\t"
	    "ret"
	);
}

// socket wrappers
CFUNC int l_socket(int domain, int type, int protocol)
{
	void* args[3];
	args[0] = (void*)(int*)domain;
	args[1] = (void*)(int*)type;
	args[2] = (void*)(int*)protocol;
	return l_socketcall(1, args);
}

CFUNC int l_connect(int sockfd, const struct sockaddr *addr, unsigned int addrlen)
{
	void* args[3];
	args[0] = (void*)(int*)sockfd;
	args[1] = (void*)addr;
	args[2] = (void*)(int*)addrlen;
	return l_socketcall(3, args);
}
/* int send(int sockfd, const void* buf, unsigned int len, int flags) { */
/*     void* args[4]; */
/*     args[0] = (void*)(int*)sockfd; */
/*     args[1] = (void*)buf; */
/*     args[2] = (void*)(unsigned int*)len; */
/*     args[3] = (void*)(int*)flags; */
/*     return l_socketcall(9, args); */
/* } */
/* int recv(int fd, void* buf, unsigned int len, int flags) { */
/*     void* args[4]; */
/*     args[0] = (void*)(int*)fd; */
/*     args[1] = (void*)buf; */
/*     args[2] = (void*)(unsigned int*)len; */
/*     args[3] = (void*)(int*)flags; */
/*     return l_socketcall(10, args); */
/* } */

// env wrappers
CFUNC char* getenv_(char* name) // written by https://github.com/Francesco149
{
	static char buf[1024 * 1024];
	static char* end = 0;
	unsigned int namelen;
	char* p;
	if (!end) {
		int fd, n;
		fd = l_open("/proc/self/environ", 0, 0);
		if (fd < 0) {
			return 0;
		}
		n = l_read((unsigned int)fd, buf, (unsigned int)sizeof(buf));
		if (n < 0) {
			return 0;
		}
		l_close(fd);
		end = buf + n;
	}
	namelen = strlen(name);
	for (p = buf; p < end;) {
		if (!strncmp(p, name, namelen)) {
			return p + namelen + 1; /* skip name and the = */
		}
		for (; *p && p < end; ++p); /* skip to next entry */
		++p;
	}
	return 0;
}

CFUNC const char* get_temp_path()
{
	const char* temp = getenv_("XDG_RUNTIME_DIR");
	temp = temp ? temp : getenv_("TMPDIR");
	temp = temp ? temp : getenv_("TMP");
	temp = temp ? temp : getenv_("TEMP");
	temp = temp ? temp : "/tmp";
	return temp;
}

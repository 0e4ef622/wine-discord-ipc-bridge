#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <strsafe.h>

#pragma region
struct sockaddr_un {
    unsigned short sun_family;               /* AF_UNIX */
    char           sun_path[108];            /* pathname */
};
/* struct sockaddr { */
/*     unsigned short sa_family;   // address family, AF_xxx */
/*     char           sa_data[14]; // 14 bytes of protocol address */
/* }; */
#define AF_UNIX     1
#define SOCK_STREAM     1
#define F_SETFL 4
#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_CREAT     00000100
#define O_APPEND    00002000
#define O_NONBLOCK  00004000
#define BUFSIZE 2048 // size of read/write buffers

#pragma endregion wine-specific header thingy
#pragma region
__declspec(naked) int l_close(int fd) {
    __asm__ (
            "push ebx\n\t"

            "mov eax, 0x06\n\t"
            "mov ebx, [esp + 4 + 4]\n\t"
            "int 0x80\n\t"

            "pop ebx\n\t"
            "ret"
            );
}
__declspec(naked) int l_socketcall(int call, void* args) {
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
__declspec(naked) int l_open(const char* filename, int flags, int mode) {
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
__declspec(naked) int l_write(unsigned int fd, const char* buf, unsigned int count) {
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
__declspec(naked) int l_read(unsigned int fd, char* buf, unsigned int count) {
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
#pragma endregion syscall wrappers
#pragma region
int l_socket(int domain, int type, int protocol) {
    void* args[3];
    args[0] = (void*)(int*)domain;
    args[1] = (void*)(int*)type;
    args[2] = (void*)(int*)protocol;
    return l_socketcall(1, args);
}
int l_connect(int sockfd, const struct sockaddr *addr, unsigned int addrlen) {
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
#pragma endregion socketcall wrappers

char* getenv_(char* name) // written by https://github.com/Francesco149
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

static const char* get_temp_path()
{
    const char* temp = getenv_("XDG_RUNTIME_DIR");
    temp = temp ? temp : getenv_("TMPDIR");
    temp = temp ? temp : getenv_("TMP");
    temp = temp ? temp : getenv_("TEMP");
    temp = temp ? temp : "/tmp";
    return temp;
}

static HANDLE hPipe = INVALID_HANDLE_VALUE;
static int sock_fd;
DWORD WINAPI winwrite_thread(LPVOID lpvParam);

int _tmain(VOID)
{
    BOOL   fConnected = FALSE;
    DWORD  dwThreadId = 0;
    HANDLE hThread = NULL;
    LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\discord-ipc-0");

    // The main loop creates an instance of the named pipe and
    // then waits for a client to connect to it. When the client
    // connects, a thread is created to handle communications
    // with that client, and this loop is free to wait for the
    // next client connect request. It is an infinite loop.

    _tprintf( TEXT("Pipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);
    hPipe = CreateNamedPipe(
            lpszPipename,             // pipe name
            PIPE_ACCESS_DUPLEX,       // read/write access
            PIPE_TYPE_BYTE |       // message type pipe
            PIPE_READMODE_BYTE |   // message-read mode
            PIPE_WAIT,                // blocking mode
            1, // max. instances
            BUFSIZE,                  // output buffer size
            BUFSIZE,                  // input buffer size
            0,                        // client time-out
            NULL);                    // default security attribute

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        _tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
        return -1;
    }

    // Wait for the client to connect; if it succeeds,
    // the function returns a nonzero value. If the function
    // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.

    fConnected = ConnectNamedPipe(hPipe, NULL) ?
        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (fConnected)
    {
        printf("Client connected\n");

        printf("Creating socket\n");

        if ((sock_fd = l_socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
            printf("Failed to create socket\n");
            return 1;
        }

        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX;

        const char *const temp_path = get_temp_path();

        char connected = 0;
        for (int pipeNum = 0; pipeNum < 10; ++pipeNum) {

            snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/discord-ipc-%d", temp_path, pipeNum);
            printf("Attempting to connect to %s\n", addr.sun_path);

            if (l_connect(sock_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
                printf("Failed to connect\n");
            } else {
                connected = 1;
                break;
            }
        }
        if (!connected) {
            for (int pipeNum = 0; pipeNum < 10; ++pipeNum) {

                snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/app/com.discordapp.Discord/discord-ipc-%d", temp_path, pipeNum);
                printf("Attempting to connect to %s\n", addr.sun_path);

                if (l_connect(sock_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
                    printf("Failed to connect\n");
                } else {
                    connected = 1;
                    break;
                }
            }
        }

        if (!connected) {
            printf("Could not connect to discord client\n");
            return 1;
        }


        printf("Connected successfully\n");

        hThread = CreateThread(
                NULL,              // no security attribute
                0,                 // default stack size
                winwrite_thread,    // thread proc
                (LPVOID) NULL,    // thread parameter
                0,                 // not suspended
                &dwThreadId);      // returns thread ID

        if (hThread == NULL)
        {
            _tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
            return 1;
        }


        for (;;) {
            char buf[BUFSIZE];
            DWORD bytes_read = 0;
            BOOL fSuccess = ReadFile(
                    hPipe,        // handle to pipe
                    buf,    // buffer to receive data
                    BUFSIZE, // size of buffer
                    &bytes_read, // number of bytes read
                    NULL);        // not overlapped I/O
            if (!fSuccess) {
                if (GetLastError() == ERROR_BROKEN_PIPE) {
                    printf("winread EOF\n");
                    return 0;
                } else {
                    printf("Failed to read from pipe\n");
                    return 1;
                }
            }

            printf("%d bytes w->l\n", bytes_read);
            /* uncomment to dump the actual data being passed from the pipe to the socket */
            /* for(int i=0;i<bytes_read;i++)putchar(buf[i]); */
            /* printf("\n"); */

            int total_written = 0, written = 0;

            while (total_written < bytes_read) {
                written = l_write(sock_fd, buf + total_written, bytes_read - total_written);
                if (written < 0) {
                    printf("Failed to write to socket\n");
                    return 1;
                }
                total_written += written;
                written = 0;
            }
        }
    }
    else
        // The client could not connect, so close the pipe.
        CloseHandle(hPipe);

    return 0;
}

DWORD WINAPI winwrite_thread(LPVOID lpvParam) {

    for (;;) {
        char buf[BUFSIZE];
        int bytes_read = l_read(sock_fd, buf, BUFSIZE);
        if (bytes_read < 0) {
            printf("Failed to read from socket\n");
            l_close(sock_fd);
            return 1;
        } else if (bytes_read == 0) {
            printf("EOF\n");
            break;
        }

        printf("%d bytes l->w\n", bytes_read);
        /* uncomment to dump the actual data being passed from the socket to the pipe */
        /* for(int i=0;i<bytes_read;i++)putchar(buf[i]); */
        /* printf("\n"); */

        DWORD total_written = 0, cbWritten = 0;

        while (total_written < bytes_read) {
            BOOL fSuccess = WriteFile(
                    hPipe,        // handle to pipe
                    buf + total_written,     // buffer to write from
                    bytes_read - total_written, // number of bytes to write
                    &cbWritten,   // number of bytes written
                    NULL);        // not overlapped I/O
            if (!fSuccess) {
                printf("Failed to write to pipe\n");
                return 1;
            }
            total_written += cbWritten;
            cbWritten = 0;
        }
    }
    return 1;
}

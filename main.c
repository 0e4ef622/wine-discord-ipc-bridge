#define USE_WS_PREFIX
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <strsafe.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFSIZE 2048 // size of read/write buffers

static const char* get_temp_path()
{
    const char *temp = getenv("XDG_RUNTIME_DIR");
    temp = temp ? temp : getenv("TMPDIR");
    temp = temp ? temp : getenv("TMP");
    temp = temp ? temp : getenv("TEMP");
    temp = temp ? temp : "/tmp";
    return temp;
}

static HANDLE handle_pipe = INVALID_HANDLE_VALUE;
static int sock_fd;
DWORD WINAPI winwrite_thread(LPVOID lpvParam);

int main(VOID)
{
    BOOL   pipe_connected = FALSE;
    DWORD  thread_id = 0;
    HANDLE thread_handle = NULL;
    LPCTSTR pipename = TEXT("\\\\.\\pipe\\discord-ipc-0");

    // Create the named pipe.

    printf("Pipe Server: Main thread awaiting client connection on %s\n", pipename);
    handle_pipe = CreateNamedPipe(
            pipename,            // pipe name
            PIPE_ACCESS_DUPLEX,  // read/write access
            PIPE_TYPE_BYTE |     // message type pipe
            PIPE_READMODE_BYTE | // message-read mode
            PIPE_WAIT,           // blocking mode
            1,                   // max. instances
            BUFSIZE,             // output buffer size
            BUFSIZE,             // input buffer size
            0,                   // client time-out
            NULL);               // default security attribute

    if (handle_pipe == INVALID_HANDLE_VALUE) {
        printf("CreateNamedPipe failed, GLE=%d.\n", GetLastError());
        return -1;
    }

    // Wait for the client to connect; if it succeeds,
    // the function returns a nonzero value. If the function
    // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.

    pipe_connected = ConnectNamedPipe(handle_pipe, NULL) ?
        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (pipe_connected) {

        printf("Client connected\n");

        printf("Creating socket\n");

        if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
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

            if (connect(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                perror("Failed to connect");
            } else {
                connected = 1;
                break;
            }
        }

        if (!connected) {
            printf("Could not connect to discord client\n");
            return 1;
        }


        printf("Connected successfully\n");

        thread_handle = CreateThread(
                NULL,            // no security attribute
                0,               // default stack size
                winwrite_thread, // thread proc
                (LPVOID) NULL,   // thread parameter
                0,               // not suspended
                &thread_id);     // returns thread ID

        if (thread_handle == NULL) {
            printf("CreateThread failed, GLE=%d.\n", GetLastError());
            return 1;
        }


        for (;;) {

            char buf[BUFSIZE];
            DWORD bytes_read = 0;

            BOOL fSuccess = ReadFile(
                    handle_pipe, // handle to pipe
                    buf,         // buffer to receive data
                    BUFSIZE,     // size of buffer
                    &bytes_read, // number of bytes read
                    NULL);       // not overlapped I/O

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
            /* for(int i = 0; i < bytes_read; ++i) putchar(buf[i]); */
            /* printf("\n"); */

            int total_written = 0;

            while (total_written < bytes_read) {
                int written = write(sock_fd, buf + total_written, bytes_read - total_written);
                if (written < 0) {
                    printf("Failed to write to socket\n");
                    return 1;
                }
                total_written += written;
                written = 0;
            }
        }
    } else {
        // The client could not connect, so close the pipe.
        CloseHandle(handle_pipe);
    }

    return 0;
}

// Runs in a separate thread. Handles data passing from the socket to the
// named pipe.
DWORD WINAPI winwrite_thread(LPVOID lpvParam) {

    for (;;) {

        char buf[BUFSIZE];
        int bytes_read = read(sock_fd, buf, BUFSIZE);

        if (bytes_read < 0) {
            printf("Failed to read from socket\n");
            close(sock_fd);
            return 1;
        } else if (bytes_read == 0) {
            printf("EOF\n");
            break;
        }

        printf("%d bytes l->w\n", bytes_read);

        /* uncomment to dump the actual data being passed from the socket to the pipe */
        /* for(int i = 0; i < bytes_read; ++i) putchar(buf[i]); */
        /* printf("\n"); */

        DWORD total_written = 0, written = 0;

        while (total_written < bytes_read) {

            BOOL success = WriteFile(
                    handle_pipe,                // handle to pipe
                    buf + total_written,        // buffer to write from
                    bytes_read - total_written, // number of bytes to write
                    &written,                   // number of bytes written
                    NULL);                      // not overlapped I/O

            if (!success) {
                printf("Failed to write to pipe\n");
                return 1;
            }

            total_written += written;
            written = 0;
        }
    }
    return 1;
}

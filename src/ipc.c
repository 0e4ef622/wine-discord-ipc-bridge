#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "syscalls.h"
#include "server.h"
#define INFO(FMTSTR, ...) \
	if(bStandalone) \
		_tprintf(TEXT(FMTSTR), ## __VA_ARGS__)

static BOOL bStandalone = FALSE;
static HANDLE hPipe = INVALID_HANDLE_VALUE;
static int sock_fd;
DWORD WINAPI winwrite_thread(LPVOID lpvParam);

int iServerLoop(BOOL bStandaloneArg)
{
	BOOL   fConnected = FALSE;
	DWORD  dwThreadId = 0;
	HANDLE hThread = NULL;
	LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\discord-ipc-0");
	bStandalone = bStandaloneArg;

	// The main loop creates an instance of the named pipe and
	// then waits for a client to connect to it. When the client
	// connects, a thread is created to handle communications
	// with that client, and this loop is free to wait for the
	// next client connect request. It is an infinite loop.

	INFO("Pipe Server: Main thread awaiting client connection on %s\n", lpszPipename);
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
		INFO("CreateNamedPipe failed, GLE=%d.\n", GetLastError());
		return -1;
	}

	// Wait for the client to connect; if it succeeds,
	// the function returns a nonzero value. If the function
	// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.

	fConnected = ConnectNamedPipe(hPipe, NULL) ?
	             TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

	if (fConnected)
	{
		INFO("Client connected\n");

		INFO("Creating socket\n");

		if ((sock_fd = l_socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		{
			INFO("Failed to create socket\n");
			return 1;
		}

		struct sockaddr_un addr;
		addr.sun_family = AF_UNIX;

		const char *const temp_path = get_temp_path();

		char *paths[] =
		{
			"%s/discord-ipc-%d",
			"%s/app/com.discordapp.Discord/discord-ipc-%d",
			"%s/snap.discord-canary/discord-ipc-%d",
			"%s/snap.discord/discord-ipc-%d"
		};

		char connected = 0;
		for (int p = 0; p < sizeof(paths) / sizeof(paths[0]); p++)
		{
			for (int pipeNum = 0; pipeNum < 10; ++pipeNum)
			{

				StringCchPrintf(addr.sun_path, sizeof(addr.sun_path), paths[p], temp_path, pipeNum);
				INFO("Attempting to connect to %s\n", addr.sun_path);

				if (l_connect(sock_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
				{
					INFO("Failed to connect\n");
				}
				else
				{
					connected = 1;
					goto breakout;
				}
			}
		}
breakout:
		;

		if (!connected)
		{
			INFO("Could not connect to discord client\n");
			return 1;
		}


		INFO("Connected successfully\n");

		hThread = CreateThread(
		              NULL,              // no security attribute
		              0,                 // default stack size
		              winwrite_thread,    // thread proc
		              (LPVOID) NULL,    // thread parameter
		              0,                 // not suspended
		              &dwThreadId);      // returns thread ID

		if (hThread == NULL)
		{
			INFO("CreateThread failed, GLE=%d.\n", GetLastError());
			return 1;
		}


		while(bStandalone || bSvcRunning())
		{
			char buf[BUFSIZE];
			DWORD bytes_read = 0;
			BOOL fSuccess = ReadFile(
			                    hPipe,        // handle to pipe
			                    buf,    // buffer to receive data
			                    BUFSIZE, // size of buffer
			                    &bytes_read, // number of bytes read
			                    NULL);        // not overlapped I/O
			if (!fSuccess)
			{
				if (GetLastError() == ERROR_BROKEN_PIPE)
				{
					INFO("winread EOF\n");
					return 0;
				}
				else
				{
					INFO("Failed to read from pipe\n");
					return 1;
				}
			}

			INFO("%d bytes w->l\n", bytes_read);
			/* uncomment to dump the actual data being passed from the pipe to the socket */
			/* for(int i=0;i<bytes_read;i++)putchar(buf[i]); */
			/* INFO("\n"); */

			int total_written = 0, written = 0;

			while (total_written < bytes_read)
			{
				written = l_write(sock_fd, buf + total_written, bytes_read - total_written);
				if (written < 0)
				{
					INFO("Failed to write to socket\n");
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

DWORD WINAPI winwrite_thread(LPVOID lpvParam)
{

	for (;;)
	{
		char buf[BUFSIZE];
		int bytes_read = l_read(sock_fd, buf, BUFSIZE);
		if (bytes_read < 0)
		{
			INFO("Failed to read from socket\n");
			l_close(sock_fd);
			return 1;
		}
		else if (bytes_read == 0)
		{
			INFO("EOF\n");
			break;
		}

		INFO("%d bytes l->w\n", bytes_read);
		/* uncomment to dump the actual data being passed from the socket to the pipe */
		/* for(int i=0;i<bytes_read;i++)putchar(buf[i]); */
		/* INFO("\n"); */

		DWORD total_written = 0, cbWritten = 0;

		while (total_written < bytes_read)
		{
			BOOL fSuccess = WriteFile(
			                    hPipe,        // handle to pipe
			                    buf + total_written,     // buffer to write from
			                    bytes_read - total_written, // number of bytes to write
			                    &cbWritten,   // number of bytes written
			                    NULL);        // not overlapped I/O
			if (!fSuccess)
			{
				INFO("Failed to write to pipe\n");
				return 1;
			}
			total_written += cbWritten;
			cbWritten = 0;
		}
	}
	return 1;
}

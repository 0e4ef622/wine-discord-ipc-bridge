#ifndef __SERVICE_H__
#define __SERVICE_H__
#include <windef.h>

#define SVCNAME TEXT("DiscordIPCBridge")
#define NOT_IMPLEMENTED(X) _tprintf(TEXT("Not Implemented.\n")); return X

VOID WINAPI
	vSvcMain(DWORD, LPTSTR*); // SvcMain
VOID
	vReportError(LPTSTR);

#endif /* __SERVICE_H__ */

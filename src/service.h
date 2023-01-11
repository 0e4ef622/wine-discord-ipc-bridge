#ifndef __SERVICE_H__
#define __SERVICE_H__
#include <windef.h>

#define SVCNAME TEXT("DiscordIPCBridge")

VOID WINAPI
	vSvcMain(DWORD, LPTSTR*); // Service Entry Point
VOID
	vReportError(LPTSTR); // Service Error Reporter

#endif /* __SERVICE_H__ */

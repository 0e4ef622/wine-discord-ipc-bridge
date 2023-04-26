#include <windows.h>
#include <tchar.h>

#include "service.h"
#include "server.h"
#define PRIVATE static // private functions

PRIVATE SERVICE_STATUS          gSvcStatus;
PRIVATE SERVICE_STATUS_HANDLE   gSvcStatusHandle;
PRIVATE HANDLE                  ghSvcStopEvent = NULL;

PRIVATE VOID vReportState(DWORD, DWORD, DWORD);
PRIVATE VOID WINAPI vHandleEvent(DWORD);

// decls
BOOL bSvcRunning()
{
	return (
	           ghSvcStopEvent == NULL
	           || WaitForSingleObjectEx(ghSvcStopEvent, 0, TRUE) == WAIT_TIMEOUT
	       );
}

VOID WINAPI vSvcMain(DWORD dw, LPTSTR * str)
{
	// Get Service Status Handle
	gSvcStatusHandle = RegisterServiceCtrlHandler(
	                       SVCNAME,
	                       vHandleEvent);

	if( !gSvcStatusHandle )
		return vReportError(TEXT("RegisterServiceCtrlHandler"));

	// These SERVICE_STATUS members remain as set here
	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwServiceSpecificExitCode = 0;
	vReportState(SERVICE_START_PENDING, NO_ERROR, 3000);

	ghSvcStopEvent = CreateEvent(
	                     NULL,    // default security attributes
	                     TRUE,    // manual reset event
	                     FALSE,   // not signaled
	                     NULL);   // no name

	if ( ghSvcStopEvent == NULL)
	{
		vReportState( SERVICE_STOPPED, GetLastError(), 0 );
		return;
	}

	vReportState( SERVICE_RUNNING, NO_ERROR, 0 );
	while(bSvcRunning())
	{
		iServerMain(FALSE);
	}

	vReportState( SERVICE_STOPPED, NO_ERROR, 0 );
}

PRIVATE VOID vReportState(DWORD dwCState, DWORD dwW32ECode, DWORD dwWHint)
{
	static DWORD dwCheckPoint = 1;
	gSvcStatus.dwCurrentState = dwCState;
	gSvcStatus.dwWin32ExitCode = dwW32ECode;
	gSvcStatus.dwWaitHint = dwWHint;

	if (dwCState == SERVICE_START_PENDING)
		gSvcStatus.dwControlsAccepted = 0;
	else
		gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if ( (dwCState == SERVICE_RUNNING) ||
	        (dwCState == SERVICE_STOPPED) )
		gSvcStatus.dwCheckPoint = 0;
	else gSvcStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

PRIVATE VOID WINAPI vHandleEvent(DWORD dwCtrl)
{
	switch(dwCtrl)
	{
	case SERVICE_CONTROL_STOP:
		vReportState(SERVICE_STOP_PENDING, NO_ERROR, 0);
		SetEvent(ghSvcStopEvent);
		vReportState(gSvcStatus.dwCurrentState, NO_ERROR, 0);
		return;
	default:
		break;
	}
}

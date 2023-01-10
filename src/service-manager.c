#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "server.h"
#include "service-manager.h"
#include "service.h"
#include "error.h"

int iAddService()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	TCHAR szUnquotedPath[MAX_PATH];
	if( !GetModuleFileName( NULL, szUnquotedPath, MAX_PATH ) ) {
		_tprintf(TEXT("Cannot install service (%d)\n"), GetLastError());
		return 1;
	}

	TCHAR szPath[MAX_PATH];
	StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\" service"), szUnquotedPath);

	// Open SCM database handle.
	schSCManager = OpenSCManager(
	                   NULL,                    // local computer
	                   NULL,                    // ServicesActive database
	                   SC_MANAGER_ALL_ACCESS);  // full access rights
	if (NULL == schSCManager) {
		_tprintf(TEXT("OpenSCManager failed (%d)\n"), GetLastError());
		return 1;
	}

	// Register service
	schService = CreateService(
	                 schSCManager,              // SCM database
	                 SVCNAME,                   // name of service
	                 SVCNAME,                   // service name to display
	                 SERVICE_ALL_ACCESS,        // desired access
	                 SERVICE_WIN32_OWN_PROCESS, // service type
	                 SERVICE_AUTO_START,        // start type
	                 SERVICE_ERROR_IGNORE,      // error control type
	                 szPath,                    // path to service's binary
	                 NULL,                      // no load ordering group
	                 NULL,                      // no tag identifier
	                 NULL,                      // no dependencies
	                 NULL,                      // LocalSystem account
	                 NULL);                     // no password

	if (schService == NULL) {
		_tprintf(TEXT("CreateService failed (%d)\n"), GetLastError());
		CloseServiceHandle(schSCManager);
		return 1;
	} else _tprintf(TEXT("Service installed successfully\n"));

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return 0;
}

int iDelService(void)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS ssStatus;

	// Open SCM database handle.
	schSCManager = OpenSCManager(
	                   NULL,                    // local computer
	                   NULL,                    // ServicesActive database
	                   SC_MANAGER_ALL_ACCESS);  // full access rights
	if (NULL == schSCManager) {
		_tprintf(TEXT("OpenSCManager failed (%d)\n"), GetLastError());
		return 1;
	}

	// Open service handle.
	schService = OpenService(
	                 schSCManager,       // SCM database
	                 SVCNAME,            // name of service
	                 DELETE);            // need delete access
	if (schService == NULL) {
		_tprintf(TEXT("OpenService failed (%d)\n"), GetLastError());
		CloseServiceHandle(schSCManager);
		return 1;
	}

	// Mark service as deleted
	if (! DeleteService(schService) ) {
		_tprintf(TEXT("DeleteService failed (%d)\n"), GetLastError());
	} else _tprintf(TEXT("Service deleted successfully\n"));

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return 0;
}

int iRunService()
{
	SERVICE_TABLE_ENTRY DispatchTable[] = {
		{ SVCNAME, (LPSERVICE_MAIN_FUNCTION) vSvcMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher( DispatchTable )) return 0;

	vReportError(TEXT("StartServiceCtrlDispatcher"));
	return 1;
}

VOID vReportError(LPTSTR szFunction)
{
	HANDLE  hEvSrc;
	LPCTSTR lpszStrs[2];
	TCHAR   tcBuf[80];

	hEvSrc = RegisterEventSource(NULL, SVCNAME);

	if(hEvSrc) {
		StringCchPrintf(tcBuf, 80, TEXT("%s failed with %d"), szFunction, GetLastError());
		lpszStrs[0] = SVCNAME;
		lpszStrs[1] = tcBuf;

		ReportEvent(hEvSrc,              // event log handle
		            EVENTLOG_ERROR_TYPE, // event type
		            0,                   // event category
		            SVC_ERROR,           // event identifier
		            NULL,                // no security identifier
		            2,                   // size of lpszStrings array
		            0,                   // no binary data
		            lpszStrs,            // array of strings
		            NULL);
	}
}

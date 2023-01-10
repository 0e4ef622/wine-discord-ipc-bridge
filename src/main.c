#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include "server.h"
#include "service-manager.h"
#define ARGV1(X) (argc > 1 && lstrcmpi( argv[1], TEXT(X)) == 0)
#define basename PathFindFileName(argv[0])

int _tmain(int argc, TCHAR *argv[]) {
	if(ARGV1("install"))		return iAddService();
	else if(ARGV1("remove"))	return iDelService();
	else if(ARGV1("service"))	return iRunService();
	else if(argc > 1)
		return (_tprintf(
				TEXT("Run Standalone : %s\n"
					"Install Service: %s install\n"
					"Remove Service : %s remove\n"),
				basename, basename, basename) * 0);
	return iServerLoop(TRUE);
}

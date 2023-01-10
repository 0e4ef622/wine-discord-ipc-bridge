#include <windows.h>
#include <tchar.h>
#include "server.h"
#include "service-manager.h"

#define NOT_IMPLEMENTED(X) _tprintf(TEXT("Not Implemented.\n")); return X

int iAddService()
{
	NOT_IMPLEMENTED(1);
}

int iDelService(void)
{
	NOT_IMPLEMENTED(1);
}

int iRunService()
{
	NOT_IMPLEMENTED(1);
}

BOOL bListening()
{
	NOT_IMPLEMENTED(FALSE);
}

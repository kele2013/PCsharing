#ifndef _SCREENSHOT_
#define _SCREENSHOT_
#include "stdafx.h"
#include <Windows.h>

#include "winsock.h"

#include "functionAPI.h"

DWORD DoSendScreenBuffer(SOCKET s);

DWORD DoRecvScreenBuffer(SOCKET s,DWORD ScreenSize,BYTE *pImageBuffer);


#endif
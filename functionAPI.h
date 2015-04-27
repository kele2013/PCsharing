#ifndef _FUNCTION_API_
#define _FUNCTION_API_

#include <windows.h>

typedef DWORD (_stdcall* pfn)();
typedef DWORD (_stdcall* pfn_1)(void*);
typedef DWORD (_stdcall* pfn_2)(void*,void*);
typedef DWORD (_stdcall* pfn_3)(void*,void*,void*);
typedef DWORD (_stdcall* pfn_4)(void*,void*,void*,void*);
typedef DWORD (_stdcall* pfn_5)(void*,void*,void*,void*,void*);
typedef DWORD (_stdcall* pfn_6)(void*,void*,void*,void*,void*,void*);


DWORD fun0_GetScreenSize(DWORD* pulBufSize);
DWORD GetScreenBuffer(BYTE *image_buffer,DWORD bufferSize);
DWORD fun1(char* str);
DWORD fun2(char * str1, char* str2);
bool Dofunction(taskInfo  &task);
extern DWORD g_fun[MAX_FUN];

#endif
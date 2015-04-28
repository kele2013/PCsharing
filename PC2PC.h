#ifndef _PC2PC_
#define _PC2PC_
#include <windows.h>
#define SYN_CALL 0x10000000
#define ASYN_CALL 0x00000000
#define PARAM_NUM 0x000000FF

#define MAX_PARAM  7
#define NAME_LEN  100
#define MAX_FUN  20
#define BUFFER_BLOCK_SIZE 1024

typedef struct ParamType
{
	char *buffer;
	DWORD bufferSize;
}ParamType,*PParamType;

typedef struct taskHeader
{
	DWORD type;
	DWORD lenOfTask;
	DWORD checkSum;
}taskHeader;

typedef struct taskInfo
{
	taskHeader th;
	BOOL isUsed;
	BOOL isSync;
	char  param[MAX_PATH];
	DWORD paramOffset[MAX_PARAM];
	DWORD  numOfParam;
	DWORD  ret;
	DWORD  index;
	//pfnTaskCallback callback;
}taskInfo,*ptaskInfo;

typedef struct ScreenInfo
{
	taskHeader th;
	DWORD  bufferSize;
}ScreenInfo,*pScreenInfo;

typedef struct ScreenACK
{
	taskHeader th;
	char*  pImageBuffer;
	DWORD  bufferSize;
}ScreenACK,*pScreenACK;

DWORD getParamtLength(void** Param,DWORD PC);


#endif
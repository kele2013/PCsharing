// PC2PC.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>

#include "winsock.h"
#include "PC2PC.h"
#include "functionAPI.h"

#pragma comment(lib,"ws2_32.lib")

#define PORT 7788
#define REMOTE_IP "192.168.1.108"

fd_set  g_fdclientSock;
SOCKET listenSock;

taskInfo  task;
taskInfo  recvTask;


bool initSocket()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested =MAKEWORD( 1, 1 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we couldn't find a useable */
		/* winsock.dll. */
		return false;
	}

	if ( LOBYTE( wsaData.wVersion ) != 1 ||
		HIBYTE( wsaData.wVersion ) != 1 ) {
			/* Tell the user that we couldn't find a useable */
			/* winsock.dll. */
			WSACleanup( );
			return false;
	}
	return true;
}

bool initSever()
{
	bool ret = false;
	do 
	{

		listenSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(listenSock == (SOCKET)(-1))
		{
			printf("[InitServer]--socket fail.\n");
			ret = false;
			break;
		}
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(PORT);
		sin.sin_addr.S_un.S_addr = INADDR_ANY;

		int nRet = bind(listenSock,(sockaddr*)&sin,(int)(sizeof(sin)));
		if(nRet == SOCKET_ERROR)
		{
			DWORD errCode = GetLastError();
			printf("[InitServer]--bind fail.\n");
			ret = false;
		}
		listen(listenSock,5);
		int clientNum = 0;
		sockaddr_in clientAddr;
		int nameLen = sizeof(clientAddr);

		while(clientNum < FD_SETSIZE)
		{
			SOCKET  clientsock = accept(listenSock,(sockaddr*)&clientAddr,&nameLen);
			if(clientsock == (SOCKET)-1)
			{
				printf("[InitServer]--accpet fail. err=%d\n",GetLastError());
				ret = false;
				break;
			}
			FD_SET(clientsock,&g_fdclientSock);
			clientNum++;

			printf("accept from client %d,IP =%s,port=%d\n",clientNum,inet_ntoa(clientAddr.sin_addr),clientAddr.sin_port);
		}
		ret = true;
	}while(false);
	printf("[InitServer]--leave\n");
	return ret;
}




void printRetBuffer(taskInfo  &task)
{
	for(int i = 0; i<task.numOfParam; i++)
	{
		PParamType  type = (PParamType)(task.param + i*sizeof(PParamType));
		if(type->bufferSize &0xFF00)
			printf("[CallRomoteFunc]--retbuffer:%s",type->buffer);

	}
}

DWORD _stdcall ServerThread(PVOID pM) 
{
	bool flag = initSever();
	if(!flag)
		return -1;

	fd_set fdRead;
	FD_ZERO(&fdRead);
	int nRet = 0;
	char* recvBuffer =(char*)malloc(sizeof(char)*1024);
	if(!recvBuffer)
	{
		return -1;
	}
	memset(recvBuffer,0,sizeof(char)*1024);
	while (true)
	{
		fdRead = g_fdclientSock;
		nRet = select(0,&fdRead,NULL,NULL,NULL);
		if(nRet!= SOCKET_ERROR)
		{
			for(int i =0; i < g_fdclientSock.fd_count;i++)
			{
				if(FD_ISSET(g_fdclientSock.fd_array[i],&fdRead))
				{
					memset(recvBuffer,0,sizeof(char)*1024);
					nRet = recv(g_fdclientSock.fd_array[i],(char*)&recvTask,sizeof(taskInfo),0);
					if (nRet == SOCKET_ERROR)
					{
						closesocket(g_fdclientSock.fd_array[i]);
						FD_CLR(g_fdclientSock.fd_array[i],&g_fdclientSock);
					}
					else
					{
						printf("recv from client %s\n",recvBuffer);//
						Dofunction(recvTask);
						if(recvTask.ret)
							send(g_fdclientSock.fd_array[i],(char*)&recvTask,sizeof(taskInfo),0);
						printf("server---after handle:\n");
						printRetBuffer(recvTask);

					}
				}

			}
		}
	}

	if(recvBuffer!=NULL)
	{
		free(recvBuffer);
	}
	return 0;

}




DWORD getParamtLength(void** Param,DWORD PC)
{
	DWORD ret=0;
	for (int i=0; i<PC; i++)
	{
		PParamType type=(PParamType)((char*)Param+sizeof(ParamType)*i);
		ret+=(USHORT) type->bufferSize;
	}

	return ret;
}

BOOL CopyParam2buffer(void** Paramer, ptaskInfo taskinfo)
{
	DWORD  PC = taskinfo->numOfParam;
	if(getParamtLength(Paramer,PC) >= MAX_PATH)
	{
		printf("getParamtLength>MAX_PATH\n");
		return FALSE;
	}
	for(int i = 0; i < PC; i++)
	{
		DWORD copyOffSet = 0;
		PParamType type = (PParamType) ((char*)Paramer + i*sizeof(ParamType));
		memcpy(taskinfo->param+copyOffSet,type->buffer,type->bufferSize);
		taskinfo->paramOffset[i]+=(USHORT) type->bufferSize;
	}
	return TRUE;

}

//得到参数信息
ParamType GetParaType(char* buffer, ULONG bufSize)
{
	ParamType type;
	type.buffer=buffer;
	type.bufferSize=bufSize;
	return type;
}


bool CallRemoteFun(SOCKET s,DWORD flag,DWORD funindex,...)
{
	DWORD num = flag&PARAM_NUM;
	bool ret = false;
	if(num > MAX_PARAM)
	{
		printf("[CallRomoteFunc]--too many params\n");
		ret = false;
	}
	printf("[CallRomoteFunc]-enter\n");

	task.numOfParam = num;	
	bool isSync = (flag&SYN_CALL)>>7;
	if(isSync)
		task.isSync = TRUE;

	void** Param = (void**)(&funindex+1);
	if(!CopyParam2buffer(Param,&task))
	{
		printf("[CallRomoteFunc]--CopyParam2buffer fail\n");
		ret = false;
	}

	task.index = funindex;
	ret = true;

	send(s,(char*)&task,sizeof(taskInfo),0);
	if(task.isSync)
	{
		recv(s,(char*)&task,sizeof(taskInfo),0);
		printRetBuffer(task);
	}

	return ret;

}

DWORD _stdcall ClientThread(PVOID pM)
{
	struct sockaddr_in srv_addr;
	SOCKET s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	memset((void*)&srv_addr,0,sizeof(sockaddr_in));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.S_un.S_addr = inet_addr(REMOTE_IP);
	srv_addr.sin_port = htons(PORT);
	if(connect(s,(struct sockaddr*)&srv_addr,sizeof(struct sockaddr_in))< 0)
	{
		DWORD err =WSAGetLastError();
		printf("connect error ,err = %d\n",err);
		closesocket(listenSock);
		getchar();
	}
	printf("connect success from %s,port =%d",inet_ntoa(srv_addr.sin_addr),ntohs(srv_addr.sin_port));

	DWORD  flag = 0x10000000;
	DWORD ScreenSize;
	CallRemoteFun(s,flag,0,GetParaType((char*)&ScreenSize,4|0xFF00));
	flag = 0x10000001;


	if(ScreenSize)
	{
		BYTE *pImageBuffer = new BYTE[ScreenSize];
		DWORD  flag = 0x10000001;
		CallRemoteFun(s,flag,1,GetParaType((char*)pImageBuffer,ScreenSize));
	}

	CallRemoteFun(s,flag,1,GetParaType("hkc", 4));
	char copyName[4];
	flag = 0x10000002;
	CallRemoteFun(s,flag,2,GetParaType("zqh", 4),GetParaType(copyName,4|0xFF00));
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	initSocket();
	HANDLE  hHAND[2];

	hHAND[0] = CreateThread(NULL,0,ServerThread,NULL,0,NULL);
	hHAND[1] = CreateThread(NULL,0,ClientThread,NULL,0,NULL);

	WaitForMultipleObjects(2,hHAND,TRUE,500);
	CloseHandle(hHAND[0]);
	CloseHandle(hHAND[1]);
	getchar();

	return 0;
}


#include "stdafx.h"
#include <Windows.h>
#include "SCreenShot.h"
#include "PC2PC.h"

DWORD hash_fun1(DWORD size)
{
	return size%11;
}
DWORD hash_fun2(BYTE *pBuffer,DWORD bufferSize)
{
	DWORD i = 0,sum = 0;
	while(pBuffer && i < bufferSize)
	{
		sum+=*pBuffer++;
	}
	return sum;

}

DWORD DoRecvScreenBuffer(SOCKET s,DWORD ScreenSize,BYTE *pImageBuffer)
{

	DWORD ret = 1;
	do
	{
		taskHeader th;
		th.type = 201;
		th.lenOfTask = sizeof(taskHeader)+sizeof(DWORD);
		ScreenInfo si;
		si.th = th;
		th.checkSum = hash_fun1(ScreenSize);
	
		ret = send(s,(char*)&si,sizeof(si),0);
		if(ret < 0)
		{
			printf("DoRecvScreenBuffer--send type failed\n");
			ret = 0;
			break;
		}



		DWORD  lenOfScreenACK = sizeof(taskHeader)+ScreenSize+sizeof(DWORD);

		int recv_ret = 0,total_recv = 0;
		int i = 0;
		BYTE *recvBuffer = new BYTE[lenOfScreenACK];
		if(!recvBuffer)
		{
			printf("DoRecvScreenBuffer--new recv buffer failed\n");
			ret = 0;
			break;
		}
		while(total_recv < lenOfScreenACK-BUFFER_BLOCK_SIZE)
		{
			recv_ret =recv(s,(char*)recvBuffer,BUFFER_BLOCK_SIZE,0);
			if(recv_ret>0)
			{
				total_recv+=recv_ret;
				i++;
			}
		}
		if(total_recv+BUFFER_BLOCK_SIZE>lenOfScreenACK)
			total_recv+=recv(s,(char*)(recvBuffer+i*BUFFER_BLOCK_SIZE),lenOfScreenACK-total_recv,0);

		ScreenACK sk;
		sk = *(ScreenACK*)recvBuffer;
		if(!sk.pImageBuffer || (sk.th.checkSum != hash_fun2((BYTE*)sk.pImageBuffer,sk.bufferSize)))
		{
			printf("DoRecvScreenBuffer--recv buffer checksum failed\n");
			ret = false;
			break;
		}

		memcpy(pImageBuffer ,sk.pImageBuffer,sk.bufferSize);
		if(recvBuffer)
		{
			delete recvBuffer;
			recvBuffer = NULL;
		}


	}while(false);
	return ret;		

}

DWORD DoSendScreenBuffer(SOCKET s)
{
	BYTE *image_buffer = NULL;
	DWORD nRet = 0;
	do
	{
		ScreenInfo si;		
		nRet = recv(s,(char*)&si,sizeof(si),0);
		if(nRet < 0)
		{
			Sleep(1000);
			nRet = recv(s,(char*)&si,sizeof(si),0);
		}
		if(si.bufferSize)
			image_buffer = new BYTE[si.bufferSize];
		GetScreenBuffer(image_buffer,si.bufferSize);



		int ret_send = 0,total_send = 0;
		DWORD i = 0;
		while(total_send < si.bufferSize - BUFFER_BLOCK_SIZE)
		{							
			ret_send = send(s,(char*)(image_buffer+BUFFER_BLOCK_SIZE*i),BUFFER_BLOCK_SIZE,0);
			if(ret_send > 0)
			{
				total_send+=ret_send;
				i++;
			}
			else
			{
				printf("DoSendScreenCMD--send imagebuffer failed.\n");
				nRet = false;
				break;
			}

		}
		if(total_send + BUFFER_BLOCK_SIZE< si.bufferSize)
			total_send+= send(i,(char*)(image_buffer+BUFFER_BLOCK_SIZE*i),si.bufferSize-ret_send,0);
	}while(false);

	if(image_buffer)
		delete image_buffer;
	return nRet;

}
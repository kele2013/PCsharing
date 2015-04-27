#include "stdafx.h"
#include "PC2PC.h"

#include "windows.h"
#include "wingdi.h"
#include "functionAPI.h"


#define JPEG_QUALITY 50     //���Ĵ�С����jpg�������û�

extern "C" {
#include "jpeglib.h"
#include "jmorecfg.h"
#include "jconfig.h"
}

#pragma comment(lib,"jpeg.lib")

DWORD g_fun[MAX_FUN] = 
{
	(DWORD)fun0_GetScreenSize,
	(DWORD)fun2,

};

DWORD fun0_GetScreenSize(DWORD* pulBufSize)
{
	bool ret = true;
	//��ȡ����DC���ߴ�
	HDC hdc=GetDC(NULL);
	int nScrx=GetDeviceCaps(hdc,HORZRES);
	int nScry=GetDeviceCaps(hdc,VERTRES);
	//���������ڴ�DC
	HDC memdc=CreateCompatibleDC(hdc);
	//�õ�����λͼ
	HBITMAP hbitmap,holdbitmap;
	hbitmap=CreateCompatibleBitmap(hdc,nScrx,nScry);//�����������λͼ
	holdbitmap=(HBITMAP)SelectObject(memdc,hbitmap);//����λͼѡ���ڴ�DC
	BitBlt(memdc,0,0,nScrx,nScry,hdc,0,0,SRCCOPY);//�����渴�Ƶ��ڴ�DC
	hbitmap=(HBITMAP)SelectObject(memdc,holdbitmap);//��������λͼ
	//�ͷ���Դ
	DeleteDC(memdc);
	DeleteObject(holdbitmap);
	//��λͼ������λͼ�ṹ
	BITMAP bitmap;
	int nresult=GetObject(hbitmap,sizeof(BITMAP),&bitmap);
	if(nresult==0) ret = false;
	//Ϊ��Ļͼ�����ݷ���ռ�
	int nLineByte=(bitmap.bmWidth*bitmap.bmBitsPixel/8+3)/4*4;
	//image_buffer = new BYTE[bmp.bmWidthBytes * bmp.bmHeight];
	*pulBufSize=nLineByte*bitmap.bmHeight;
	return true;

}


/*===================================================================================
function:       jpegѹ��
input:          1:���ɵ��ļ���,2:bmp��ָ��,3:λͼ���,4:λͼ�߶�,5:��ɫ���
return:         int
description:    bmp�����ظ�ʽΪ(RGB)
===================================================================================*/
int savejpeg(char *filename, unsigned char *bits, int width, int height, int depth)
{


	//RGB˳�����
	for (int i=0, j=0; j < 1366*768*4; i+=3, j+=4)
	{
		*(bits+i)=*(bits+j+2);
		*(bits+i+1)=*(bits+j+1);
		*(bits+i+2)=*(bits+j);
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;                 /* target file */
	JSAMPROW row_pointer[1];        /* pointer to JSAMPLE row[s] */
	int     row_stride;             /* physical row width in image buffer */

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s/n", filename);
		return -1;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = width;      /* image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = 3;         /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;         /* colorspace of input image */

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, JPEG_QUALITY, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);

	row_stride = width * depth; /* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
		//�����������޸ģ�����jpg�ļ���ͼ���ǵ��ģ����Ը���һ�¶���˳��
		//����ԭ���룺row_pointer[0] = & bits[cinfo.next_scanline * row_stride];
		row_pointer[0] = & bits[(cinfo.image_height - cinfo.next_scanline - 1) * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
	return 0;
}


DWORD CaptureScreen(BYTE *image_buffer,DWORD *pulBufSize)
{
	bool flag = true;
	do
	{
		//��ȡ����DC���ߴ�
		HDC hdc=GetDC(NULL);
		int nScrx=GetDeviceCaps(hdc,HORZRES);
		int nScry=GetDeviceCaps(hdc,VERTRES);
		//���������ڴ�DC
		HDC memdc=CreateCompatibleDC(hdc);
		//�õ�����λͼ
		HBITMAP hbitmap,holdbitmap;
		hbitmap=CreateCompatibleBitmap(hdc,nScrx,nScry);//�����������λͼ
		holdbitmap=(HBITMAP)SelectObject(memdc,hbitmap);//����λͼѡ���ڴ�DC
		BitBlt(memdc,0,0,nScrx,nScry,hdc,0,0,SRCCOPY);//�����渴�Ƶ��ڴ�DC
		hbitmap=(HBITMAP)SelectObject(memdc,holdbitmap);//��������λͼ
		//�ͷ���Դ
		DeleteDC(memdc);
		DeleteObject(holdbitmap);
		//��λͼ������λͼ�ṹ
		BITMAP bitmap;
		int nresult=GetObject(hbitmap,sizeof(BITMAP),&bitmap);
		if(nresult==0)
		{
			flag = false;
			break;
		}
		//Ϊ��Ļͼ�����ݷ���ռ�
		int nLineByte=(bitmap.bmWidth*bitmap.bmBitsPixel/8+3)/4*4;
		//image_buffer = new BYTE[bmp.bmWidthBytes * bmp.bmHeight];

		//LPBYTE lpdata=new unsigned char[nLineByte*bitmap.bmHeight];
		//image_buffer=lpdata;
		*pulBufSize=nLineByte*bitmap.bmHeight;

		//if(!lpdata) goto ret;
		//���λͼ��Ϣͷ�ṹ
		BITMAPINFOHEADER *pbitmapInfoHeader=new BITMAPINFOHEADER;
		if(pbitmapInfoHeader==NULL)
		{
			flag = false;
			break;
		}
		pbitmapInfoHeader->biSize=sizeof(BITMAPINFOHEADER);
		pbitmapInfoHeader->biWidth=bitmap.bmWidth;
		pbitmapInfoHeader->biHeight=bitmap.bmHeight;
		pbitmapInfoHeader->biPlanes=1;
		pbitmapInfoHeader->biBitCount=bitmap.bmBitsPixel;
		pbitmapInfoHeader->biCompression=BI_RGB;
		pbitmapInfoHeader->biSizeImage=0;
		pbitmapInfoHeader->biXPelsPerMeter=0;
		pbitmapInfoHeader->biYPelsPerMeter=0;
		pbitmapInfoHeader->biClrImportant=0;
		pbitmapInfoHeader->biClrUsed=0;
		//����λͼ�ļ�ͷ
		BITMAPFILEHEADER bitmapFileHeader;
		bitmapFileHeader.bfType=0x4d42;
		bitmapFileHeader.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+nLineByte*bitmap.bmHeight;
		bitmapFileHeader.bfReserved1=0;
		bitmapFileHeader.bfReserved2=0;
		bitmapFileHeader.bfOffBits =sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER);

		//��ȡDIB
		nresult=GetDIBits(hdc,hbitmap,0,bitmap.bmHeight,image_buffer,(BITMAPINFO*)pbitmapInfoHeader,DIB_RGB_COLORS);
		ReleaseDC(NULL,hdc);
		DeleteObject(hbitmap);
		if(nresult==0) 
		{
			flag = false;
			break;
		}
	}while(false);
		return flag;
}

DWORD GetScreenBuffer(BYTE *image_buffer,DWORD pScreeSize)
{
	bool ret = false;
	char filename[] ="ok.jpg";	
	do{
		if(image_buffer)
		{
			DWORD bufferSize = 0;
			CaptureScreen(image_buffer,&bufferSize);
			if(pScreeSize!=bufferSize)
			{
				ret = false;
				break;
			}
			savejpeg(filename, image_buffer, 1366, 768, 3);
			ret = true;
		}
		else
			ret = false;
	}while(false);

	return ret;
}

/*
DWORD fun1(BYTE *image_buffer)
{
	MessageBoxA(NULL,str,str,MB_OK);
	return 1;
}
*/
DWORD fun2(char * str1, char* str2)
{
	int len=strlen(str1);
	strncpy(str2,str1,len);
	str2[len]=0;
	return 1;
}


bool getParamList(const taskInfo &task,void* p[])
{
	int numParm=task.numOfParam;
	if (numParm > MAX_PARAM)
	{
		return false;
	}
	for (int i=0; i<numParm; i++)
	{
		int offset=task.paramOffset[i];
		p[i]= (void*)&task.param[offset];
	}
	return true;
}

bool Dofunction(taskInfo  &task)
{
	int num = task.numOfParam;
	int index = task.index;

	DWORD funAddr=g_fun[index];

	void* P[MAX_PARAM]={0};
	getParamList(task,P);

	if (num > MAX_PARAM)
	{
		printf("[Dofunction]-- too many params");
		return false;
	}

	if (num==0)
	{
		task.ret = ((pfn)funAddr)();
	}
	else if (num==1)
	{
		task.ret = ((pfn_1)funAddr)(P[0]);
	}
	else if (num==2)
	{
		task.ret = ((pfn_2)funAddr)(P[0],P[1]);
	}
	else if (num==3)
	{
		task.ret = ((pfn_3)funAddr)(P[0],P[1],P[2]);
	}
	else if (num==4)
	{
		task.ret = ((pfn_4)funAddr)(P[0],P[1],P[2],P[3]);
	}
	else if (num==5)
	{
		task.ret = ((pfn_5)funAddr)(P[0],P[1],P[2],P[3],P[4]);
	}
	else if (num==6)
	{
		task.ret = ((pfn_6)funAddr)(P[0],P[1],P[2],P[3],P[4],P[5]);
	}
	else
	{
		return false;
	}

	printf("[Dofunction]-- leave");

	return true;

}


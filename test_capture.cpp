// test_capture.cpp : Defines the entry point for the console application.
//

// test_capture.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "wingdi.h"


#define JPEG_QUALITY 50     //它的大小决定jpg的质量好坏

extern "C" {
#include "jpeglib.h"
#include "jmorecfg.h"
#include "jconfig.h"
}

#pragma comment(lib,"jpeg.lib")

BYTE *image_buffer; //指向位图buffer的全局指针，window下像素格式: BGRA(4个字节)


/*===================================================================================
function:       jpeg压缩
input:          1:生成的文件名,2:bmp的指针,3:位图宽度,4:位图高度,5:颜色深度
return:         int
description:    bmp的像素格式为(RGB)
===================================================================================*/
int savejpeg(char *filename, unsigned char *bits, int width, int height, int depth)
{


	//RGB顺序调整
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
		//这里我做过修改，由于jpg文件的图像是倒的，所以改了一下读的顺序
		//这是原代码：row_pointer[0] = & bits[cinfo.next_scanline * row_stride];
		row_pointer[0] = & bits[(cinfo.image_height - cinfo.next_scanline - 1) * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
	return 0;
}


//截屏
void  CaptureScreen(DWORD* pulBufSize )
{

	//获取桌面DC及尺寸
	HDC hdc=GetDC(NULL);
	int nScrx=GetDeviceCaps(hdc,HORZRES);
	int nScry=GetDeviceCaps(hdc,VERTRES);
	//创建兼容内存DC
	HDC memdc=CreateCompatibleDC(hdc);
	//得到桌面位图
	HBITMAP hbitmap,holdbitmap;
	hbitmap=CreateCompatibleBitmap(hdc,nScrx,nScry);//创建桌面兼容位图
	holdbitmap=(HBITMAP)SelectObject(memdc,hbitmap);//将空位图选入内存DC
	BitBlt(memdc,0,0,nScrx,nScry,hdc,0,0,SRCCOPY);//将桌面复制到内存DC
	hbitmap=(HBITMAP)SelectObject(memdc,holdbitmap);//返回桌面位图
	//释放资源
	DeleteDC(memdc);
	DeleteObject(holdbitmap);
	//用位图句柄填充位图结构
	BITMAP bitmap;
	int nresult=GetObject(hbitmap,sizeof(BITMAP),&bitmap);
	if(nresult==0)goto ret;
	//为屏幕图像数据分配空间
	int nLineByte=(bitmap.bmWidth*bitmap.bmBitsPixel/8+3)/4*4;
	//image_buffer = new BYTE[bmp.bmWidthBytes * bmp.bmHeight];

	if(image_buffer)
	{
		delete []image_buffer;
		image_buffer=NULL;
	}

	LPBYTE lpdata=new unsigned char[nLineByte*bitmap.bmHeight];
	image_buffer=lpdata;
	*pulBufSize=nLineByte*bitmap.bmHeight;

	if(!lpdata) goto ret;
	//填充位图信息头结构
	BITMAPINFOHEADER *pbitmapInfoHeader=new BITMAPINFOHEADER;
	if(pbitmapInfoHeader==NULL) goto ret;
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
	//构造位图文件头
	BITMAPFILEHEADER bitmapFileHeader;
	bitmapFileHeader.bfType=0x4d42;
	bitmapFileHeader.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+nLineByte*bitmap.bmHeight;
	bitmapFileHeader.bfReserved1=0;
	bitmapFileHeader.bfReserved2=0;
	bitmapFileHeader.bfOffBits =sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER);

	//获取DIB
	nresult=GetDIBits(hdc,hbitmap,0,bitmap.bmHeight,lpdata,(BITMAPINFO*)pbitmapInfoHeader,DIB_RGB_COLORS);
	ReleaseDC(NULL,hdc);
	DeleteObject(hbitmap);
	if(nresult==0) 
		goto ret;
		
	
	ret:
	return;
}



int sendjpg2Server(SOCKET s)
{
	char filename[]="ok.jpg";
	
	DWORD dwFileSize=0;
	CaptureScreen(&dwFileSize);
	savejpeg(filename, image_buffer, 1366, 768, 3);
	Sleep(500);
	
	if(image_buffer)
	{
		delete []image_buffer;
		image_buffer=NULL;
	}
	return 0;
}



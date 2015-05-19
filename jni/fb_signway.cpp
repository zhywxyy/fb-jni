#include <stdio.h>  
#include <string.h>  
#include <errno.h>  
#include <fcntl.h> 
#include <android/log.h>  
#include <linux/fb.h>
#include <sys/mman.h>
#include "com_signway_fb_FbSignway.h"

#define LOG_TAG "signway-fb-jni"  
  
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)  
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)  
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__) 

const char* extend_screen_path = "/dev/graphics/fb4";

#define STANDARD_WIDTH   800
#define STANDARD_HEIGHT  480

enum
{
	PREPARE_FB = 0x8080,
	DISPLAY_FB,
	CLEAR_FB 
};

//14byte文件头  
typedef struct  
{  
    char cfType[2];//文件类型，"BM"(0x4D42)  
    long cfSize;//文件大小（字节）  
    long cfReserved;//保留，值为0  
    long cfoffBits;//数据区相对于文件头的偏移量（字节）  
}__attribute__((packed)) BITMAPFILEHEADER;  
  
//40byte信息头  
typedef struct  
{  
    char ciSize[4];//BITMAPFILEHEADER所占的字节数  
    long ciWidth;//宽度  
    long ciHeight;//高度  
    char ciPlanes[2];//目标设备的位平面数，值为1  
    int ciBitCount;//每个像素的位数  
    char ciCompress[4];//压缩说明  
    char ciSizeImage[4];//用字节表示的图像大小，该数据必须是4的倍数  
    char ciXPelsPerMeter[4];//目标设备的水平像素数/米  
    char ciYPelsPerMeter[4];//目标设备的垂直像素数/米  
    char ciClrUsed[4]; //位图使用调色板的颜色数  
    char ciClrImportant[4]; //指定重要的颜色数，当该域的值等于颜色数时（或者等于0时），表示所有颜色都一样重要  
}__attribute__((packed)) BITMAPINFOHEADER;  
  
/*typedef struct  
{  
    unsigned short blue;  
    unsigned short green;  
    unsigned short red;  
    unsigned short reserved;  
}__attribute__((packed)) PIXEL;//颜色模式RGB  */
  
BITMAPFILEHEADER FileHead;  
BITMAPINFOHEADER InfoHead;  
  
static char *fbp = 0;  
//static int  xres = 0;  
//static int  yres = 0;  
//static int  bits_per_pixel = 0; 
int *dst_buf = NULL; 
int dst_total_length = 0;
int tmplen = 0;

static int  fbfd = 0;
struct fb_var_screeninfo vinfo;  
struct fb_fix_screeninfo finfo;  
long int screensize = 0;  
//struct fb_bitfield red;  
//struct fb_bitfield green;  
//struct fb_bitfield blue; 

//int width, height;  

static void fb_update(struct fb_var_screeninfo *vi)   //将要渲染的图形缓冲区的内容绘制到设备显示屏来  
{    
    vi->yoffset = 1;    
    ioctl(fbfd, FBIOPUT_VSCREENINFO, vi);    
    vi->yoffset = 0;    
    ioctl(fbfd, FBIOPUT_VSCREENINFO, vi);    
}   

#if 0
static int cursor_bitmpa_format_convert(char *dst, char *src)
{  
    int i ,j ;  
    char *psrc = src ;  
    char *pdst = dst;  
    char *p = psrc;  
    int value = 0x00;  
      
    /* 由于bmp存储是从后面往前面，所以需要倒序进行转换 */  
    pdst += (width * height * 4);  
    for(i = 0; i < height; i++)
    {  
        p = psrc + (i + 1) * width * 3;  
        for(j = 0; j < width; j++)
        {  
            pdst -= 4;  
            p -= 3;  
            pdst[0] = p[0];  
            pdst[1] = p[1];  
            pdst[2] = p[2];  
            //pdst[3] = 0x00;  
  
            value = *((int*)pdst);  
            value = pdst[0];  
            if(value == 0x00)
            {  
                pdst[3] = 0x00;  
            }
            else
            {  
                pdst[3] = 0xff;  
            }  
        }  
    }  
  
    return 0;  
} 
#endif

static int show_bmp(jint* zBitmap1, jint* zBitmap2, jint* zBitmap3)
{	
	int *buf = dst_buf;
	int *fp1 = NULL, *fp2 = NULL, *fp3 = NULL;
	
	if(zBitmap1 == NULL || zBitmap2 == NULL || zBitmap3 == NULL)
	{
		LOGE("Bitmap object error\n");
		return -1;
	}	
			
	fp1 = (int*)zBitmap1;
	fp2 = (int*)zBitmap2;
	fp3 = (int*)zBitmap3;
    
    for(int i = 0; i < STANDARD_HEIGHT; i++)
    {
    	memcpy(buf, fp1, tmplen);
    	buf += STANDARD_WIDTH;
    	fp1 += STANDARD_WIDTH;
        
        memcpy(buf, fp2, tmplen);
    	buf += STANDARD_WIDTH;
    	fp2 += STANDARD_WIDTH;
        
        memcpy(buf, fp3, tmplen);
    	buf += STANDARD_WIDTH;
    	fp3 += STANDARD_WIDTH;
    }  
        
    memcpy(fbp, (char*)dst_buf, dst_total_length);     
      
    LOGI("show bmp\n");  
    
    return 0;
}

#if 0
int show_bmp(const char* path)  
{  
    FILE *fp;  
    int rc;  
    int line_x, line_y;  
    long int location = 0, BytesPerLine = 0;  
    char *bmp_buf = NULL;  
    //char *bmp_buf_dst = NULL;  
    char * buf = NULL;  
    int flen = 0;  
    int ret = -1;  
    int total_length = 0;  
    
    if(path == NULL)
    {
    	LOGE("path Error, return");
    	return -1;
   	}
   	
   	LOGI("path = %s", path);
  
    fp = fopen(path, "rb");  
    if (fp == NULL)  
    {  
    	LOGE("fopen file failed\n");
        return -1;  
    }  
    
    /* 求解文件长度 */
    //fseek(fp, 0, SEEK_SET);
    fseek(fp, 0, SEEK_END);
    flen = ftell(fp);
    
    bmp_buf = (char*)calloc(1, flen - 54);
    if(bmp_buf == NULL)
    {
    	LOGE("malloc bmp buffer error!\n");
    	return -1;
    }
    
    fseek(fp, 0, SEEK_SET);
  
    rc = fread(&FileHead, sizeof(BITMAPFILEHEADER), 1,  fp);  
    if(rc != 1)  
    {  
        LOGE("read header error!\n");  
        fclose(fp);  
        return -2;  
    }  
  
    //检测是否是bmp图像  
    if(memcmp(FileHead.cfType, "BM", 2) != 0)  
    {  
        LOGE("it's not a BMP file\n");  
        fclose(fp);  
        return -3;  
    }  
  
    rc = fread((char *)&InfoHead, sizeof(BITMAPINFOHEADER), 1, fp);  
    if(rc != 1)  
    {  
        LOGE("read infoheader error!\n");  
        fclose(fp);  
        return -4;  
    }  
    
    width = InfoHead.ciWidth;
    height = InfoHead.ciHeight;
    LOGI("FileHead.cfSize = %d byte\n", FileHead.cfSize);
    LOGI("flen = %d", flen);
    LOGI("width = %d, height = %d", width, height);
    total_length = width * height * 4;
    
    LOGI("total_length = %d", total_length);
  
    //跳转的数据区  
    fseek(fp, FileHead.cfoffBits, SEEK_SET);  
    LOGI("FileHead.cfoffBits = %d\n", FileHead.cfoffBits);
    LOGI("FileHead.ciBitCount = %d\n", InfoHead.ciBitCount);
      
    //每行字节数  
    buf = bmp_buf;  
    while ((ret = fread(buf, 1, total_length, fp)) >= 0) 
    {  
        if (ret == 0) 
        {  
            usleep(100);  
            continue;  
        }  
        LOGI("ret = %d", ret);  
        buf = ((char*) buf) + ret;  
        total_length = total_length - ret;  
        if(total_length == 0)
        	break;  
    }  
      
    total_length = width * height * 4;  
    LOGI("total_length = %d", total_length);  
    
    /*bmp_buf_dst = (char*)calloc(1, total_length);  
    if(bmp_buf_dst == NULL)
    {  
        LOGE("load > malloc bmp out of memory!");  
        return -1;  
    }  */
      
    //cursor_bitmpa_format_convert(bmp_buf_dst, bmp_buf);  
    //memcpy(fbp, bmp_buf_dst, total_length); 
    memcpy(fbp, bmp_buf, total_length); 
    
    //free(bmp_buf_dst);
    //bmp_buf_dst = NULL;
    free(bmp_buf); 
    bmp_buf = NULL;
      
    LOGI("show bmp\n");  
    return 0;  
}  
#endif

JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbOpen
  (JNIEnv *env, jobject obj, jstring device)
{
    const char *device_name = env->GetStringUTFChars(device, NULL);  
    if(device_name == NULL) {  
        LOGW("str is NULL!, set to be \"/dev/graphics/fb4\"\n");  
        device_name = extend_screen_path;  
    }  
   
    fbfd = open(device_name, O_RDWR);  
    if(fbfd < 0) {  
        LOGE("open %s failed..., errno = %d\n", device_name, errno);  
        return -1;
    }  
    
    LOGI("open %s success, fd = %d!\n", device_name, fbfd);
    
    if(ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))  
	{  
		LOGI("Error：reading fixed information.\n");  
		return -1;  
	}  
	
	if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))  
	{  
		LOGI("Error: reading variable information.\n");  
		return -1;  
	} 	 
	
	LOGI("R:%d,G:%d,B:%d\n", vinfo.red, vinfo.green, vinfo.blue);  
	
	LOGI("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);  
	//xres = vinfo.xres;  
	//yres = vinfo.yres;  
	//bits_per_pixel = vinfo.bits_per_pixel;  
	
	//计算屏幕的总大小（字节）  
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;  
	
	LOGI("screensize = %d byte\n",screensize); 
	
	//对象映射  
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);  
	if((int)fbp == -1)  
	{  
		LOGE("failed to map framebuffer device to memory.\n");  
		return -1;  
	}  
	
	memset(fbp, 0, screensize);
	
	LOGI("sizeof file header=%d\n", sizeof(BITMAPFILEHEADER));  
	
	dst_total_length = STANDARD_WIDTH * STANDARD_HEIGHT * 3 * 4;
	
	dst_buf = (int*)calloc(1, dst_total_length);
    if(dst_buf == NULL)
    {
    	LOGE("malloc buffer error!\n");
    	return -1;
    }
    
    tmplen = 4 * STANDARD_WIDTH;
    
    env->ReleaseStringUTFChars(device, device_name);  
    return fbfd;
}

JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbClose
  (JNIEnv *env, jobject obj)
{
	LOGI("fb close\n");
	
	//删除对象映射  
    munmap(fbp, screensize); 
    
    free(dst_buf); 
    dst_buf = NULL;
     
	close(fbfd);
	return 0;
}

JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbRead
  (JNIEnv *env, jobject obj, jbyteArray buf, jint len)
{
	return 0;
}

JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbWrite
  (JNIEnv *env, jobject obj, jbyteArray buf, jint len)
{
	return 0;
}

JNIEXPORT jint JNICALL Java_com_signway_fb_FbSignway_FbIoctl
  (JNIEnv *env, jobject obj, jint cmd, jintArray bp1, jintArray bp2, jintArray bp3)
{
	jint *Arr1 = NULL, *Arr2 = NULL, *Arr3 = NULL;
	int ret = -1;
	
	LOGI("IOCTL()->fbfd = %d, cmd = %d\n",	fbfd, cmd);  
		
	switch(cmd)
	{
		/*case PREPARE_FB:
		{    		 
    		 ret = merge_bmp(file, file_path1, file_path2, file_path3); 
		}
		break;*/
		/*case DISPLAY_ONE_FB:
		{
			
		}
		break;*/
		case DISPLAY_FB:
		{			
			int nArrLen = env->GetArrayLength(bp1);
			//LOGI("nArrLen = %d", nArrLen);
			
			Arr1 = (int*)env->GetIntArrayElements(bp1, 0);
			Arr2 = (int*)env->GetIntArrayElements(bp2, 0);
			Arr3 = (int*)env->GetIntArrayElements(bp3, 0);
			
			//显示图像  
			ret = show_bmp(Arr1, Arr2, Arr3);  
			//ret = show_bmp(file_path1);  
			if(ret >= 0)
				fb_update(&vinfo); 
			
			env->ReleaseIntArrayElements(bp1, Arr1, 0); 
			env->ReleaseIntArrayElements(bp2, Arr2, 0);
			env->ReleaseIntArrayElements(bp3, Arr3, 0);
		}
		break;
		case CLEAR_FB:
		{
			memset(fbp, 0, screensize);
			ret = 0;
		}
		break;
		default:
			LOGE("cmd is error\n");
			ret = -1;
	} 
    
    return ret;  
}
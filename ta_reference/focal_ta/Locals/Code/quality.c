#include <focal_common.h>


#define IMG_SIZE 110592
#define GRAD_THRESHOLD 150

extern unsigned short g_sensor_width;
extern unsigned short g_sensor_height;

#if 1
static long abs_new(long data_abs)
{
	long data_return;
	if (data_abs<0)
	{
		data_return = -1*data_abs;
		return data_return;
	}
	else
	{
		data_return = data_abs;
		return data_return;
	}
}

static long SqrtNew_long(long n) //鐢ㄨ凯浠ｆ硶寮�柟
{ 
    #define pre 1    
    long  val,last; 
    unsigned char i;

    if (n <= 0)
	{
		return 0;
	}
    if (n < 6)
    {
        if (n < 2)
        {
            return n;
        }
        return n/2;
    }   
    val = n;
    i = 0;
    while (val > 1)
    {
        val >>= 1;
        i++;
    }
    val <<= (i >> 1);
    val = (val + val + val) >> 1;
    do
    {
      last = val;
      val = ((val + n/val) >> 1);
    }while(abs_new(val-last) > pre);//绮惧害鎺у埗
    return val; 
} 
#endif 

//?锟斤拷?Y2?锟斤拷?锟斤拷???锟斤拷?锟斤拷???DD锟教★拷?锟斤拷


static void ftThreshold(unsigned char *pSrcImg, unsigned char *pDst, int nWidth, int nHeight)
{
	int nSize = nWidth*nHeight;
	int i;

	for (i = 0; i < nSize; i++)
	{
		if (*pSrcImg > GRAD_THRESHOLD){
			*pDst = 255;
		}else{
			*pDst = 0;
		}
		pSrcImg++;
		pDst++;
	}
}

static void ftDilation(unsigned char *pSrcImg, unsigned char *pDst, int nWidth, int nHeight, int winSize)
{
	int i,j,n;
	unsigned char *lpSrc;
	unsigned char *lpDst;
	unsigned char pixel;

	//锟斤拷1锟斤拷?????锟斤拷??锟斤拷锟斤拷??锟斤拷11?a????DD?锟斤拷?锟斤拷
	for (i=0; i<nHeight; i++)
	{
		for(j=winSize; j<nWidth-winSize; j++)
		{
			lpSrc = pSrcImg + i*nWidth + j;
			lpDst = pDst + i*nWidth + j;

			pixel = (unsigned char)*lpSrc;

			*lpDst = 0;

			for (n=0; n<2*winSize+1; n++)
			{
				pixel = (unsigned char)*(lpSrc + n -winSize);
				if (pixel == 255)
				{
					*lpDst = 255;
					break;
				}
			}
		}
	}
}

static void ftErosion(unsigned char *pSrcImg, unsigned char *pDst, int nWidth, int nHeight, int winSize)
{
	int i,j,n;
	unsigned char *lpSrc;
	unsigned char *lpDst;
	unsigned char pixel;

	//锟斤拷1锟斤拷?????锟斤拷??锟斤拷锟斤拷??锟斤拷11?a????DD?锟斤拷锟斤拷锟斤拷
	for (i=0; i<nHeight; i++)
	{
		for (j=winSize; j<nWidth-winSize; j++)
		{
			lpSrc = pSrcImg + i*nWidth + j;
			lpDst = pDst + i*nWidth + j;

			pixel = (unsigned char)*lpSrc;

			*lpDst = 255;

			for (n=0; n<2*winSize+1; n++)
			{
				pixel = (unsigned char)*(lpSrc + n - winSize);
				if (pixel == 0)
				{
					*lpDst = 0;
					break;
				}
			}
		}
	}
}

#if 0
static void ftOpen(unsigned char *pSrcImg, unsigned char *pDst, int nWidth, int nHeight, int winSize)
{
	ftErosion(pSrcImg, pDst, nWidth, nHeight, winSize);
	ftDilation(pSrcImg, pDst, nWidth, nHeight, winSize);
}

static void ftClose(unsigned char *pSrcImg, unsigned char *pDst, int nWidth, int nHeight, int winSize)
{
	ftDilation(pSrcImg, pDst, nWidth, nHeight, winSize);
	ftErosion(pSrcImg, pDst, nWidth, nHeight, winSize);
}
#endif

static void ftPreprocess(unsigned char *pSrcImg, unsigned char *pGradImg, long *pGxx, long *pGxy, \
	long *pGyy, int nWidth, int nHeight)
{
	unsigned char *pGradient, *pClose;

	//锟斤拷锟捷讹拷
	long gx, gy, e0, e1, e2, e3, e4, e5, e6, e7, e8;
	int i, j;
	unsigned char *pIndex;
	int Sobel_X[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	int Sobel_Y[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

	pGradient = (unsigned char *)TEE_Malloc(nWidth*nHeight*sizeof(unsigned char), 0);
	pClose = (unsigned char *)TEE_Malloc(nWidth*nHeight*sizeof(char), 0);
	
	for (i=1; i<nHeight-1; i++)
	{
		for (j=1; j<nWidth-1; j++)
		{			
			pIndex = pSrcImg + i*nWidth + j;
			e0 = *(pIndex -nWidth-1);
			e1 = *(pIndex -nWidth);
			e2 = *(pIndex -nWidth+1);
			e3 = *(pIndex -1);
			e4 = *pIndex;
			e5 = *(pIndex + 1);
			e6 = *(pIndex +nWidth-1);
			e7 = *(pIndex +nWidth);
			e8 = *(pIndex +nWidth+1);

			gx = Sobel_X[0]*e0 + Sobel_X[1]*e1 + Sobel_X[2]*e2 + Sobel_X[3]*e3 + Sobel_X[4]*e4 
				+ Sobel_X[5]*e5 + Sobel_X[6]*e6 + Sobel_X[7]*e7 + Sobel_X[8]*e8;
			gy = Sobel_Y[0]*e0 + Sobel_Y[1]*e1 + Sobel_Y[2]*e2 + Sobel_Y[3]*e3 + Sobel_Y[4]*e4 
				+ Sobel_Y[5]*e5 + Sobel_Y[6]*e6 + Sobel_Y[7]*e7 + Sobel_Y[8]*e8;

			*(pGradient + i*nWidth + j) = (unsigned char)(abs_new(gx)+abs_new(gy));	

			gx = (long)gx/4;
			gy = (long)gy/4;

			*(pGxx + i*nWidth + j) = gx * gx;
			*(pGxy + i*nWidth + j) = gx * gy;
			*(pGyy + i*nWidth + j) = gy * gy;
		}
	}
	
	//?D?锟斤拷?锟斤拷
	ftThreshold(pGradient, pGradient, nWidth, nHeight);

	//D?锟斤拷??锟斤拷??2锟斤拷
	ftDilation(pGradient, pClose, nWidth, nHeight, 1);
	ftErosion(pClose, pGradImg, nWidth, nHeight, 1);

	TEE_Free(pGradient);
	TEE_Free(pClose);
}

static int ftNonZero(unsigned char *pGradImg, int nWidth, int nHeight)
{
	int i, j;
	int cnt = 0;
	unsigned char *lpSrc;
	for (i=0; i<nHeight; i++)
	{
		for (j=0; j<nWidth; j++)
		{
			lpSrc = pGradImg + i*nWidth + j;
			if (*lpSrc != 0)
			{
				cnt++;
			}
		}
	}
	return cnt;
}

static float ftAreaScore(unsigned char *pGradImg, int nWidth, int nHeight)
{
	int cnt = 0;
	float areaScore;

	cnt = ftNonZero(pGradImg, nWidth, nHeight);
	//areaScore = 100. * cnt/ (0.75 * nWidth * nHeight);
	areaScore = 100*cnt/ (75 * nWidth * nHeight/100);
	areaScore = areaScore > 100 ? 100 : areaScore;

	return areaScore;
}


////////////// Final Version
static float ftOCLScore(long *pGxx, long *pGxy, long *pGyy, int nWidth, int nHeight)
{
	long oclScore, totalOCL;

	int size_window = 10;
	int i, j, m, l, offset;	
	long H11, H12, H21, H22;
	long lamda1, lamda2, var1, var2;
	int cnt_block;

	//锟街匡拷统锟斤拷OCL锟矫凤拷
	totalOCL = 0;
	cnt_block = 0;
	for (i=0; i<nHeight; i+=size_window)
	{
		for (j=0; j<nWidth; j+=size_window)
		{
			H11 = 0;
			H12 = 0;
			H21 = 0;
			H22 = 0;
			for (m=0; m<size_window; m++)
			{
				for (l=0; l<size_window; l++)
				{
					offset = (i+m>=nHeight?nHeight-1:i+m)*nWidth + (j+l>=nWidth?nWidth-1:j+l);
					H11 += (long)(*(pGxx + offset))/100;
					H12 += (long)(*(pGxy + offset))/100;
					H21 += (long)(*(pGxy + offset))/100;
					H22 += (long)(*(pGyy + offset))/100;
				}				
			}

			//锟斤拷2锟轿凤拷锟斤拷锟斤拷锟斤拷锟绞斤拷锟�锟阶撅拷锟斤拷H锟斤拷锟斤拷锟斤拷值
			var1 = H11+H22;
			var2 = SqrtNew_long((H11-H22)*(H11-H22) + 4*H12*H12);
			//var2 = sqrtl((H11-H22)*(H11-H22) + 4*H12*H12);
			lamda1 = var1 - var2;
			lamda2 = var1 + var2;

			//?T??oclScore锟斤拷?D??锟斤拷?a0锟斤拷?锟斤拷?锟戒ó?锟斤拷?a1
			if (lamda2<=0)
			{
				oclScore = 0;
			}else
			{
				oclScore = 100 - 100*lamda1/lamda2;
			}	

			totalOCL += oclScore;
			cnt_block++;
		}
	}

	oclScore = totalOCL/cnt_block;

	return oclScore;
}

//#define FP_SENSOR_WIDTH             88
//#define FP_SENSOR_HEIGH             88

//#define FP_SENSOR_SIZE              (FP_SENSOR_WIDTH*FP_SENSOR_HEIGH)
int GetImageScore(unsigned char *pSrcImg, int nWidth, int nHeight)
{
	long areaScore, oclScore;
	int fpSensorSize = g_sensor_width * g_sensor_height;
	//int isGoodImg;
	long *pGxx;
	long *pGxy;
	long *pGyy;

	//锟斤拷锟斤拷锟捷讹拷
	unsigned char *	pGradImg = (unsigned char *)TEE_Malloc(nWidth*nHeight*sizeof(char), 0);
	memset(pGradImg, 0, nWidth*nHeight*sizeof(char));

	pGxx = (long *)TEE_Malloc(nWidth*nHeight*sizeof(long), 0);
	pGxy = (long *)TEE_Malloc(nWidth*nHeight*sizeof(long), 0);
	pGyy = (long *)TEE_Malloc(nWidth*nHeight*sizeof(long), 0);
	memset(pGxx, 0, nWidth*nHeight*sizeof(long));
	memset(pGxy, 0, nWidth*nHeight*sizeof(long));
	memset(pGyy, 0, nWidth*nHeight*sizeof(long));

	ftPreprocess(pSrcImg, pGradImg, pGxx, pGxy, pGyy, nWidth, nHeight);

	//锟叫讹拷图锟斤拷锟斤拷锟斤拷
	areaScore = ftAreaScore(pGradImg, nWidth, nHeight);

	//areaThres = 50
	if (areaScore < fpSensorSize/500)
	{
		//isGoodImg = 0;
		return areaScore;
	}

	oclScore = ftOCLScore(pGxx, pGxy, pGyy, nWidth, nHeight);

	TEE_Free(pGradImg);
	TEE_Free(pGxx);
	TEE_Free(pGxy);
	TEE_Free(pGyy);

	return oclScore;
}

#ifndef __QUALITY_H__
#define __QUALITY_H__

//long ftAreaScore(unsigned char *pSrcImg, int nWidth, int nHeight);

//long ftOCLScore(unsigned char *pSrc, int nWidth, int nHeight);

//pSrcImg: 源图像， nWidth: 源图像的宽度， nHeight：源图像的高度, oclThres: 预设的OCL阈值(70)
//函数返回指纹是否为质量好的图像： 0为差图，1为好图
int GetImageScore(unsigned char *pSrcImg, int nWidth, int nHeight);

#endif/*__QUALITY_H__*/
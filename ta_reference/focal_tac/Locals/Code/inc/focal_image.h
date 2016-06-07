/**
@file focal_image.h
@version 1.0

*/

#ifndef _FOCAL_IMAGE_H
#define _FOCAL_IMAGE_H
  
#include <stdint.h>

typedef struct {
	int16_t width;		/** < width in pixels, -1 indicates unknown value*/
	int16_t height;	/** < heigt in pixels, -1 indicates unknown value*/
	int16_t ppi;		/** < sampling intensity, -1 indicates unknown value*/
	int8_t bits_per_pixels;	/** < Greyscale bit depth of the pixels, -1 indicates unknown value */
	int8_t channels;		/** < Number of channels */
	int8_t greyscale_polarity; /** < 0 indicates that ridges are black, valleys are white, 1 indicates the inverse situation , -1 indicates unknown value*/
} FocalFrameFormat;
 
typedef FocalFrameFormat focal_frame_format_t;

typedef struct {
	FocalFrameFormat format;
	uint16_t frame_count;
	uint8_t* buffer;
	uint32_t capacity;
} FocalImage;

typedef FocalImage focal_image_t;
typedef FocalImage FocalStitchedImg;
typedef FocalImage FocalCoverageMask;
 
#endif

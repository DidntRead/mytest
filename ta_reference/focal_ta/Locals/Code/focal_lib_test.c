/**
 * @file   focal_main_test.c
 *
 */
	 
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "tci.h"
#include "spi.h"

#include "tlStd.h"
#include "TlApi/TlApi.h"

#include "focal_log.h"


//#include "focal_main_test.h"
#include "focal_lib_test.h"

#include "drtlspi_api.h"
#include "drspi_Api.h"


#include "focal_spi.h"

/* add for alipay begin*/
//#include "drfocalApi.h"
/* add for alipay end*/

 
 //vincent
static struct mt_chip_conf spi_chip_config;
static uint8_t tx_buf[16645];
static uint8_t rx_buf[16645];
 
 //#define _PERFORMANCE_MEASUREMENT_
 
 
int32_t focal_lib_test(int32_t x,int32_t y)
{
	int32_t z;
	z = x + y;
	return z;
}

 

 



#ifndef __FOCAL_FP_SPI_H__
#define __FOCAL_FP_SPI_H__

#include <stdint.h>

#define MAX_BULK_READ_LEN 0x3fff

int32_t focal_fp_sensor_write_sfr_register(uint8_t reg_addr, uint8_t reg_value);
int32_t focal_fp_sensor_read_sfr_register(uint8_t reg_addr, uint8_t *reg_value);
int32_t focal_fp_sensor_write_sram_register(uint16_t reg_addr, uint16_t reg_value);
int32_t focal_fp_sensor_write_sram_register_bit(uint16_t reg_addr, int16_t bit_buf[], int32_t len);
int32_t focal_fp_sensor_read_sram_register(uint16_t reg_addr, uint16_t *reg_value);
int32_t focal_fp_sensor_readbulk_sram(uint16_t startAddr, uint16_t *pReadBuf, uint16_t ReadLen);
int32_t focal_fp_spi_reset(void);

int32_t platform_spi_write(uint8_t *tx_buf, int32_t len);
int32_t platform_spi_read(uint8_t *tx_buf, uint8_t *rx_buf, int32_t len);


#endif/*__FOCAL_FP_SPI_H__*/


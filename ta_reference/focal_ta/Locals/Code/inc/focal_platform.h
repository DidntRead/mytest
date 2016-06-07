#ifndef __FOCAL_PLATFOEM_H__
#define __FOCAL_PLATFOEM_H__

void platform_usleep(int us);
int32_t platform_spi_write(uint8_t *tx_buf, uint32_t len);
int32_t platform_spi_read(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len);


#endif /*__FOCAL_PLATFOEM_H__*/

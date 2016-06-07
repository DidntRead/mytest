
#ifndef __FOCAL_SPI_H__
#define __FOCAL_SPI_H__

#include <stddef.h>
#include <stdint.h>

extern int focal_spi_init(uint32_t freq_low_khz, uint32_t freq_high_khz);

extern int focal_spi_destroy(void);

extern int focal_spi_select_low_freq(void);
extern int focal_spi_select_high_freq(void);

extern int focal_spi_read(uint8_t *buff, size_t num_bytes);
extern int focal_spi_write(uint8_t *buff, size_t num_bytes);
extern int focal_spi_writeread(uint8_t *tx, size_t tx_bytes, uint8_t *rx, size_t rx_bytes);

extern int focal_spi_cs_low(void);
extern int focal_spi_cs_high(void);

extern int focal_hw_reset_high(void);
extern int focal_hw_reset_low(void);

extern int focal_delay_us(int usec);

#endif // __FOCAL_SPI_H__


#ifndef _FOCAL_MAIN_TEST_H_
#define _FOCAL_MAIN_TEST_H_

#include <stddef.h>
#include <stdint.h>

// ------------------------------------------------------------------------------------------------
//--------------------------------------
#if 1
extern void focal_test_spi_config(struct mt_chip_conf *chip_config);
#endif
extern uint32_t focal_test_spi_register_test(void);
//--------------------------------------

extern void focal_test_spi_chipcfg(void);
extern int focal_test_spi_test(void);
extern void focal_test_read_int_loop(void);
extern int focal_test_spi_init(uint32_t freq_low_khz, uint32_t freq_high_khz);
extern int focal_test_spi_destroy(void);
extern int focal_test_spi_select_low_freq(void);
extern int focal_test_spi_select_high_freq(void);
#if 0
extern int focal_test_spi_read(uint8_t *buff, size_t num_bytes);
extern int focal_test_spi_write(uint8_t *buff, size_t num_bytes);
extern int focal_test_spi_writeread(uint8_t *tx, size_t tx_bytes, uint8_t *rx, size_t rx_bytes);
#endif
extern int focal_test_spi_cs_low(void);
extern int focal_test_spi_cs_high(void);
extern int focal_test_hw_reset_high(void);
extern int focal_test_hw_reset_low(void);
#if 0
extern int focal_test_delay_us(int usec);
#endif
// ------------------------------------------------------------------------------------------------

#endif /* _FOCAL_MAIN_TEST_H_ */

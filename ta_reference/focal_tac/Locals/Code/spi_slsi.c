
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "spi_slsi.h"

#define LOG_TAG "spi_slsi"
#include "log.h"

static const char spi_dev[]="/dev/spidev4.0";

static int g_fd_spi_device;

struct sec_spi_info
{
	int port;
	//unsigned long speed;
	unsigned long long speed;
};

#define SEC_IOC_SPI_PREPARE	_IOWR('k', 21, struct sec_spi_info)
#define SEC_IOC_SPI_UNPREPARE	_IOWR('k', 22, struct sec_spi_info)

int spi_prepare(int port, unsigned long speed)
{
        int ret;
        struct sec_spi_info spi_info;

        spi_info.port = port;
        spi_info.speed = speed;

	LOG_I("%s (%lx)\n",__func__, SEC_IOC_SPI_PREPARE);

        g_fd_spi_device = open(spi_dev, O_RDWR);
 
 	if (g_fd_spi_device < 0) {
                LOG_E("open SPI device error\n");
                goto err_dev_open;
        }

        ret = ioctl(g_fd_spi_device, SEC_IOC_SPI_PREPARE, &spi_info);
        if (ret != 0) {
                LOG_E("Failed to prepare spi: ret(%d)", ret);
                goto err_dev_ioctl;
        }

        return 0;

err_dev_ioctl:
        close(g_fd_spi_device);
err_dev_open:
        return -1;
}

int spi_unprepare(int port, unsigned long speed)
{
        int ret;
        struct sec_spi_info spi_info;

        spi_info.port = port;
        spi_info.speed = speed;

	LOG_I("%s (%lx)\n",__func__, SEC_IOC_SPI_UNPREPARE);

        ret = ioctl(g_fd_spi_device, SEC_IOC_SPI_UNPREPARE, &spi_info);
        if (ret != 0) {
                LOG_E("Fail to unprepare spi: ret(%d)", ret);
                goto err_dev_ioctl;
        }

        close(g_fd_spi_device);
        return 0;

err_dev_ioctl:
        close(g_fd_spi_device);
        return -1;
}


fdtput -t x /media/boot/exynos5422-odroidxu3.dtb /spi@12d30000 samsung,spi-src-clk 0
fdtput -t x /media/boot/exynos5422-odroidxu3.dtb /spi@12d30000 num-cs 1
fdtput -c /media/boot/exynos5422-odroidxu3.dtb /spi@12d30000/spidev
fdtput -t s /media/boot/exynos5422-odroidxu3.dtb /spi@12d30000/spidev compatible "spidev"
fdtput -t x /media/boot/exynos5422-odroidxu3.dtb /spi@12d30000/spidev reg 0
fdtput -t i /media/boot/exynos5422-odroidxu3.dtb /spi@12d30000/spidev spi-max-frequency 20000000
fdtput -c /media/boot/exynos5422-odroidxu3.dtb /spi@12d30000/spidev/controller-data
fdtput -t x /media/boot/exynos5422-odroidxu3.dtb /spi@12d30000/spidev/controller-data cs-gpio 0x46, 0x5, 0x0
fdtput -t x /media/boot/exynos5422-odroidxu3.dtb /spi@12d30000/spidev/controller-data samsung,spi-feedback-delay 0
ls /dev/spidev*


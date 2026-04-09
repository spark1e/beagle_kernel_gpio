##
## Makefile — gpio_irq_logger kernel module
##
## Build natively on the BeagleBone Black:
##   make
##
## Cross-compile from a host machine (adjust paths to your toolchain):
##   make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
##        KDIR=/path/to/bbb-kernel-source
##
## Install / remove:
##   sudo insmod gpio_irq_logger.ko gpio_pin=60 trigger=0
##   sudo rmmod gpio_irq_logger
##   dmesg | grep gpio_irq
##

obj-m := gpio_irq_logger.o

# Default: build against the running kernel (native on BBB)
KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean

ifneq ($(KERNELRELEASE),)
obj-m := ik220.o
else
KERNEL_SRCDIR := /usr/src/kernels/2.6.32-754.27.1.el6.i686/

default:

	$(MAKE) -C $(KERNEL_SRCDIR) M=$$PWD modules
endif

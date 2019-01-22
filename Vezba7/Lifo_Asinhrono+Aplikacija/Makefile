# If KERNELRELEASE is defined, we've been invoked from
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	obj-m := Lifo.o
# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
# Ako KERNELDIR nije definisan postavi ga na ovu vrednost
KERNELDIR ?= /root/linux-xlnx-zynmp-dt-fixes-for-4.10
# Podesi promenljivu PWD na vrednost trenutnog direktorijuma
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	gcc -o test asynctest.c
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f *~
	rm -f test

endif

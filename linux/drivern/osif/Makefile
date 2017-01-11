# needed environment variables
# (shold be set by the reconos toolchain)
# ARCH, CROSS_COMPILE, KDIR

obj-m := reconos_osif.o
reconos_osif-objs := osif.o

EXTRA_CFLAGS+=-I$(PWD)/include
EXTRA_CFLAGS+=-I$(PWD)/../common/include
EXTRA_CFLAGS+=-I$(PWD)/../../../lib/include

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

obj-m := ima_test.o

KDIR := /lib/modules/$(shell uname -r)/build
VERBOSE = 0

all:
	$(MAKE) -C $(KDIR) M=$(PWD) K_BUILD_VERBOSE=$(VERBOSE) CPNFIG_DEBUG_INFO=y modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean


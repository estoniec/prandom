obj-m += gfrandom.o
gfrandom-objs := driver.o prandom.o lib.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

test_algo: test.c lib.c prandom.c
	gcc -o test test.c lib.c prandom.c -I.

test: test_algo
	./test

.PHONY: all clean test test
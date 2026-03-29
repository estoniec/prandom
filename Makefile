obj-m += gfrandom.o
gfrandom-objs := driver.o prandom.o Galois_Field_256.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

test_algo: test.c Galois_Field_256.c prandom.c
	gcc -o test test.c Galois_Field_256.c prandom.c

test_sanitized: test.c Galois_Field_256.c prandom.c
	gcc -o test test.c Galois_Field_256.c prandom.c -fsanitize=address,undefined -g

test: test_algo
	./test

.PHONY: all clean test test

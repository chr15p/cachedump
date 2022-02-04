obj-m += cachedump.o

MODULE=cachedump.ko

ifndef KVER
KVER=$(shell uname -r)
endif

all:
	make -C /lib/modules/${KVER}/build M=$(PWD) modules

clean:
	make -C /lib/modules/${KVER}/build M=$(PWD) clean

install:
	sudo install -v -m 755 -d /lib/modules/$(KVER)/
	sudo install -v -m 644 ${MODULE} /lib/modules/$(KVER)/${MODULE} 
	sudo depmod -a

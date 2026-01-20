obj-m += socket_probe.o
obj-m += sock_alloc_probe.o
obj-m += inet_create_probe.o
obj-m += linkage_probe.o
obj-m += security_probe.o
obj-m += caller_probe.o
obj-m += socket_dual_probe.o
obj-m += socket_latency_probe.o
obj-m += puzzle_probe.o
obj-m += full_chain_probe.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

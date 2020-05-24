#!/bin/sh

qemu-system-arm \
	-M versatilepb \
	-kernel xhyp \
	-nographic \
	-chardev file,id=s11,server,nowait,path=/tmp/xhyp_log \
	-device virtio-serial-device,id=virtio-serial0,max_ports=16 \
	-device virtserialport,chardev=s11,name=org.linux-kvm.port.11,bus=virtio-serial0.0,id=port11

exit 0

	-device_add virtio-serial-pci,id=virtio-serial0,max_ports=16,bus=pci.0,addr=0x8
	-chardev socket,id=s11,path=/tmp/tt11,server,nowait \
	-chardev stdio,id=s11,server,nowait \
	-nodefaults \

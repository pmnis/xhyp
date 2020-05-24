#!/bin/sh

qemu-system-arm \
	-M versatilepb \
	-kernel xhyp \
	-chardev file,id=s0,server,nowait,path=/tmp/xhyp_0_log \
	-chardev file,id=s1,server,nowait,path=/tmp/xhyp_log \
	-device virtio-serial-device,id=virtio-serial0,max_ports=16 \
	-device virtserialport,chardev=s1,name=port0,bus=virtio-serial0.0,id=port0 \
	-device virtconsole,bus=virtio-serial0.0,chardev=s0,id=cons0,name=org.fedoraproject.console.0 \
	-nographic


exit 0

	-device_add virtio-serial-pci,id=virtio-serial0,max_ports=16,bus=pci.0,addr=0x8
	-chardev socket,id=s11,path=/tmp/tt11,server,nowait \
	-chardev stdio,id=s11,server,nowait \
	-nodefaults \


	-nographic \
	-nodefaults  -chardev stdio,id=s11,server,nowait,path=/tmp/xhyp_log \
	-device virtserialport,chardev=s1,name=org.linux-kvm.port.1,bus=virtio-serial0.0,id=port0 \

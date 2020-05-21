#!/bin/sh

qemu-system-arm -M versatilepb -kernel xhyp -nographic  -device virtio-serial-device

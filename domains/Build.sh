#!/bin/ksh

function copy_os
{
	print "Creating binary for ${PWD##*/}"
	${CROSS_COMPILE}objcopy os -O  binary domain$1.bin
	return $?
}

function build_domain
{
	[[ -r os ]] || {
		print "No OS found for domain: ${PWD##*/}"
		exit 1
	}
	copy_os $1

        arm-linux-objcopy  \
                --input-target binary \
                --output-target elf32-littlearm  \
                --binary-architecture arm \
                --rename-section .data=.dm${1} \
                domain$1.bin domain.ho

	return $?
}

# delete old domains

# is that all ?
[[ $1 == "clean" ]] && {
	rm -rf domain??
	exit 0
}

# no, then build domains
for d in 1 2 3 4
do
	[[ -d domain0${d} ]] || exit 1
	( cd domain0$d && build_domain $d) || exit 1
done


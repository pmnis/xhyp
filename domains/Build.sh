#!/bin/ksh

function copy_os
{
	print "Creating binary for ${PWD##*/}"
	${CROSS_COMPILE}objcopy os -O  binary domain.bin
	return $?
}

function build_domain
{
	[[ -r os ]] || {
		print "No OS found for domain: ${PWD##*/}"
		exit 1
	}
	copy_os
	return $?
}

# delete old domains

# is that all ?
[[ $1 == "clean" ]] && {
	rm -rf domain??
	exit 0
}

# no, then build domains
for d in domain??
do
	print "Building $d $*"
	[[ -d ${d} ]] || exit 1
	( cd $d && build_domain $*) || exit 1
done


#!/bin/ksh


function usage
{
	print "$0: address domain AP"
	exit 0
}
[[ $# == 3 ]] || usage()

typeset -i 16 addr
typeset -i 16 domain
typeset -i 16 ap
typeset -i 16 pte
typeset -i 16 section

l1=${1//0x}
l2=${2//0x}

addr="0x$l1"
domain="0x$l2"
section=0x012

function get_ap
{
	case $1 in
		"rwna")	print "0x01"
		;;
		"rwro")	print "0x02"
		;;
		"rwrw")	print "0x03"
		;;
	esac
}

addr=$(( addr << 20 ))
pte=$(( addr | section))
domain=$(( domain << 5))
ap=$(get_ap $3)
ap=$(( ap << 10 ))
pte=$(( pte | domain | ap))

printf "pte	:	0x%08x\n" $pte


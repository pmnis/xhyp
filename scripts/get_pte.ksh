#!/bin/ksh

typeset -i 16 pte
typeset -i 16 domain

l=${1//0x}

pte="0x$l"
printf "PTE: 0x%08lx\n" $pte

function get_ap
{
	typeset -i 16 x
	typeset -i d

	x=$1
	x=$(( x & 0x03 ))
	d=$x
	case $d in
		0)	print "depends on SR"
		;;
		1)	print "RW-NA"
		;;
		2)	print "RW-RO"
		;;
		3)	print "RW-RW"
		;;
	esac
}

section=$(( (pte & 0xfffff000) >> 12 ))
#domain=$(( (pte & 0x000001e0) >> 5))
ap=$(( (pte & 0x00000ff0) >> 4))
type=$(( pte & 0x03 ))

printf "type	:	%x\n" $type
printf "address	:	%x\n" $section
#printf "domain	:	%x\n" $domain
printf "ap	:	%x	%s\n" $ap
#get_ap $ap
#exit
printf "ap	:	%x	%s\n" $ap $(get_ap $ap)


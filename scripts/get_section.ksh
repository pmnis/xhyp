#!/bin/ksh

typeset -i 16 pte
typeset -i 16 domain

l=${1//0x}

pte="0x$l"
printf "PTE: 0x%08lx\n" $pte

function get_ap
{
	case $1 in
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

section=$((pte >> 20))
domain=$(( (pte & 0x000001e0) >> 5))
ap=$(( (pte & 0x00000c00) >> 10))
type=$(( pte & 0x03 ))

printf "type	:	%x\n" $type
printf "section	:	%x\n" $section
printf "domain	:	%x\n" $domain
printf "ap	:	%x	%s\n" $ap $(get_ap $ap)


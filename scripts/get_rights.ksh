#!/bin/ksh

typeset -i 16 rights

l=${1//0x}

rights="0x$l"
printf "rights: 0x%08lx\n" $rights

function get_r
{
	case $1 in
		0)	printf "NoACCESS"
		;;
		1)	printf "CLIENT"
		;;
		2)	printf "RESERVED"
		;;
		3)	printf "MANAGER"
		;;
	esac
}


i=0
while (( i < 16))
do
	(( i == 0 )) && print "XHYP"
	(( i == 4 )) && print "RTOS"
	(( i == 8 )) && print "LINUX"
	(( i == 12 )) && print "NONE"
	printf "domain %02d	:	%s\n" $i $(get_r $(( rights & 0x03 )) )
	: $(( rights = rights >> 2))
	: $(( i++))
done
exit 0

section=$((pte >> 20))
domain=$(( (pte & 0x000001e0) >> 5))
ap=$(( (pte & 0x00000c00) >> 10))
type=$(( pte & 0x03 ))

printf "section	:	%x\n" $section
printf "domain	:	%x\n" $domain
printf "ap	:	%x	%s\n" $ap $(get_ap $ap)


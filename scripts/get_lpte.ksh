#!/bin/ksh

typeset -i 16 pte
typeset -i 16 domain

l=${1//0x}

pte="0x$l"
printf "PTE: 0x%08lx\n" $pte

function get_ap
{
	typeset -i 16 x

	x=$1
	(( x & (1<<2) )) && printf "F" || printf "."
	(( x & (1<<3) )) && printf "P" || printf "."
	(( x & (1<<4) )) && printf "Y" || printf "."
	(( x & (1<<6) )) && printf "D" || printf "."
	(( x & (1<<7) )) && printf "R" || printf "."
	(( x & (1<<8) )) && printf "U" || printf "."
	(( x & (1<<9) )) && printf "X" || printf "."
	(( x & (1<<10) )) && printf "S" || printf "."
}

section=$(( (pte & 0xfffff000) >> 12 ))
#domain=$(( (pte & 0x000001e0) >> 5))
ap=$(( (pte & 0x00000fff) ))
type=$(( pte & 0x03 ))

printf "type	   :	%x\n" $type
printf "address	   :	%x\n" $section
printf "Linux bits :	%x	%s\n" $ap $(get_ap $ap)


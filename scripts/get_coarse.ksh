#!/bin/ksh

typeset -i 16 pte
typeset -i 16 domain
typeset -i 16 coarse

l=${1//0x}

pte="0x$l"
printf "PTE: 0x%08lx\n" $pte

coarse=$((pte & 0xfffffc00))
domain=$(( (pte & 0x000001e0) >> 5))
type=$(( pte & 0x03 ))

printf "type		:	%x\n" $type
printf "coarse entry	:	0x%08x\n" $coarse
printf "domain		:	%x\n" $domain


#!/bin/ksh

typeset -i 16 pte
typeset -i 16 domain

MASK=0xffffffff
IMASK=0xfff
l=${1//0x}

pte="0x$l"
printf "PTE: 0x%08lx\n" $(( pte & MASK ))


printf "Index: %08lx\n" $(( pte >> 20 & IMASK ))

printf "Offset: %08lx\n" $(( 4 * (pte >> 20 & IMASK) ))


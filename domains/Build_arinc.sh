#!/bin/ksh

typeset -i nb_frame=0

while read type id start size reste 
do
	[[ ! ${type} || ${type: 0:1} == "#" ]] && continue
	[[ ${type}  == "major" ]] && continue
	[[ ${type}  == "minor" ]] && : $(( nb_frame++ ))
done < arinc_table

mkdir -p ${BASEDIR}/generated

{ # redirect all on generated/arinc_table.c
cat << EOF
/*
 * arinc.c
 *
 * generated arinc frames
 *
 * Author: Pierre Morel <pmorel@mnis.fr>
 *
 * $LICENSE:
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <xhyp/arinc.h>

struct major_frame init_major_frame = {
EOF

function major
{
	print ".minor_count = $nb_frame,"
	print ".frame_start = $1,"
	print ".frame_size = $2,"
	print ".frame_period = $3,"
	print ".minor = {"
}

function minor
{
	print "{"
	print ".dom_id = $3,"
	print ".slot_start = $1,"
	print ".slot_size = $2,"
	print "},"
}

while read type id start size reste 
do
	[[ ! ${type} || ${type:0} == "#" ]] && continue
	[[ ${type}  == "major" ]] && major ${start} ${size} ${reste}
	[[ ${type}  == "minor" ]] && minor ${start} ${size} ${reste}
done < arinc_table

print "},"
print "};"
print "struct major_frame *major = &init_major_frame;"

} > ${BASEDIR}/generated/arinc_table.c

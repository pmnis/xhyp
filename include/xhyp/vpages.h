/*
 * vpages.h
 *
 * Virtual Pages definitions
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

#ifndef __VPAGES_H
#define __VPAGES_H

#define VPGD_SIZE		0x00001000
#define VPMD_SIZE		0x00000100

#define VPMD_VALID_BIT		0x00000001
#define VPMD_DOMAIN_MASK	0x000001e0
#define VPMD_DOMAIN_SHIFT	5
#define VPMD_ADDR_MASK		0xfffffc00
#define VPMD_TO_PMD_OFFSET	0x00004000 /* Offset for real PMD	*/

#define VSEC_VALID_BIT	0x00000001
#define VSEC_TYPE_BIT	0x00000002
#define VSEC_CACHE_BIT	0x00000010
#define VSEC_BUFFER_BIT	0x00000020
#define VSEC_DOMAIN_MASK	0x000001e0
#define VSEC_DOMAIN_SHIFT	5
#define VSEC_ACCESS_BIT	0x00002000
#define VSEC_SPECIAL_BIT	0x00004000
#define VSEC_DIRTY_BIT	0x00008000
#define VSEC_EXEC_BIT	0x00010000
#define VSEC_WRITE_BIT	0x00020000
#define VSEC_READ_BIT	0x00040000
#define VSEC_ADDR_MASK	0xfff00000
#define VSEC_TO_PTE_OFFSET	VPMD_TO_PMD_OFFSET

#define VSEC_DEFAULT	(VSEC_VALID_BIT|VSEC_TYPE_BIT|VSEC_WRITE_BIT|VSEC_READ_BIT|VSEC_EXEC_BIT)

#define VPTE_VALID_BIT		0x00000001
#define VPTE_FILE_BIT		0x00000002
#define VPTE_SWAP_BIT		0x00000004
#define VPTE_CACHE_BIT		0x00000010
#define VPTE_BUFFER_BIT		0x00000020
#define VPTE_ACCESS_BIT		0x00000040
#define VPTE_SPECIAL_BIT	0x00000080
#define VPTE_DIRTY_BIT		0x00000100
#define VPTE_EXEC_BIT		0x00000200
#define VPTE_WRITE_BIT		0x00000400
#define VPTE_READ_BIT		0x00000800

#define VPTE_ADDR_MASK		0xfffff000
#define VPTE_TO_PTE_OFFSET	0x00001000 /* Offset for real PTE	*/
/*
 * VPTE_VALID_BIT	set	PTE_SMALL|PTE_BITS
 * VPTE_FILE_BIT	ignore
 * VPTE_SWAP_BIT	ignore
 * VPTE_CACHE_BIT	set 	PTE_CACHED
 * VPTE_BUFFER_BIT	set 	PTE_BUFFERED
 * VPTE_ACCESS_BIT	ignore
 * VPTE_SPECIAL_BIT	ignore
 * VPTE_DIRTY_BIT	ignore
 * VPTE_EXEC_BIT	ignore
 * VPTE_WRITE_BIT	AP: 1 1 else AP: 1 0
 * VPTE_READ_BIT	ignore
 */

#endif

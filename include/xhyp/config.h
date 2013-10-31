/*
 * config.h
 *
 * XHYP configuration
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

#ifndef __CONFIG_H
#define __CONFIG_H

#include <autoconf.h>
#include <xhyp/generated.h>
#include <xhyp/pages.h>

#ifndef __ASSEMBLY__
extern unsigned long periph_base;

#define MEMORY_SIZE	CONFIG_MEMORY_SIZE

#define STACK_SIZE	CONFIG_STACK_SIZE

#endif

#define PERIPH_BASE	CONFIG_PERIPH_BASE	/* 256 Mo */
#define PERIPH_SIZE	CONFIG_PERIPH_SIZE	/*  16 Mo */

#define VERSION_STRING	"xhyp 1.0"
#define COPYRIGHTS_STRING	"(C) MNIS"

#define XHYP_TBL_SIZE	MAX_MMU_TABLE_SIZE * NB_DOMAINS
#define XHYP_CORE_SIZE	0x00100000 
#define XHYP_MEM_SIZE	0x02000000

#define D01_BASE_ADDR	CONFIG_D01_ADDR
#define D01_VBASE_ADDR	XHYP_MEM_SIZE
#define D01_SPAGE_ADDR	D01_BASE_ADDR
#define D01_TAGS_ADDR	D01_SPAGE_ADDR + 0x4000
#define D01_LOAD_ADDR	D01_BASE_ADDR + SECTION_SIZE
#define D01_START_ADDR	D01_VBASE_ADDR + SECTION_SIZE
#define D01_PHYS2VIRT	D01_BASE_ADDR - D01_VBASE_ADDR
#define D01_SIZE	CONFIG_D01_SIZE
#define D01_FLAGS	DFLAGS_CACHE_WT


#define D02_BASE_ADDR	CONFIG_D02_ADDR
#define D02_VBASE_ADDR	XHYP_MEM_SIZE
#define D02_SPAGE_ADDR	D02_BASE_ADDR 
#define D02_TAGS_ADDR	D02_SPAGE_ADDR + 0x4000
#define D02_LOAD_ADDR	D02_BASE_ADDR + SECTION_SIZE
#define D02_START_ADDR	D02_VBASE_ADDR + SECTION_SIZE
#define D02_PHYS2VIRT	D02_BASE_ADDR - D02_VBASE_ADDR
#define D02_SIZE	CONFIG_D02_SIZE
#define D02_FLAGS	DFLAGS_CACHE_WT
//#define D02_OFFSET	0xC00	/* if linux is C0 */
#define D02_OFFSET	0x00	/* if linux is C0 */

#define D03_BASE_ADDR	CONFIG_D03_ADDR
#define D03_VBASE_ADDR	XHYP_MEM_SIZE
#define D03_SPAGE_ADDR	D03_BASE_ADDR 
#define D03_TAGS_ADDR	D03_SPAGE_ADDR + 0x4000
#define D03_LOAD_ADDR	D03_BASE_ADDR + SECTION_SIZE
#define D03_START_ADDR	D03_VBASE_ADDR + SECTION_SIZE
#define D03_PHYS2VIRT	D03_BASE_ADDR - D03_VBASE_ADDR
#define D03_SIZE	CONFIG_D03_SIZE
#define D03_FLAGS	DFLAGS_CACHE_WT
#define D03_OFFSET	0xC00	/* if linux is C0 */

#define D04_BASE_ADDR	CONFIG_D04_ADDR
#define D04_VBASE_ADDR	XHYP_MEM_SIZE
#define D04_SPAGE_ADDR	D04_BASE_ADDR 
#define D04_TAGS_ADDR	D04_SPAGE_ADDR + 0x4000
#define D04_LOAD_ADDR	D04_BASE_ADDR + SECTION_SIZE
#define D04_START_ADDR	D04_VBASE_ADDR + SECTION_SIZE
#define D04_PHYS2VIRT	D04_BASE_ADDR - D04_VBASE_ADDR
#define D04_SIZE	CONFIG_D04_SIZE
#define D04_FLAGS	DFLAGS_CACHE_WT
#define D04_OFFSET	0x000


#endif

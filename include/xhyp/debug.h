/*
 * debug.h
 *
 * debug definitions
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

#ifndef __DEBUG_H
#define __DEBUG_H

#include <xhyp/sched.h>

#define DEBUG

#define DEB_PANIC	0x00000001
#define DEB_CALL	0x00000002
#define DEB_CTX		0x00000004
#define DEB_SCHED	0x00000008
#define DEB_TRACE	0x00000010
#define DEB_IRQ		0x00000020
#define DEB_ABT		0x00000040
#define DEB_INFO	0x00000080
#define DEB_HYP		0x00001000
#define DEB_MMU		0x00002000
#define DEB_CACHE	0x00004000
#define DEB_PTE		0x00008000
#define DEB_TLB		0x00010000
#define DEB_TAGS	0x00020000
#define DEB_ALLOC	0x00040000
#define DEB_MODE	0x00080000
#define DEB_DEBUG	0x10000000
#define DEB_ALL		0xffffffff
#define DEB_F1		DEB_PANIC|DEB_CALL|DEB_IRQ|DEB_FAULT|DEB_SCHED

extern unsigned long debug_level;
extern void show_regs(unsigned long *);
extern void dump_pgd(unsigned long *p);


#ifdef DEBUG
#define deb_printf(lvl, format, arg...) do { \
	if (lvl & debug_level) { \
	time_update(); \
	if (!current) current = &domain_table[0]; \
        	printk("%03d.%06d [%d/%d/%d]-(%s-%d)-%s: " format , \
		xtime.tv_sec, xtime.tv_usec, \
		current->id, current->mode, current->old_mode,\
	 	__FILE__, __LINE__,  __func__, ## arg); \
	}} while (0)
#elif DEBUG2
#define deb_printf(lvl, format, arg...) do { \
	if (lvl & debug_level) { \
	time_update(); \
	if (!current) current = &domain_table[0]; \
	if ((current->id != 0) && (current->id != 4)) { \
        	printk("%03d.%06d [%d/%d]-(%s-%d)-%s: " format , \
		xtime.tv_sec, xtime.tv_usec, \
		current->id, current->mode, \
	 	__FILE__, __LINE__,  __func__, ## arg); \
	}}} while (0)
#else
#define deb_printf(lvl, format, arg...) do {} while (0)
#endif

#define debinfo(format, arg...)	deb_printf(DEB_INFO, format, ## arg)
#define debirq(format, arg...)	deb_printf(DEB_IRQ, format, ## arg)
#define debabt(format, arg...)	deb_printf(DEB_ABT, format, ## arg)
#define debsched(format, arg...) deb_printf(DEB_SCHED, format, ## arg)
#define debhyp(format, arg...) deb_printf(DEB_HYP, format, ## arg)
#define debpte(format, arg...) deb_printf(DEB_PTE, format, ## arg)
#define deballoc(format, arg...) deb_printf(DEB_ALLOC, format, ## arg)
#define debpanic(format, arg...) deb_printf(DEB_PANIC, format, ## arg)
#define debtlb(format, arg...) deb_printf(DEB_TLB, format, ## arg)
#define debcache(format, arg...) deb_printf(DEB_CACHE, format, ## arg)
#define debdeb(format, arg...) deb_printf(DEB_DEBUG, format, ## arg)
#define debmode(format, arg...) deb_printf(DEB_MODE, format, ## arg)
#define debctx(format, arg...) deb_printf(DEB_CTX, format, ## arg)

extern int PANIC;

#endif


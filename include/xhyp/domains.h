/*
 * domains.h
 *
 * Domains definitions
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
#ifndef __DOMAINS_H
#define __DOMAINS_H

#include <xhyp/config.h>
#include <xhyp/list.h>
#include <xhyp/pages.h>
#include <xhyp/context.h>
#include <xhyp/timer.h>
#include <xhyp/shared_page.h>
#include <xhyp/lowlevel.h>
#include <xhyp/soc.h>

#define DTYPE_HYP	0
#define DTYPE_DRV	1
#define DTYPE_RTOS	2
#define DTYPE_GPOS	3

#define DSTATE_NONE     0x0000
#define DSTATE_READY    0x0001
#define DSTATE_RUN      0x0002

#define DSTATE_SLEEP    0x0010
#define DSTATE_STOP     0x0020

#define DSTATE_DEAD     0x0100

#define DSTATE_RMASK	0x0003
#define DSTATE_SMASK	0x0030


#define DEV_START	0x00
#define DEV_STOP	0x01
#define DEV_SCHEDIN	0x02
#define DEV_SCHEDOUT	0x03
#define DEV_ERROR	0x04
#define DEV_SLEEP	0x05
#define DEV_WAKEUP	0x06
#define DEV_SIZE	0x07

#define DMODE_INIT	0x0000
#define DMODE_SVC	0x0001
#define DMODE_IRQ	0x0002
#define DMODE_USR	0x0003

#define DMODE_ABT	0x0004
#define DMODE_UND	0x0005
#define DMODE_FIQ	0x0006
#define DMODE_SYSCALL	0x0007
#define DMODE_HYPCALL	0x0008
#define DMODE_NONE	0x0009
#define DMODE_SIZE	0x000a

#define DFLAGS_HYPCALL	0x0001

#define DMEV_INIT	0x00
#define DMEV_IRQ	0x01
#define DMEV_IRET	0x02
#define DMEV_SYSCALL	0x03
#define DMEV_SYSRET	0x04
#define DMEV_SIZE	0x05

extern int table_mode[DMODE_SIZE][DMEV_SIZE];

extern char *mode_str[DMODE_SIZE];

#define DFLAGS_MASK	0x00ff
#define DFLAGS_CACHE_WB	0x0001
#define DFLAGS_CACHE_WT	0x0002
#define DFLAGS_BUFFER	0x0010

struct domain {
	struct list	list;
	unsigned long	tbl_l1;
	unsigned long	*tbl_l2;
	unsigned long	state;
	unsigned long	old_state;
	unsigned long	status;
	unsigned long	mode;
	unsigned long	old_mode;
	unsigned long	old_mode2;
	unsigned long	type;
	unsigned long	flags;
	unsigned long	tag;
	unsigned long	d_sum;
	int		ctx_level;
	int		no_check;
	char name[32];
/* Load stuff 					*/
	unsigned long	base_addr;
	unsigned long	vbase_addr;
	unsigned long	load_addr;
	unsigned long	start_addr;
	struct shared_page	*sp;
	unsigned long	tags_addr;
	unsigned long	offset;
	unsigned long	phys2virt;
	unsigned long	phys2hyp;
	unsigned long	size;
	unsigned long	device;
	unsigned long	device_size;
/* Scheduling stuff				*/
	unsigned long	prio;
	int 		budget;
	int		period;
	int		allocated_slices;
	unsigned long	jiffies;
	unsigned long	irq;
	unsigned long	id;
	unsigned long	rights;
	unsigned long	syscall;
	unsigned long	hypercall;
	unsigned long	nb_hypercalls;
	unsigned long	slices;
/* Statistiques					*/
	unsigned long t_mode;
	struct timespec	t_irqin;
	struct timespec	t_irq;
	struct timespec	t_abtin;
	struct timespec	t_abt;
	struct timespec	t_sysin;
	struct timespec	t_sys;
	struct timespec	t_usrin;
	struct timespec	t_usr;
/* Virtualisation stuff				*/
	unsigned long	mmu_on;
	struct context	ctx;
	struct context	ctx_syscall;
	//struct shadow	s_usr;
	//struct shadow	s_irq;
	//struct shadow	s_abt;
	//struct shadow	s_und;
	//struct shadow	s_sys;
	//struct shadow	s_rst;
	//unsigned long	(*v_irq_handler)(unsigned long);
	unsigned long	d_irq_pending;
	unsigned long	d_irq_enabled;
	unsigned long	d_irq_mask;
	unsigned long	d_drv_type;
	unsigned long	d_pgd;
	unsigned long	d_cpsr;
	unsigned long	d_spsr;
};

struct runqueue {
	unsigned long bitmap;
	struct list list[32];
};

extern struct domain domain_table[];
extern struct domain *xhyp;

extern int setup_domains(void);
extern void schedule_domains(void);
extern void schedule(void);

extern struct domain *current;

extern pgd_t map_usr(struct domain *d, unsigned long idx);
extern pgd_t map_drv(struct domain *d, unsigned long idx);
extern pgd_t map_usr_ro(struct domain *d, unsigned long idx);

#define MAP_WITH_OFFSET	0x01
extern void domain_build_pagetables(struct domain *d, unsigned long flags);

static inline unsigned long domain_to_rights(int id)
{
	return D_CLIENT << (2 * id);
}

static inline unsigned long guest_to_rights(int type, int id)
{
	unsigned long tmp;

	switch (type) {
	case DTYPE_HYP:
		tmp = D_HYP;
		break;
	case DTYPE_DRV:
	case DTYPE_RTOS:
		tmp = D_CLIENT | domain_to_rights(id);
		break; 
	case DTYPE_GPOS:
		tmp = D_CLIENT | D_GPOS;
		break;
	}
	return tmp;
}

#define G_CLIENT	(D_CLIENT | (D_CLIENT << 2) | (D_CLIENT << 4) || (D_CLIENT << 6))
static inline unsigned long rights_translate(struct domain *d, unsigned long rights)
{
	unsigned long tmp;

	tmp = rights & G_CLIENT;	/* only 4 doms per linux	*/
					/* partition always client */
	tmp = ((tmp) << (12 )) | D_CLIENT;	/* linux part uses	*/
					/* domains 12,13,14 and 15	*/
	return tmp;
}

extern void send_irq(struct domain *d, unsigned long irq);

#define new_mode(x) { current->old_mode = current->mode; current->mode= x;}
#define idle_domain	&domain_table[0]

static inline void switch_to(void)
{
	_dom_ttb = current->tbl_l1;
	_dom_rights = current->rights;
	_context->sregs.spsr &= ~0xff;
	_context->sregs.spsr |= 0x10;
        _switch_to();
}

extern void mode_new(struct domain *d, int mode);
extern void mode_save(struct domain *d, int mode);
extern void mode_set(struct domain *d, int mode);
extern void mode_restore(struct domain *d);

#endif


/*
 * soc.h
 *
 * System On Chip definitions
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

/*
 * the actual reset code
 */
#define m_mask	0x1f
#define m_abt	0x17
#define m_fiq	0x11
#define m_irq	0x12
#define m_svc	0x13
#define m_sys	0x1f
#define m_und	0x1b
#define m_usr	0x10

#define dis_fiq		0x40
#define dis_irq		0x80
#define dis_irqs	0xc0
/* MSK: Disable IRQ and FIQ, set ARM state and Supervisor mode		*/
#define mode_init	(m_svc|dis_irqs)
#define mode_domain	m_usr
#define mask_domain	(dis_irqs|m_mask)
/*
 *	C0:	Status Register
 *	C1:	Control Register
 *	C2:	Translation Table Base Register
 *	C3:	Domain Access Control Register
 *	C4:	RESERVED
 *	C5:	Fault Status Register
 *	C6:	Fault Address Register
 *	C7:	Cache operations
 *	C8:	TLB Operations
 *	C9:	Cache LockDown and TCM operations
 *	C10:	TLB LockDown operations
 *	C11:	RESERVED
 *	C12:	RESERVED
 *	C13:	Process ID register
 *	C14:	RESERVED
 *	C15:	Test and Debug
 */

/* setup the MEMORY							*/
/*
 *	Control Register bits
 * 	0000 0000 0000 0000 LRVI 00rS B111 1CAM
 *	L: use THUMB				not T/T
 *	R: replacement strategy for cache	Random/RR
 *	V: Exception vectors			low/high
 *	I: Instructon cache			disabled/enabled
 *	r: ROM Protection
 *	S: System Protection (MMU)
 *	B: Endianess				little/big
 *	C: Data cache				disabled/enabled
 *	A: Alignement fault checking		disabled/enabled
 *	M: MMU					disabled/enabled
*/
#define CP1_M	0x00000001
#define CP1_A	0x00000002
#define CP1_C	0x00000004
#define CP1_B	0x00000080
#define CP1_S	0x00000100
#define CP1_r	0x00000200
#define CP1_I	0x00001000
#define CP1_V	0x00002000
#define CP1_R	0x00004000
#define CP1_l	0x00008000
#define CP1_init_clear	CP1_r | CP1_S | CP1_B | CP1_C | CP1_M
#define CP1_init_set	CP1_I|CP1_A


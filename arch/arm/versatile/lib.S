/* XHYP startup code for versatilepb
 *
 * xhyp starts here and calls reset
*/

#include <xhyp/soc.h>
#include <xhyp/mmu.h>


#define mmu_coarse	0x11
	@ clean and flush TLB
	@ clean and flush D-cache
	@ flush I-cache

	.text
tci_loop:
	mrc	p15, 0, r15, c7, c14, 3	@ test clean and flush D-cache
	bne	tci_loop
	mov	r1, #0
	mcr	p15, 0, r1, c7, c10, 4	@ write barrier
	mov	r1, #0
	mcr	p15, 0, r1, c7, c5, 0	@ Flush I-cache
	mov	r1, #0
	mcr	p15, 0, r1, c8, c7, 0	@ Invalidate TLB

.globl _load_tlb
_load_tlb:
	mcr	p15, 0, r0, c2, c0, 0	@ Load TTB Address
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, #0
	mcr     p15, 0, r0, c8, c7, 0   @ Invalidate TLB
	mcr     p15, 0, r0, c8, c6, 0   @ Invalidate TLB
	mcr     p15, 0, r0, c8, c5, 0   @ Invalidate TLB
	mcr     p15, 0, r0, c8, c7, 0   @ Invalidate TLB
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	pc, lr

.globl _setup_mmu
_setup_mmu:
	mcr	p15, 0, r0, c7, c10, 4	@ write barrier
	mcr	p15, 0, r0, c2, c0, 0	@ Load TTB Address
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mrc	p15, 0, r0, c1, c0, 0	@ Read control register in r0
	orr	r0, r0, #1		@ Enable MMU
	mcr	p15, 0, r0, c1, c0, 0	@ Load control register from r0
	mov	r0, r0
	mov	r0, r0
	mov	pc, lr

/*
** Managing Control register 0
*/
.globl _get_c0_ID
_get_c0_ID:
	mrc	p15, 0, r0, c0, c0, 0
	mov	pc, lr
.globl _get_c0_CT
_get_c0_CT:
	mrc	p15, 0, r0, c0, c0, 1
	mov	pc, lr
.globl _get_c0_TCM
_get_c0_TCM:
	mrc	p15, 0, r0, c0, c0, 2
	mov	pc, lr

/*
** Managing Control register 0
*/
.globl _get_c0
_get_c0:
	mrc	p15, 0, r0, c0, c0, 0
	mov	pc, lr
/*
** Managing Control register 1
*/
.globl _get_c1
_get_c1:
	mrc	p15, 0, r0, c1, c0, 0
	mov	pc, lr
/*
** Managing Control register 2
*/
.globl _get_c2
_get_c2:
	mrc	p15, 0, r0, c2, c0, 0
	mov	pc, lr
/*
** Managing Control register 3
*/
.globl _get_c3
_get_c3:
	mrc	p15, 0, r0, c3, c0, 0
	mov	pc, lr
.globl _set_c3
_set_c3:
	mcr	p15, 0, r0, c3, c0, 0
	mov	pc, lr
.globl _get_spsr
_get_spsr:
	mrs	r0, spsr
	mov	pc, lr
.globl _setup_domain
_setup_domain:
	mcr	p15, 0, r0, c3, c0, 0	@ Load domain's register
	mov	pc, lr

.globl _get_cpsr
_get_cpsr:
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	mov	pc, lr

@Data Abort Fault status register handling

.globl _get_dfsr
_get_dfsr:
	mrc	p15, 0, r0, c5, c0, 0	@ R0 = Data FSR
	mov	pc, lr

.globl _get_ifsr
_get_ifsr:
	mrc	p15, 0, r0, c5, c0, 1	@ R0 = Instr FSR
	mov	pc, lr

.globl _get_far
_get_far:
	mrc	p15, 0, r0, c6, c0, 0	@ R0 = Fault Address Register
	mov	pc, lr

@ TLB maintenance
.globl _tlb_flush
_tlb_flush:
	push	{r0}
	mov	r0, #0
	mcr	p15, 0, r0, c8, c7, 0	@ Invalidate TLB
	pop	{r0}
	mov	pc, lr

.globl _cache_flush
_cache_flush:
	mrc     p15, 0, r15, c7, c14, 3	@ Flush all cache
	bne _cache_flush
	tst	r0, #HYP_FC_ALLI
	moveq	r0, #0
	mcreq	p15, 0, r0, c7, c5, 0	@ Invalisate I cache
	mcreq	p15, 0, r0, c7, c10, 4	@ write barrier
	mov	pc, lr

.globl _clean_d_entry
_clean_d_entry:
	mcr     p15, 0, r0, c7, c10, 1
	mcr     p15, 0, r0, c7, c10, 4
	mov	pc, lr

/*
**	IRQ management
*/
.globl _cpu_it_enable
_cpu_it_enable:
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	bic	r0,r0,#dis_irq	@ BIT CLEAR dis_irq
	msr	cpsr,r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	pc, lr

.globl _cpu_it_disable
_cpu_it_disable:
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	orr	r0,r0,#dis_irq	@ disable IRQs
	msr	cpsr,r0
	mov	pc, lr

# Wait for interrupt in V5TE
.globl _wfi
_wfi:
	mrs     r0, cpsr			@ Enable IRQ
	bic     r0, r0, #0xc0
	msr	cpsr, r0

	mov	r0, #0
        mcr     p15, 0, r0, c7, c0, 4           @ Wait for interrupt
	b	_wfi


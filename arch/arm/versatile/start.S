/* XHYP startup code for versatilepb
 *
 * xhyp starts here and calls reset
*/

#include <xhyp/soc.h>
#include <xhyp/mmu.h>

/* We need to know the position of the segments defined in the LDS	*/

/* Here we start				*/
.globl _start
_start:
	b	reset
	ldr	pc, _undef
	ldr	pc, _swi
	ldr	pc, _prefetch
	ldr	pc, _data_abrt
	ldr	pc, _hang
	ldr	pc, _irq
	ldr	pc, _fiq

_hang:		.word	do_hang
_swi:		.word	_exp_swi
_undef:		.word	_exp_undefined
_prefetch:	.word	_exp_prefetch
_data_abrt:	.word	_exp_data_abrt
_irq:		.word	_exp_irq
_fiq:		.word	_exp_fiq

	.align 8

.globl	_context
_context:	.word	_saved_regs
.globl	_saved_regs
_saved_regs:	.space	0x100, 0

.globl	_hyp_ttb
_hyp_ttb:	.word	0
.globl	_hyp_rights
_hyp_rights:	.word	0
.globl	_dom_ttb
_dom_ttb:	.word	0
.globl	_dom_rights
_dom_rights:	.word	0


# MACROS
	.macro go_sys
	mrs	r0, cpsr
	bic	r0, r0, #m_mask
	orr	r0, r0, #m_sys
	msr	cpsr, r0
	.endm
	.macro go_svc
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	bic	r0, r0, #m_mask
	orr	r0, r0, #m_svc
	msr	cpsr, r0
	.endm
	.macro save_frame
	ldr	sp, _context
        stm	sp, {r0 - r12}		@ save all registers
        add     sp, sp, #60		@ update pointer
        mrs     r3, cpsr                @ get CPSR
        mrs     r2, spsr                @ get SPSR
        mov     r1, lr			@ get link register
        stm	sp, {r1 , r2, r3}       @ and save it all
	.endm

        .macro sys_enter
	save_frame
	
	go_sys

	mov	r1, sp			@ save shadow sp
	mov	r2, lr			@ save shadow lr
	
	go_svc

	ldr	sp, _context
        add     sp, sp, #52		@ update pointer
        stm	sp, {r1 , r2}           @ and save shadow sp/lr

	ldr	r0, _hyp_rights
	mcr	p15, 0, r0, c3, c0, 0	@ Load domain's register

	@mrc	p15, 0, r0, c1, c0, 0	@ Load control register in r0
	@bic	r0, r0, #CP1_V		@ set Exceptions in low page
	@mcr	p15, 0, r0, c1, c0, 0

	ldr	r0, _hyp_ttb
	mcr	p15, 0, r0, c2, c0, 0	@ Load TTB Address
	mov	r0, r0
	mov	r0, r0
	mov	r0, #0
	mcr     p15, 0, r0, c8, c7, 0   @ Invalidate TLB
	mcr     p15, 0, r0, c8, c6, 0   @ Invalidate TLB
	mcr     p15, 0, r0, c8, c5, 0   @ Invalidate TLB
	mcr     p15, 0, r0, c8, c7, 0   @ Invalidate TLB
	mcr	p15, 0, r0, c7, c5, 4	@ Prefetch flush
	mov	r0, r0
	mov	r0, r0
	ldr	r0, _c_stack
	mov	sp, r0

        .endm

        .macro sys_leave
	ldr	r0, _dom_rights
	mcr	p15, 0, r0, c3, c0, 0	@ Load domain's register

	ldr	r0, _dom_ttb
	mcr	p15, 0, r0, c2, c0, 0	@ Load TTB Address
	mov	r0, r0
	mov	r0, r0
	mov	r0, #0
	mcr     p15, 0, r0, c8, c7, 0   @ Invalidate TLB
	mcr     p15, 0, r0, c8, c6, 0   @ Invalidate TLB
	mcr     p15, 0, r0, c8, c5, 0   @ Invalidate TLB
	mcr     p15, 0, r0, c8, c7, 0   @ Invalidate TLB
	mcr	p15, 0, r0, c7, c5, 4	@ Prefetch flush
	mov	r0, r0

	ldr	sp, _context
        ldr     r0, [sp, #64]           @ retrieve spsr
        ldr     r1, [sp, #56]           @ retrieve usr_lr
        ldr     r2, [sp, #52]           @ retrieve usr_sp
        msr     spsr, r0                @ load spsr
	mov	r0, #0

	go_sys				@ go SYS
	mov	lr, r1			@ load shadow lr
	mov	sp, r2			@ load shadow sp

	go_svc
        ldm	sp, {r0 - r12}          @ restore registers
	add	sp, sp, #60		@ update pointer
	ldm	sp, {pc}^		@ go to userland
	nop
	nop
	nop
        .endm

_exp_irq:
	subs	lr, lr, #4		@ adjust the return address
	sys_enter
	bl	exp_irq
h2:					@ NEVER REACHED
	b	h2

_exp_swi:
	sys_enter
	mov	r0, lr
	bl	exp_swi			@ handle SWI
	sys_leave

/* The following routine never returns	*/
/* r0: current context			*/
.globl _switch_to
_switch_to:
	sys_leave

_exp_undefined:
	sys_enter
	bl	exp_undefined		@ r1: saved registers
	@ we only return if we emulate the undefined instruction
	sys_leave

_exp_prefetch:
	subs	lr, lr, #4		@ adjust the return address
	sys_enter
	mov	r0, lr
	bl	exp_prefetch
	@ we only return if we have an error in a domain
	sys_leave

_exp_data_abrt:
	subs	lr, lr, #8		@ adjust the return address
	sys_enter
	mov	r0, lr
	bl	exp_data_abrt
	sys_leave


_exp_fiq:
	stmfd	sp!,{r0-r12, lr}
	nop
	nop
	bl	exp_fiq
	ldmfd	sp,{r0-r12, pc}
	nop
	nop

reset:
	mov	r7, r0
	mov	r8, r1
	mov	r9, r2
	mrs	r0,cpsr			@ GET PROCESSOR STATUS REGISTER
	bic	r0,r0,#m_mask		@ BIT CLEAR mode mask
	orr	r0,r0,#mode_init	@ SET SVC and disable IRQs
	msr	cpsr,r0			@ LOAD PSR
	ldr	sp, _svc_stack		@ Top stack at _svc_stack
	mrc	p15, 0, r0, c1, c0, 0	@ Load control register in r0
	bic	r0, r0, #CP1_M|CP1_C|CP1_B	@ clear memory controls
	bic	r0, r0, #CP1_S|CP1_r	@ clear rom and system protections
	@orr	r0, r0, #CP1_I		@ set Instruction cache
	@orr	r0, r0, #CP1_C		@ set Data cache
	orr	r0, r0, #CP1_A		@ set Address alignement
	@orr	r0, r0, #CP1_V		@ set Exceptions in high page
	mcr	p15, 0, r0, c1, c0, 0
	bl setup_stacks			@ setup all modes stacks

	mov	r0, r7
	mov	r1, r8
	mov	r2, r9
	ldr	sp, _c_stack
	ldr	pc, _start_xhyp		@ jump to C code

_start_xhyp:	.word	start_xhyp
_c_stack:	.word	xhyp_stack

		.space	0x100, 0
_abt_stack:	.word	__abt_stack
__abt_stack:	.space	0x100, 0
_fiq_stack:	.word	__fiq_stack
__fiq_stack:	.space	0x100, 0
_irq_stack:	.word	__irq_stack
__irq_stack:	.space	0x100, 0
_svc_stack:	.word	__svc_stack
__svc_stack:	.space	0x100, 0
_sys_stack:	.word	__sys_stack
__sys_stack:	.space	0x100, 0
_und_stack:	.word	__und_stack
__und_stack:	.space	0x100, 0

	.align	8
setup_stacks:
	/* Init ABT 		*/
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	bic	r0, r0, #m_mask
	orr	r0, r0, #m_abt
	msr	cpsr, r0
	ldr	sp, _abt_stack
	/* Init FIQ 		*/
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	bic	r0, r0, #m_mask
	orr	r0, r0, #m_fiq
	msr	cpsr, r0
	ldr	sp, _fiq_stack
	/* Init IRQ 		*/
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	bic	r0, r0, #m_mask
	orr	r0, r0, #m_irq
	msr	cpsr, r0
	ldr	sp, _irq_stack
	/* Init UND		*/
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	bic	r0, r0, #m_mask
	orr	r0, r0, #m_und
	msr	cpsr, r0
	ldr	sp, _und_stack
	/* Init SYS and USR	*/
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	bic	r0, r0, #m_mask
	orr	r0, r0, #m_sys
	msr	cpsr, r0
	ldr	sp, _sys_stack
	/* Init SVC		*/
	mrs	r0,cpsr		@ LOAD PROCESSOR STATUS REGISTER
	bic	r0, r0, #m_mask
	orr	r0, r0, #m_svc
	msr	cpsr, r0
	ldr	sp, _svc_stack
	mov	pc, lr

	.align	5
do_hang:
	bl	do_hang

/*
**	r0: Translation Table Base Address
*/
#define mmu_coarse	0x11
	@ clean and flush TLB
	@ clean and flush D-cache
	@ flush I-cache
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
/* It management */
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

.globl _wfi
_wfi:
	mrs     r0, cpsr			@ Disable IRQ
	orr     r0, r0, #0xc0
	msr	cpsr, r0
	mov	r0, #0
        mcr     p15, 0, r0, c7, c0, 4           @ Wait for interrupt
	mrs     r0, cpsr			@ enable IRQ
	bic     r0, r0, #0xc0
	msr	cpsr, r0
	mov	pc, lr

.globl _wfi2
_wfi2:
        mrs     r0, cpsr                        @ enable IRQ
        bic     r0, r0, #0xc0
        msr     cpsr, r0
        b       _wfi2


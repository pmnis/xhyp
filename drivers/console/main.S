/*
 * very simple domain code
 *
*/
#include <xhyp/hyp.h>

/* Here we start				*/

.globl _start
_start:
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	ldr	sp, _stack_top
	@sub	sp, sp, #0x100
	ldr	pc, _start_kernel

	.globl _magic
_magic:
	.word	0xcafec0ca
	.globl _start_kernel
_start_kernel:
	.word	start_kernel


_stack_base01:
	.space 1024
_stack_top:
	.word	__stack_end__
	.space 1024


.globl _hyp_console
_hyp_console:
        swi	#_HYP_console
        mov     pc, lr

.globl _hyp_get_cp15
_hyp_get_cp15:
        swi	#_HYP_cpuctrl
        mov     pc, lr

.globl _hyp_idle
_hyp_idle:
	swi	#_HYP_idle
	mov	pc, lr

.globl _hyp_exit
_hyp_exit:
	swi	#_HYP_exit
	mov	pc, lr

.globl _hyp_syscall_request
_hyp_syscall_request:
	swi	#_HYP_syscall_request
	mov	pc, lr

.globl _hyp_syscall_return
_hyp_syscall_return:
	swi	#_HYP_syscall_return
	mov	pc, lr

.globl _hyp_irq_request
_hyp_irq_request:
	swi	#_HYP_irq_request
	mov	pc, lr

.globl _hyp_irq_enable
_hyp_irq_enable:
	swi	#_HYP_irq_enable
	mov	pc, lr

.globl _hyp_irq_disable
_hyp_irq_disable:
	swi	#_HYP_irq_disable
	mov	pc, lr

.globl _hyp_irq_return
_hyp_irq_return:
	swi	#_HYP_irq_return

.globl _hyp_syscall
_hyp_syscall:
	swi	#_HYP_syscall

.globl _hyp_pgfault_request
_hyp_pgfault_request:
	swi	#_HYP_pgfault_request

.globl _hyp_undef_request
_hyp_undef_request:
	swi	#_HYP_undef_request

.globl get_cpsr
get_cpsr:
@	mrs	r0, cpsr
	mov	pc, lr

.globl _hyp_context_switch
_hyp_context_switch:
	swi	#_HYP_switch_task
	mov	pc, lr

.globl _hyp_preempt_enable
_hyp_preempt_enable:
	@swi	#_HYP_preempt_enable
	mov	pc, lr

.globl _hyp_preempt_disable
_hyp_preempt_disable:
	@swi	#_HYP_preempt_disable
	mov	pc, lr

.globl _hyp_hyp
_hyp_hyp:
	swi	#_HYP_hyp
	mov	pc, lr


	.space 1024


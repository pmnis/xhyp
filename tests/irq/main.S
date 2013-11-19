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
	ldr	sp, =_stack_top
	mov	lr, #4
	mov	r12, sp
	stmfd	sp!, {r11-r12, lr-pc}
	sub	r11, r12, #4
	mov	r0, #0
	bl	start_kernel
	bl	_hyp_exit


	.space 1024
_stack_top:
	.word	__stack_end__



        .global _hyp_io_write
_hyp_io_write:
        svc     #_HYP_io_write
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
	mov	pc, lr

.globl _hyp_syscall
_hyp_syscall:
	swi	#_HYP_syscall
	mov	pc, lr

.globl _hyp_pgfault_request
_hyp_pgfault_request:
	swi	#_HYP_pgfault_request
	mov	pc, lr

.globl _hyp_undef_request
_hyp_undef_request:
	swi	#_HYP_undef_request
	mov	pc, lr

.globl _hyp_new_pgd
_hyp_new_pgd:
	swi	#_HYP_new_pgd
	mov	pc, lr

.globl _hyp_console
_hyp_console:
	swi	#_HYP_console
	mov	pc, lr



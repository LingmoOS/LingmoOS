#
# arch/arm64/setjmp.S
#
# setjmp/longjmp for arm64
#

# we specify -mgeneral-regs-only as a build flag thus do not need to
# save d8-d15

	.text
	.balign 8
	.globl setjmp
	.type setjmp, #function
setjmp:
	mov	x1, sp
	stp	x19, x20, [x0, #0]
	stp	x21, x22, [x0, #16]
	stp	x23, x24, [x0, #32]
	stp	x25, x26, [x0, #48]
	stp	x27, x28, [x0, #64]
	stp	x29, x30, [x0, #80]
	str	x1,       [x0, #96]
	mov	x0, #0 			/* set the return value of setjmp */
	br	x30
	.size setjmp,.-setjmp

	.text
	.balign 8
	.globl longjmp
	.type longjmp, #function
longjmp:
	ldp	x19, x20, [x0, #0]
	ldp	x21, x22, [x0, #16]
	ldp	x23, x24, [x0, #32]
	ldp	x25, x26, [x0, #48]
	ldp	x27, x28, [x0, #64]
	ldp	x29, x30, [x0, #80]
	ldr	x2,	  [x0, #96]
	mov	sp, x2
	mov	x0, x1
	cbnz	x1, 1f
	mov	x0, #1
1:
	br	x30
	.size longjmp,.-longjmp

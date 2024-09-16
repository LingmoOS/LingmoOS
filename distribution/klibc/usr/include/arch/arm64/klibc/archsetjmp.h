/*
 * arch/arm64/include/klibc/archsetjmp.h
 */

#ifndef _KLIBC_ARCHSETJMP_H
#define _KLIBC_ARCHSETJMP_H

/*
 * x19-x28 are callee saved, also save fp, lr, sp.
 * d8-d15 are unused as we specify -mgeneral-regs-only as a build flag.
 */

struct __jmp_buf {
	uint64_t __x19, __x20, __x21, __x22;
	uint64_t __x23, __x24, __x25, __x26;
	uint64_t __x27, __x28, __x29, __x30;
	uint64_t __sp;
};

typedef struct __jmp_buf jmp_buf[1];

#endif				/* _SETJMP_H */

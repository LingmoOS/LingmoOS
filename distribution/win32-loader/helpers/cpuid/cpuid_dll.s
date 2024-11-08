/* Licensed under the zlib/libpng license (same as NSIS) */

/*
 CPUID hepler module for the Nullsoft Installer (NSIS).

 The exported cpuid_available and cpuid functions should be used
 with the cpuid.nsh header.
 */

.text
/*
 Check if CPUID opcode is available by toggling ID bit in EFLAGS register
 Returns a non-zero value if CPUID opcode is available otherwise 0.

 C function declaration:
 __declspec(dllexport) unsigned int __attribute__((__stdcall__))
 cpuid_available(void)
 */
.globl _cpuid_available@0
_cpuid_available@0:
	/* Transfer content of EFLAGS register into ECX via stack */
	pushfl
	popl	%ecx
	/* Save original content of EFLAGS register in EAX */
	movl	%ecx, %eax
	/* Toggle ID bit (bit number: 21) */
	xorl	$0x00200000, %ecx
	/* Update EFLAGS register with the contents of ECX and
	   read back EFLAGS into ECX */
	pushl	%ecx
	popfl
	pushfl
	popl	%ecx
	/* Restore original content of EFLAGS register */
	pushl	%eax
	popfl
	/* If ID bit could be modified in EFLAGS register then the
	   exclusive-or (XOR) operation leads to a non-zero value in EAX */
	xorl	%ecx, %eax
	ret

/*
 Invoke CPUID opcode
 Parameters:
   info - array of four 32 bit intergers getting the contents of the registers EAX, EBX, ECX and EDX
   id - query code

 C function declaration:
 __declspec(dllexport) void __attribute__((__stdcall__))
 cpuid(unsigned int info[4], unsigned int id)
 */
.globl _cpuid@8
_cpuid@8:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	/* Write query code to EAX */
	movl	12(%ebp), %eax
	cpuid
	/* Save contents of the registers EAX, EBX, ECX and EDX
	   in the provided array */
	movl	%eax, (%esi)
	movl	%ebx, 4(%esi)
	movl	%ecx, 8(%esi)
	movl	%edx, 12(%esi)
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret	$8

/*
 C function declaration:
 BOOL __attribute__((__stdcall__))
 DllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
 */
.globl _DllEntryPoint@12
_DllEntryPoint@12:
        movl    $1, %eax
        ret	$12

.section .drectve
.ascii " -export:\"cpuid@8\""
.ascii " -export:\"cpuid_available@0\""

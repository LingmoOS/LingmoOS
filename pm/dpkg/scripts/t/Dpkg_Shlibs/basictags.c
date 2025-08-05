/*
 * $ gcc -shared -fPIC -Wl,-soname -Wl,libbasictags.so.1 \
 *     -o libbasictags.so.1 -DAMD64 basictags.c
 * $ objdump -wfpTR libbasictags.so.1 > objdump.basictags-amd64
 * $ gcc -shared -fPIC -Wl,-soname -Wl,libbasictags.so.1 \
 *     -o libbasictags.so.1 -DI386 basictags.c
 * $ objdump -wfpTR libbasictags.so.1 > objdump.basictags-i386
 * $ gcc -shared -fPIC -Wl,-soname -Wl,libbasictags.so.1 \
 *     -o libbasictags.so.1 -DMIPS basictags.c
 * $ objdump -wfpTR libbasictags.so.1 > objdump.basictags-mips
 */

void symbol11_optional(void);
void symbol11_optional(void) {}

#if defined(AMD64)
void symbol21_amd64(void);
void symbol21_amd64(void) {}
#elif defined(I386)
void symbol22_i386(void);
void symbol22_i386(void) {}
#elif defined(MIPS)
void symbol23_mips(void);
void symbol23_mips(void) {}
#endif

#if defined(AMD64)
void symbol25_64(void);
void symbol25_64(void) {}
#else
void symbol24_32(void);
void symbol24_32(void) {}
#endif

#if defined(MIPS)
void symbol27_big(void);
void symbol27_big(void) {}
#else
void symbol26_little(void);
void symbol26_little(void) {}
#endif

#if defined(I386)
void symbol28_little_32(void);
void symbol28_little_32(void) {}
#endif

void symbol31_randomtag(void);
void symbol31_randomtag(void) {}

/* (arch=i386|optional)symbol41_i386_and_optional@Base */
#if defined(I386)
void symbol41_i386_and_optional(void);
void symbol41_i386_and_optional(void) {}
#endif
/* (arch=mips|optional)symbol42_i386_and_optional@Base */
#if defined(MIPS)
void symbol42_mips_and_optional(void);
void symbol42_mips_and_optional(void) {}
#endif

void symbol51_untagged(void);
void symbol51_untagged(void) {}

/* Licensed under the zlib/libpng license (same as NSIS) */

/*
 SHA256 hepler module for the Nullsoft Installer (NSIS).
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdint.h>

#define BUFFER_SIZE 262144
#define SHA256_BLOCK_LEN 64
#define SHA256_STATE_LEN 8

extern void
sha256_compress(uint32_t state[SHA256_STATE_LEN],
                const uint8_t block[SHA256_BLOCK_LEN]) asm("sha256_compress");

typedef struct {
  uint32_t hash[SHA256_STATE_LEN];
  uint64_t size; /* total size */
  size_t n;      /* bytes in block */
  uint8_t block[SHA256_BLOCK_LEN];
} sha256_context_t;

/**
 * Allocate and acquire context
 * @return Pointer of allocated and intialized context which has to be freed by
 * caller via destroy
 */
sha256_context_t *__declspec(dllexport) __stdcall init(void) {
  sha256_context_t *context = GlobalAlloc(GPTR, sizeof(sha256_context_t));
  if (context) {
    uint32_t *hash = context->hash;
    *(hash++) = UINT32_C(0x6A09E667);
    *(hash++) = UINT32_C(0xBB67AE85);
    *(hash++) = UINT32_C(0x3C6EF372);
    *(hash++) = UINT32_C(0xA54FF53A);
    *(hash++) = UINT32_C(0x510E527F);
    *(hash++) = UINT32_C(0x9B05688C);
    *(hash++) = UINT32_C(0x1F83D9AB);
    *hash = UINT32_C(0x5BE0CD19);
  }
  return context;
}

/**
 * Include provided data into the computation of the hash value
 * @param context Pointer to context information
 * @param data Pointer of a buffer containing data to be added
 * @param len Number of bytes of data to be added
 * @return 0 if the function failed
 */
int __declspec(dllexport) __stdcall update(sha256_context_t *context,
                                           const uint8_t *data, size_t len) {
  uint32_t *hash = context->hash;
  register const uint8_t *p = data, *q;
  size_t n = context->n;

  if (n > 0) {
    /* Take care of last block */
    size_t r = SHA256_BLOCK_LEN - n;
    if (r > len) {
      r = len;
    }
    /* Fill remaining space of last block with current data */
    CopyMemory(context->block + n, p, r);
    p += r;
    n += r;
    if (n < SHA256_BLOCK_LEN) {
      context->n = n;
    } else {
      /* Process complete block */
      sha256_compress(hash, context->block);
      context->n = 0;
    }
    n = len - r;
  } else {
    n = len;
  }

  /* Determine starting address of the first incomplete block
   *
   * SHA256_BLOCK_LEN must be a power of 2 number so that the
   * bit mask works as expected.
   */
  q = p + (n & ((size_t) ~(SHA256_BLOCK_LEN - 1)));

  while (p < q) {
    sha256_compress(hash, p);
    p += SHA256_BLOCK_LEN;
  }

  n = p - data;
  if (n < len) {
    /* Copy left over data into block */
    n = len - n;
    CopyMemory(context->block, p, n);
    context->n = n;
  }

  context->size += len;
  return 1;
}

/**
 * Complete hashing and get hash value
 * @param context Pointer to context information
 * @param hash Array of eight unsigned 32-bit integers receiving the final
 *             hash value
 */
void __declspec(dllexport) __stdcall final(sha256_context_t *context,
                                           uint32_t hash[SHA256_STATE_LEN]) {
  uint64_t size = context->size;
  register uint8_t *p = context->block;
  uint8_t *q = p + SHA256_BLOCK_LEN;
  const uint8_t *r = q - sizeof(size);
  size_t n = context->n;

  CopyMemory(hash, context->hash, sizeof((*context).hash));

  p += n;
  *(p++) = 0x80;
  if (p > r) {
    SecureZeroMemory(p, (q - p));
    p = context->block;
    sha256_compress(hash, p);
  }
  SecureZeroMemory(p, (r - p));

  /* Append total size in bits (1 byte = 8 bits, times 8 = left shift by 3) */
  p = q;
  size <<= 3;
  while (p > r) {
    *(--p) = (size & 0xFF);
    size >>= 8;
  }
  sha256_compress(hash, context->block);
}

/**
 * Destroys the context information
 * @param context Pointer to context information
 * @return 0 if the function failed
 */
int __declspec(dllexport) __stdcall destroy(sha256_context_t *context) {
  return (NULL == GlobalFree(context));
}

/**
 * Compute SHA256 hash value of the content read from provided file handle
 * @param fh Handle of file to read from
 * @param hash Array of eight unsigned 32-bit integers receiving the final
 *             hash value
 */
void __declspec(dllexport) __stdcall hash(HANDLE fh,
                                          uint32_t hash[SHA256_STATE_LEN]) {
  uint8_t *buffer = GlobalAlloc(GMEM_FIXED, BUFFER_SIZE);

  if (buffer) {
    sha256_context_t *context = init();
    if (context) {
      DWORD n;
      while (ReadFile(fh, buffer, BUFFER_SIZE, &n, NULL) && n > 0) {
        update(context, buffer, n);
      }
      final(context, hash);
      destroy(context);
    }
    GlobalFree(buffer);
  }
}

BOOL __stdcall DllMainCRTStartup(HINSTANCE hinstDll, DWORD dwReason,
                                 LPVOID lpvReserved) {
  return TRUE;
}

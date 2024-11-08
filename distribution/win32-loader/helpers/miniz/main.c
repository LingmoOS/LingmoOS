/* Licensed under the zlib/libpng license (same as NSIS) */

/*
 miniz hepler module for the Nullsoft Installer (NSIS).
 */

#define WIN32_LEAN_AND_MEAN
#include <stdint.h>
#include <windows.h>

#include "miniz.h"
#include "miniz_tdef.h"

/**
 * Compute the 32-bit cyclic redundancy check (CRC32) value of the
 * provided data
 * @param crc initial CRC32 value
 * @param data pointer of a buffer containing the data
 * @param len size of data in bytes
 * @return CRC32 value
 */
uint32_t __declspec(dllexport) __stdcall crc32(uint32_t crc, const unsigned char *ptr, size_t buf_len)
{
    return (uint32_t)(mz_crc32(crc, ptr, buf_len) & 0xffffffff);
}

/**
 * Destroys the compressor context
 * @param pComp pointer to compressor context information
 */
void __declspec(dllexport) __stdcall destroy(tdefl_compressor *pComp)
{
    GlobalFree(pComp);
}

/**
 * Allocate and acquire compressor context
 * @param flags compression flags
 * @return pointer of allocated and intialized compressor context which has
 * to be freed by caller via destroy
 */
tdefl_compressor *__declspec(dllexport) __stdcall init(int flags)
{
    tdefl_compressor *compressor = GlobalAlloc(GPTR, sizeof(tdefl_compressor));
    if (compressor)
    {
        tdefl_status status = tdefl_init(compressor, NULL, NULL, flags);
        if (status != TDEFL_STATUS_OKAY)
        {
            destroy(compressor);
            /* Set application defined error */
            SetLastError(1 << 29 | status);
            compressor = NULL;
        }
    }
    return compressor;
}

/**
 * Compresses as much as possible of the data from the input buffer and write
 * as much as possible of the resulting compressed data to the specified
 * output buffer
 * @param compressor pointer of allocated and intialized compressor context
 * @param pIn_buf input buffer
 * @param pIn_buf_size size of input buffer in bytes
 * @param pOut_buf output buffer
 * @param pOut_buf_size size of output buffer in bytes
 * @param flush flag to perform flush operation
 * @return negative values indicate a failure
 */
tdefl_status __declspec(dllexport) __stdcall compress(tdefl_compressor *compressor, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, tdefl_flush flush)
{
    return tdefl_compress(compressor, pIn_buf, pIn_buf_size, pOut_buf, pOut_buf_size, flush);
}

BOOL __stdcall DllMainCRTStartup(HINSTANCE hinstDll, DWORD dwReason, LPVOID lpReserved)
{
    return TRUE;
}

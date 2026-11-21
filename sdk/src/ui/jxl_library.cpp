// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The JPEG XL functions the image decoder calls, forwarding to libjxl once
// the first JPEG XL file is read. Linked, libjxl and its Highway, Brotli and
// LCMS dependencies would load, and initialise, in every application.

#include <dlfcn.h>

#include <jxl/decode.h>
#include <jxl/resizable_parallel_runner.h>

static void* openLibrary(const char* name)
{
    return dlopen(name, RTLD_LAZY | RTLD_LOCAL);
}

static void* decoderLibrary()
{
    static void* const handle = openLibrary(LINGMO_JXL_LIBRARY);
    return handle;
}

static void* threadsLibrary()
{
    static void* const handle = openLibrary(LINGMO_JXL_THREADS_LIBRARY);
    return handle;
}

template<typename Function>
static Function* resolve(void* library, const char* name)
{
    return library ? reinterpret_cast<Function*>(dlsym(library, name)) : nullptr;
}

// Each function looks itself up once; without the library, creating a
// decoder or a runner fails, and nothing else is reached.
#define JXL_FUNCTION(library, name) static auto* const real = resolve<decltype(::name)>(library(), #name)

extern "C" {

JxlDecoder* JxlDecoderCreate(const JxlMemoryManager* memory_manager)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderCreate);
    return real ? real(memory_manager) : nullptr;
}

void JxlDecoderDestroy(JxlDecoder* dec)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderDestroy);
    if (real)
        real(dec);
}

JxlDecoderStatus JxlDecoderSubscribeEvents(JxlDecoder* dec, int events_wanted)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderSubscribeEvents);
    return real ? real(dec, events_wanted) : JXL_DEC_ERROR;
}

JxlDecoderStatus JxlDecoderSetParallelRunner(JxlDecoder* dec, JxlParallelRunner parallel_runner,
                                             void* parallel_runner_opaque)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderSetParallelRunner);
    return real ? real(dec, parallel_runner, parallel_runner_opaque) : JXL_DEC_ERROR;
}

JxlDecoderStatus JxlDecoderSetUnpremultiplyAlpha(JxlDecoder* dec, JXL_BOOL unpremul_alpha)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderSetUnpremultiplyAlpha);
    return real ? real(dec, unpremul_alpha) : JXL_DEC_ERROR;
}

JxlDecoderStatus JxlDecoderSetInput(JxlDecoder* dec, const uint8_t* data, size_t size)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderSetInput);
    return real ? real(dec, data, size) : JXL_DEC_ERROR;
}

void JxlDecoderCloseInput(JxlDecoder* dec)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderCloseInput);
    if (real)
        real(dec);
}

JxlDecoderStatus JxlDecoderProcessInput(JxlDecoder* dec)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderProcessInput);
    return real ? real(dec) : JXL_DEC_ERROR;
}

JxlDecoderStatus JxlDecoderGetBasicInfo(const JxlDecoder* dec, JxlBasicInfo* info)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderGetBasicInfo);
    return real ? real(dec, info) : JXL_DEC_ERROR;
}

JxlDecoderStatus JxlDecoderImageOutBufferSize(const JxlDecoder* dec, const JxlPixelFormat* format, size_t* size)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderImageOutBufferSize);
    return real ? real(dec, format, size) : JXL_DEC_ERROR;
}

JxlDecoderStatus JxlDecoderSetImageOutBuffer(JxlDecoder* dec, const JxlPixelFormat* format, void* buffer, size_t size)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderSetImageOutBuffer);
    return real ? real(dec, format, buffer, size) : JXL_DEC_ERROR;
}

JxlDecoderStatus JxlDecoderSetPreferredColorProfile(JxlDecoder* dec, const JxlColorEncoding* color_encoding)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderSetPreferredColorProfile);
    return real ? real(dec, color_encoding) : JXL_DEC_ERROR;
}

JxlDecoderStatus JxlDecoderGetICCProfileSize(const JxlDecoder* dec, JxlColorProfileTarget target, size_t* size)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderGetICCProfileSize);
    return real ? real(dec, target, size) : JXL_DEC_ERROR;
}

JxlDecoderStatus JxlDecoderGetColorAsICCProfile(const JxlDecoder* dec, JxlColorProfileTarget target,
                                                uint8_t* icc_profile, size_t size)
{
    JXL_FUNCTION(decoderLibrary, JxlDecoderGetColorAsICCProfile);
    return real ? real(dec, target, icc_profile, size) : JXL_DEC_ERROR;
}

void* JxlResizableParallelRunnerCreate(const JxlMemoryManager* memory_manager)
{
    JXL_FUNCTION(threadsLibrary, JxlResizableParallelRunnerCreate);
    return real ? real(memory_manager) : nullptr;
}

void JxlResizableParallelRunnerDestroy(void* runner_opaque)
{
    JXL_FUNCTION(threadsLibrary, JxlResizableParallelRunnerDestroy);
    if (real)
        real(runner_opaque);
}

void JxlResizableParallelRunnerSetThreads(void* runner_opaque, size_t num_threads)
{
    JXL_FUNCTION(threadsLibrary, JxlResizableParallelRunnerSetThreads);
    if (real)
        real(runner_opaque, num_threads);
}

uint32_t JxlResizableParallelRunnerSuggestThreads(uint64_t xsize, uint64_t ysize)
{
    JXL_FUNCTION(threadsLibrary, JxlResizableParallelRunnerSuggestThreads);
    return real ? real(xsize, ysize) : 1;
}

JxlParallelRetCode JxlResizableParallelRunner(void* runner_opaque, void* jpegxl_opaque, JxlParallelRunInit init,
                                              JxlParallelRunFunction func, uint32_t start_range, uint32_t end_range)
{
    JXL_FUNCTION(threadsLibrary, JxlResizableParallelRunner);
    return real ? real(runner_opaque, jpegxl_opaque, init, func, start_range, end_range) : JXL_PARALLEL_RET_RUNNER_ERROR;
}

} // extern "C"

// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/egl_dmabuf_importer.h"

#include <GLES3/gl3.h>
// Extension declarations require the core GL types first.
#include <GLES2/gl2ext.h>
#include <drm_fourcc.h>
#include <gbm.h>
#include <sys/stat.h>

#include "core/logging.h"
#include "base/geometry.h"

namespace Lingmo::Render {

namespace {

bool drmFormatHasAlpha(uint32_t format)
{
    switch (format) {
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_XBGR8888:
    case DRM_FORMAT_RGBX8888:
    case DRM_FORMAT_BGRX8888:
    case DRM_FORMAT_XRGB2101010:
    case DRM_FORMAT_XBGR2101010:
    case DRM_FORMAT_RGBX1010102:
    case DRM_FORMAT_BGRX1010102:
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_BGR565:
    case DRM_FORMAT_RGB888:
    case DRM_FORMAT_BGR888:
        return false;
    default:
        return true;
    }
}

} // namespace

EglImageTexture::EglImageTexture(std::shared_ptr<EglContext> context, EGLImageKHR image,
                                 uint32_t name, const base::Size &size, PixelFormat format)
    : GlTexture(name, size, format, GL_TEXTURE_EXTERNAL_OES, std::move(context))
    , m_image(image)
{
}

EglImageTexture::~EglImageTexture()
{
    ScopedEglCurrent current(*m_context);
    if (!current.valid())
        LM_FATAL(lingmoRender) << "Cannot make EGL context current for image retirement";
    auto destroyImage =
            reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (m_image != EGL_NO_IMAGE_KHR && destroyImage)
        destroyImage(m_context->display(), m_image);
}

EglDmaBufImporter::EglDmaBufImporter(std::shared_ptr<EglContext> context, gbm_device *device)
    : m_context(std::move(context))
{
    if (!m_context->supports("EGL_EXT_image_dma_buf_import")) {
        LM_INFO(lingmoRender) << "EGL_EXT_image_dma_buf_import is missing; no linux-dmabuf";
        return;
    }

    // The device clients should allocate on. Without it the protocol falls back
    // to the pre-feedback version.
    struct stat deviceStat = {};
    const int gbmFd = device ? gbm_device_get_fd(device) : -1;
    if (gbmFd >= 0 && fstat(gbmFd, &deviceStat) == 0)
        m_dmaBufMainDevice = deviceStat.st_rdev;

    auto queryFormats = reinterpret_cast<PFNEGLQUERYDMABUFFORMATSEXTPROC>(
            eglGetProcAddress("eglQueryDmaBufFormatsEXT"));
    auto queryModifiers = reinterpret_cast<PFNEGLQUERYDMABUFMODIFIERSEXTPROC>(
            eglGetProcAddress("eglQueryDmaBufModifiersEXT"));

    if (!m_context->supports("EGL_EXT_image_dma_buf_import_modifiers") || !queryFormats
        || !queryModifiers) {
        // Without the query extension only implicit modifiers can be imported,
        // and only for the formats every driver on this platform handles.
        static const uint32_t kImplicitFormats[] = {DRM_FORMAT_ARGB8888, DRM_FORMAT_XRGB8888,
                                                    DRM_FORMAT_ABGR8888, DRM_FORMAT_XBGR8888};
        for (uint32_t format : kImplicitFormats)
            m_dmaBufFormats.emplace(format, std::vector<uint64_t> {});
        LM_INFO(lingmoRender) << "EGL cannot enumerate dmabuf modifiers; implicit modifiers only";
        return;
    }

    EGLint formatCount = 0;
    if (!queryFormats(m_context->display(), 0, nullptr, &formatCount) || formatCount <= 0)
        return;

    std::vector<EGLint> formats(formatCount);
    if (!queryFormats(m_context->display(), formatCount, formats.data(), &formatCount))
        return;
    formats.resize(formatCount);

    for (EGLint format : formats) {
        EGLint modifierCount = 0;
        if (!queryModifiers(m_context->display(), format, 0, nullptr, nullptr, &modifierCount))
            continue;

        std::vector<EGLuint64KHR> modifiers(modifierCount > 0 ? modifierCount : 0);
        if (modifierCount > 0
            && !queryModifiers(m_context->display(), format, modifierCount, modifiers.data(),
                               nullptr, &modifierCount)) {
            continue;
        }
        modifiers.resize(modifierCount > 0 ? modifierCount : 0);

        m_dmaBufFormats.emplace(static_cast<uint32_t>(format),
                                std::vector<uint64_t>(modifiers.begin(), modifiers.end()));
    }

    LM_INFO(lingmoRender).printf("EGL can import %zu DMA-BUF formats", m_dmaBufFormats.size());
}

EGLImageKHR EglDmaBufImporter::createImage(const DmaBufAttributes &attributes) const
{
    if (attributes.size.isEmpty() || attributes.planes.empty()
        || attributes.planes.size() > kMaxDmaBufPlanes) {
        return EGL_NO_IMAGE_KHR;
    }

    auto createImage =
            reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    if (!createImage)
        return EGL_NO_IMAGE_KHR;

    const bool explicitModifier = attributes.modifier != DRM_FORMAT_MOD_INVALID;

    // Per-plane attribute names are not consecutive, so they are spelled out.
    static const EGLint kPlaneFdAttrs[] = {EGL_DMA_BUF_PLANE0_FD_EXT, EGL_DMA_BUF_PLANE1_FD_EXT,
                                           EGL_DMA_BUF_PLANE2_FD_EXT, EGL_DMA_BUF_PLANE3_FD_EXT};
    static const EGLint kPlaneOffsetAttrs[] = {
            EGL_DMA_BUF_PLANE0_OFFSET_EXT, EGL_DMA_BUF_PLANE1_OFFSET_EXT,
            EGL_DMA_BUF_PLANE2_OFFSET_EXT, EGL_DMA_BUF_PLANE3_OFFSET_EXT};
    static const EGLint kPlanePitchAttrs[] = {
            EGL_DMA_BUF_PLANE0_PITCH_EXT, EGL_DMA_BUF_PLANE1_PITCH_EXT,
            EGL_DMA_BUF_PLANE2_PITCH_EXT, EGL_DMA_BUF_PLANE3_PITCH_EXT};
    static const EGLint kPlaneModifierLoAttrs[] = {
            EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT,
            EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT};
    static const EGLint kPlaneModifierHiAttrs[] = {
            EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT,
            EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT, EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT};

    std::vector<EGLint> attribs = {
            EGL_WIDTH,
            attributes.size.width(),
            EGL_HEIGHT,
            attributes.size.height(),
            EGL_LINUX_DRM_FOURCC_EXT,
            static_cast<EGLint>(attributes.format),
    };

    for (size_t i = 0; i < attributes.planes.size(); ++i) {
        const DmaBufPlane &plane = attributes.planes[i];
        attribs.push_back(kPlaneFdAttrs[i]);
        attribs.push_back(plane.fd.get());
        attribs.push_back(kPlaneOffsetAttrs[i]);
        attribs.push_back(static_cast<EGLint>(plane.offset));
        attribs.push_back(kPlanePitchAttrs[i]);
        attribs.push_back(static_cast<EGLint>(plane.stride));
        if (explicitModifier) {
            attribs.push_back(kPlaneModifierLoAttrs[i]);
            attribs.push_back(static_cast<EGLint>(attributes.modifier & 0xffffffff));
            attribs.push_back(kPlaneModifierHiAttrs[i]);
            attribs.push_back(static_cast<EGLint>(attributes.modifier >> 32));
        }
    }
    attribs.push_back(EGL_NONE);

    // EGL dups the fds it needs, so the caller keeps ownership of its own.
    EGLImageKHR image = createImage(m_context->display(), EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
                                    static_cast<EGLClientBuffer>(nullptr), attribs.data());
    if (image == EGL_NO_IMAGE_KHR)
        LM_WARNING(lingmoRender).printf("eglCreateImageKHR failed for a DMA-BUF: 0x%04x", eglGetError());
    return image;
}

std::shared_ptr<Texture> EglDmaBufImporter::import(const DmaBufAttributes &attributes)
{
    auto imageTargetTexture = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
            eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    if (!imageTargetTexture)
        return nullptr;
    EGLImageKHR image = createImage(attributes);
    if (image == EGL_NO_IMAGE_KHR)
        return nullptr;

    GLuint name = 0;
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, name);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    imageTargetTexture(GL_TEXTURE_EXTERNAL_OES, image);
    const GLenum error = glGetError();
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

    if (error != GL_NO_ERROR) {
        LM_WARNING(lingmoRender).printf("glEGLImageTargetTexture2DOES failed: 0x%04x", error);
        glDeleteTextures(1, &name);
        auto destroyImage = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(
                eglGetProcAddress("eglDestroyImageKHR"));
        if (destroyImage)
            destroyImage(m_context->display(), image);
        return nullptr;
    }

    const PixelFormat format =
            drmFormatHasAlpha(attributes.format) ? PixelFormat::Rgba8888 : PixelFormat::Rgbx8888;
    auto texture =
            std::make_shared<EglImageTexture>(m_context, image, name, attributes.size, format);
    texture->inverted = attributes.yInvert;
    return texture;
}

} // namespace Lingmo::Render

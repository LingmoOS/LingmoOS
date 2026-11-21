// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "render/dmabuf.h"
#include "render/gl_texture.h"
#include <EGL/eglext.h>
#include "base/geometry.h"
struct gbm_device;

namespace Lingmo::Render {
class EglImageTexture : public GlTexture
{
public:
    EglImageTexture(std::shared_ptr<EglContext> context, EGLImageKHR image, uint32_t name,
                    const base::Size &size, PixelFormat format);
    ~EglImageTexture() override;

private:
    EGLImageKHR m_image = EGL_NO_IMAGE_KHR;
};

class EglDmaBufImporter
{
public:
    EglDmaBufImporter(std::shared_ptr<EglContext> context, gbm_device *device);

    const DmaBufFormats &formats() const { return m_dmaBufFormats; }

    dev_t mainDevice() const { return m_dmaBufMainDevice; }

    std::shared_ptr<Texture> import(const DmaBufAttributes &attributes);

    // The EGLImage for `attributes`, or EGL_NO_IMAGE_KHR; the caller destroys
    // it with eglDestroyImageKHR.
    EGLImageKHR createImage(const DmaBufAttributes &attributes) const;

private:
    std::shared_ptr<EglContext> m_context;
    DmaBufFormats m_dmaBufFormats;
    dev_t m_dmaBufMainDevice = 0;
};
} // namespace Lingmo::Render

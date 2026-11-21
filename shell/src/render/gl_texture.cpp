// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/gl_texture.h"

#include <GLES3/gl3.h>
#include <cstdio>
#include <cstring>

#include "core/logging.h"
#include "render/egl_context.h"
#include "base/region.h"

namespace Lingmo::Render {

namespace {
// Repack scratch space kept between uploads without shrinking.
constexpr size_t kScratchKeep = 1 << 20;
} // namespace

GlTexture::GlTexture(uint32_t name, const base::Size &size, PixelFormat format, uint32_t target,
                     std::shared_ptr<EglContext> context)
    : m_context(std::move(context))
    , m_name(name)
    , m_size(size)
    , m_format(format)
    , m_target(target)
{
}

GlTexture::~GlTexture()
{
    if (!m_name)
        return;
    if (m_context) {
        ScopedEglCurrent current(*m_context);
        if (!current.valid())
            LM_FATAL(lingmoRender) << "Cannot make EGL context current for texture retirement";
        glDeleteTextures(1, &m_name);
    } else {
        glDeleteTextures(1, &m_name);
    }
}

bool GlTextureUploader::unpacksRows()
{
    if (!m_unpacksRows) {
        const auto *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
        const auto *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
        int major = 0;
        m_unpacksRows = (version && std::sscanf(version, "OpenGL ES %d", &major) == 1 && major >= 3)
                || (extensions
                    && hasExtension(extensions, "GL_EXT_unpack_subimage"));
    }
    return *m_unpacksRows;
}

void GlTextureUploader::uploadPixels(const base::Size &size, const base::Rect &rect, const uint8_t *pixels,
                                     int stride, bool create)
{
    const int tight = rect.width() * 4;
    const uint8_t *source = pixels + static_cast<size_t>(rect.y()) * stride
            + static_cast<size_t>(rect.x()) * 4;
    // GLES 3 reads the rows where the client left them; GLES 2 needs them packed.
    const bool rows = stride != tight && stride % 4 == 0 && unpacksRows();
    if (stride != tight && !rows) {
        const size_t needed = static_cast<size_t>(tight) * rect.height();
        // A rare large repack would otherwise keep its capacity for the
        // process lifetime; the buffer is sized for what is uploaded now.
        if (m_uploadScratch.capacity() > 2 * needed && m_uploadScratch.capacity() > kScratchKeep)
            std::vector<uint8_t>().swap(m_uploadScratch);
        m_uploadScratch.resize(needed);
        for (int y = 0; y < rect.height(); ++y) {
            std::memcpy(m_uploadScratch.data() + static_cast<size_t>(y) * tight,
                        source + static_cast<size_t>(y) * stride, tight);
        }
        source = m_uploadScratch.data();
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    if (rows)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, stride / 4);
    if (create) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, source);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x(), rect.y(), rect.width(), rect.height(),
                        GL_RGBA, GL_UNSIGNED_BYTE, source);
    }
    if (rows)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

std::unique_ptr<Texture> GlTextureUploader::create(const base::Size &size, const uint8_t *pixels,
                                                   int stride, PixelFormat format)
{
    if (size.isEmpty() || !pixels || stride < size.width() * 4)
        return nullptr;

    GLuint name = 0;
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_2D, name);
    uploadPixels(size, base::Rect(base::Point(), size), pixels, stride, /*create=*/true);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return std::make_unique<GlTexture>(name, size, format, GL_TEXTURE_2D, m_context);
}

bool GlTextureUploader::update(Texture *texture, const uint8_t *pixels, int stride,
                               PixelFormat format)
{
    return update(texture, pixels, stride, format,
                  texture ? base::Rect(base::Point(), texture->size()) : base::Rect());
}

bool GlTextureUploader::update(Texture *texture, const uint8_t *pixels, int stride,
                               PixelFormat format, const base::Region &damage)
{
    auto *glTexture = dynamic_cast<GlTexture *>(texture);
    if (!glTexture || !pixels || glTexture->format() != format)
        return false;
    // EGLImage imports live on GL_TEXTURE_EXTERNAL_OES and are never uploaded into.
    if (glTexture->target() != GL_TEXTURE_2D)
        return false;
    if (stride < glTexture->size().width() * 4)
        return false;
    const base::Region changed = damage.intersected(base::Rect(base::Point(), glTexture->size()));
    if (changed.isEmpty())
        return true;

    glBindTexture(GL_TEXTURE_2D, glTexture->name());
    // A few rectangles go one by one; past that, calls cost more than bytes.
    if (changed.rectCount() <= 16) {
        for (const base::Rect &rect : changed)
            uploadPixels(glTexture->size(), rect, pixels, stride, /*create=*/false);
    } else {
        uploadPixels(glTexture->size(), changed.boundingRect(), pixels, stride,
                     /*create=*/false);
    }
    // The smaller levels still hold the old pixels.
    if (glTexture->mipmapped) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexture->mipmapped = false;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

} // namespace Lingmo::Render

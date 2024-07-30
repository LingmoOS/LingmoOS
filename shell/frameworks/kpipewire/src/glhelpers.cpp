/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "glhelpers.h"
#include <QList>
#include <QVersionNumber>
#include <epoxy/gl.h>
#include <libdrm/drm_fourcc.h>
#include <logging.h>
#include <mutex>

#include <gbm.h>

namespace GLHelpers
{

void initDebugOutputOnce()
{
    auto callback = [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam) {
        Q_UNUSED(source)
        Q_UNUSED(severity)
        Q_UNUSED(userParam)
        while (length && std::isspace(message[length - 1])) {
            --length;
        }

        switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            qCWarning(PIPEWIRE_LOGGING, "%#x: %.*s", id, length, message);
            break;

        case GL_DEBUG_TYPE_OTHER:
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        case GL_DEBUG_TYPE_PORTABILITY:
        case GL_DEBUG_TYPE_PERFORMANCE:
        default:
            qCDebug(PIPEWIRE_LOGGING, "%#x: %.*s", id, length, message);
            break;
        }
    };
    glDebugMessageCallback(callback, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
}

static std::once_flag initDebugOnce;
void initDebugOutput()
{
    if (!PIPEWIRE_LOGGING().isDebugEnabled()) {
        return;
    }

    if (!eglGetCurrentDisplay()) {
        // Epoxy gets very confused and it will crash
        return;
    }

    std::call_once(initDebugOnce, initDebugOutputOnce);
}

#define ENUM_STRING(x) case x: return #x;

QByteArray formatGLError(GLenum err)
{
    switch (err) {
    ENUM_STRING(GL_NO_ERROR)
    ENUM_STRING(GL_INVALID_ENUM)
    ENUM_STRING(GL_INVALID_VALUE)
    ENUM_STRING(GL_INVALID_OPERATION)
    ENUM_STRING(GL_STACK_OVERFLOW)
    ENUM_STRING(GL_STACK_UNDERFLOW)
    ENUM_STRING(GL_OUT_OF_MEMORY)
    default:
        return QByteArray("0x") + QByteArray::number(err, 16);
    }
}

QByteArray formatEGLError(GLenum err)
{
    switch (err) {
        ENUM_STRING(EGL_SUCCESS)
        ENUM_STRING(EGL_BAD_DISPLAY)
        ENUM_STRING(EGL_BAD_CONTEXT)
        ENUM_STRING(EGL_BAD_PARAMETER)
        ENUM_STRING(EGL_BAD_MATCH)
        ENUM_STRING(EGL_BAD_ACCESS)
        ENUM_STRING(EGL_BAD_ALLOC)
        ENUM_STRING(EGL_BAD_CONFIG)
    default:
        return QByteArray("0x") + QByteArray::number(err, 16);
    }
}

EGLImage createImage(EGLDisplay display, const DmaBufAttributes &dmabufAttribs, uint32_t format, const QSize &size, gbm_device *gbmDevice)
{
    Q_ASSERT(!size.isEmpty());
    gbm_bo *imported = nullptr;
    if (gbmDevice) {
        gbm_import_fd_data importInfo = {static_cast<int>(dmabufAttribs.planes[0].fd),
                                         static_cast<uint32_t>(size.width()),
                                         static_cast<uint32_t>(size.height()),
                                         static_cast<uint32_t>(dmabufAttribs.planes[0].stride),
                                         GBM_BO_FORMAT_ARGB8888};
        imported = gbm_bo_import(gbmDevice, GBM_BO_IMPORT_FD, &importInfo, GBM_BO_USE_SCANOUT);
        if (!imported) {
            qCWarning(PIPEWIRE_LOGGING) << "Failed to process buffer: Cannot import passed GBM fd - " << strerror(errno);
            return EGL_NO_IMAGE_KHR;
        }
    }

    const bool hasModifiers = dmabufAttribs.modifier != DRM_FORMAT_MOD_INVALID;

    static int lastSize = 37; // 37 is what we get on a normal system with working dmabuf, it's just a reasonable default
    QList<EGLint> attribs;
    attribs.reserve(lastSize);
    attribs << EGL_WIDTH << size.width() << EGL_HEIGHT << size.height() << EGL_LINUX_DRM_FOURCC_EXT << EGLint(format)

            << EGL_DMA_BUF_PLANE0_FD_EXT << dmabufAttribs.planes[0].fd << EGL_DMA_BUF_PLANE0_OFFSET_EXT << EGLint(dmabufAttribs.planes[0].offset)
            << EGL_DMA_BUF_PLANE0_PITCH_EXT << EGLint(dmabufAttribs.planes[0].stride);

    if (hasModifiers) {
        attribs << EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT << EGLint(dmabufAttribs.modifier & 0xffffffff) << EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT
                << EGLint(dmabufAttribs.modifier >> 32);
    }

    if (dmabufAttribs.planes.count() > 1) {
        attribs << EGL_DMA_BUF_PLANE1_FD_EXT << dmabufAttribs.planes[1].fd << EGL_DMA_BUF_PLANE1_OFFSET_EXT << EGLint(dmabufAttribs.planes[1].offset)
                << EGL_DMA_BUF_PLANE1_PITCH_EXT << EGLint(dmabufAttribs.planes[1].stride);

        if (hasModifiers) {
            attribs << EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT << EGLint(dmabufAttribs.modifier & 0xffffffff) << EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT
                    << EGLint(dmabufAttribs.modifier >> 32);
        }
    }

    if (dmabufAttribs.planes.count() > 2) {
        attribs << EGL_DMA_BUF_PLANE2_FD_EXT << dmabufAttribs.planes[2].fd << EGL_DMA_BUF_PLANE2_OFFSET_EXT << EGLint(dmabufAttribs.planes[2].offset)
                << EGL_DMA_BUF_PLANE2_PITCH_EXT << EGLint(dmabufAttribs.planes[2].stride);

        if (hasModifiers) {
            attribs << EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT << EGLint(dmabufAttribs.modifier & 0xffffffff) << EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT
                    << EGLint(dmabufAttribs.modifier >> 32);
        }
    }

    if (dmabufAttribs.planes.count() > 3) {
        attribs << EGL_DMA_BUF_PLANE3_FD_EXT << dmabufAttribs.planes[3].fd << EGL_DMA_BUF_PLANE3_OFFSET_EXT << EGLint(dmabufAttribs.planes[3].offset)
                << EGL_DMA_BUF_PLANE3_PITCH_EXT << EGLint(dmabufAttribs.planes[3].stride);

        if (hasModifiers) {
            attribs << EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT << EGLint(dmabufAttribs.modifier & 0xffffffff) << EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT
                    << EGLint(dmabufAttribs.modifier >> 32);
        }
    }

    attribs << EGL_NONE;
    lastSize = attribs.size();

    static auto eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");

    /* MESA says:
     * "If <target> is EGL_LINUX_DMA_BUF_EXT, <dpy> must be a valid display,
     *  <ctx> must be EGL_NO_CONTEXT..."
     */
    EGLImage ret = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, imported, attribs.data());
    if (ret == EGL_NO_IMAGE_KHR) {
        qCWarning(PIPEWIRE_LOGGING) << "invalid image" << GLHelpers::formatEGLError(eglGetError());
    }
    if (imported) {
        gbm_bo_destroy(imported);
    }
    return ret;
}
}

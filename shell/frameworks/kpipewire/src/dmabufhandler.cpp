// SPDX-FileCopyrightText: 2022 Aleix Pol i Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: Apache-2.0

#include "dmabufhandler.h"
#include "glhelpers.h"
#include <QGuiApplication>
#include <fcntl.h>
#include <gbm.h>
#include <logging_dmabuf.h>
#include <qpa/qplatformnativeinterface.h>
#include <unistd.h>
#include <xf86drm.h>

static QByteArray fetchRenderNode()
{
    int max_devices = drmGetDevices2(0, nullptr, 0);
    if (max_devices <= 0) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "drmGetDevices2() has not found any devices (errno=" << -max_devices << ")";
        return "/dev/dri/renderD128";
    }

    std::vector<drmDevicePtr> devices(max_devices);
    int ret = drmGetDevices2(0, devices.data(), max_devices);
    if (ret < 0) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "drmGetDevices2() returned an error " << ret;
        return "/dev/dri/renderD128";
    }

    QByteArray render_node;

    for (const drmDevicePtr &device : devices) {
        if (device->available_nodes & (1 << DRM_NODE_RENDER)) {
            render_node = device->nodes[DRM_NODE_RENDER];
            break;
        }
    }

    drmFreeDevices(devices.data(), ret);
    return render_node;
}

DmaBufHandler::DmaBufHandler()
{
}

DmaBufHandler::~DmaBufHandler()
{
    if (m_drmFd) {
        close(m_drmFd);
    }
}

void DmaBufHandler::setupEgl()
{
    if (m_eglInitialized) {
        return;
    }

    m_egl.display = static_cast<EGLDisplay>(QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("egldisplay"));

    // Use eglGetPlatformDisplayEXT() to get the display pointer
    // if the implementation supports it.
    if (!epoxy_has_egl_extension(m_egl.display, "EGL_EXT_platform_base") || !epoxy_has_egl_extension(m_egl.display, "EGL_MESA_platform_gbm")) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "One of required EGL extensions is missing";
        return;
    }

    if (m_egl.display == EGL_NO_DISPLAY) {
        m_egl.display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR, (void *)EGL_DEFAULT_DISPLAY, nullptr);
    }
    if (m_egl.display == EGL_NO_DISPLAY) {
        const QByteArray renderNode = fetchRenderNode();
        m_drmFd = open(renderNode.constData(), O_RDWR);

        if (m_drmFd < 0) {
            qCWarning(PIPEWIREDMABUF_LOGGING) << "Failed to open drm render node" << renderNode << "with error: " << strerror(errno);
            return;
        }

        m_gbmDevice = gbm_create_device(m_drmFd);

        if (!m_gbmDevice) {
            qCWarning(PIPEWIREDMABUF_LOGGING) << "Cannot create GBM device: " << strerror(errno);
            return;
        }
        m_egl.display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA, m_gbmDevice, nullptr);
    }

    if (m_egl.display == EGL_NO_DISPLAY) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "Error during obtaining EGL display: " << GLHelpers::formatGLError(eglGetError());
        return;
    }

    EGLint major, minor;
    if (eglInitialize(m_egl.display, &major, &minor) == EGL_FALSE) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "Error during eglInitialize: " << GLHelpers::formatGLError(eglGetError());
        return;
    }

    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "bind OpenGL API failed";
        return;
    }

    EGLConfig configs;
    auto createConfig = [&] {
        static const EGLint configAttribs[] = {
            EGL_SURFACE_TYPE,
            EGL_WINDOW_BIT,
            EGL_RED_SIZE,
            8,
            EGL_GREEN_SIZE,
            8,
            EGL_BLUE_SIZE,
            8,
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_BIT,
            EGL_CONFIG_CAVEAT,
            EGL_NONE,
            EGL_NONE,
        };

        EGLint count = 333;
        if (eglChooseConfig(m_egl.display, configAttribs, &configs, 1, &count) == EGL_FALSE) {
            qCWarning(PIPEWIREDMABUF_LOGGING) << "choose config failed";
            return false;
        }
        // if (count != 1) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "eglChooseConfig returned this many configs:" << count;
        //     return false;
        // }
        return true;
    };

    bool b = createConfig();
    static const EGLint configAttribs[] = {EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE, EGL_NONE};
    Q_ASSERT(configs);
    m_egl.context = eglCreateContext(m_egl.display, b ? configs : EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, configAttribs);

    Q_ASSERT(b);
    Q_ASSERT(m_egl.context);
    if (m_egl.context == EGL_NO_CONTEXT) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "Couldn't create EGL context: " << GLHelpers::formatEGLError(eglGetError());
        return;
    }

    qCDebug(PIPEWIREDMABUF_LOGGING) << "Egl initialization succeeded";
    qCDebug(PIPEWIREDMABUF_LOGGING) << QStringLiteral("EGL version: %1.%2").arg(major).arg(minor);

    m_eglInitialized = true;
}

GLenum closestGLType(const QImage &image)
{
    switch (image.format()) {
    case QImage::Format_RGB888:
        return GL_RGB;
    case QImage::Format_BGR888:
        return GL_BGR;
    case QImage::Format_RGB32:
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
        return GL_RGBA;
    default:
        qDebug() << "cannot convert QImage format to GLType" << image.format();
        return GL_RGBA;
    }
}

bool DmaBufHandler::downloadFrame(QImage &qimage, const PipeWireFrame &frame)
{
    Q_ASSERT(frame.dmabuf);
    const QSize streamSize = {frame.dmabuf->width, frame.dmabuf->height};
    Q_ASSERT(qimage.size() == streamSize);
    setupEgl();
    if (!m_eglInitialized) {
        return false;
    }

    if (!eglMakeCurrent(m_egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_egl.context)) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "Failed to make context current" << GLHelpers::formatEGLError(eglGetError());
        return false;
    }
    EGLImageKHR image =
        GLHelpers::createImage(m_egl.display, *frame.dmabuf, PipeWireSourceStream::spaVideoFormatToDrmFormat(frame.format), qimage.size(), m_gbmDevice);

    if (image == EGL_NO_IMAGE_KHR) {
        qCWarning(PIPEWIREDMABUF_LOGGING) << "Failed to record frame: Error creating EGLImageKHR - " << GLHelpers::formatEGLError(eglGetError());
        return false;
    }

    GLHelpers::initDebugOutput();
    // create GL 2D texture for framebuffer
    GLuint texture;
    GLuint fbo;
    glGenTextures(1, &texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, texture);

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &texture);
        eglDestroyImageKHR(m_egl.display, image);
        return false;
    }

    glReadPixels(0, 0, frame.dmabuf->width, frame.dmabuf->height, closestGLType(qimage), GL_UNSIGNED_BYTE, qimage.bits());

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);
    eglDestroyImageKHR(m_egl.display, image);
    return true;
}

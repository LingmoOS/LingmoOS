/*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include <qpa/qplatformnativeinterface.h>
#include <QGuiApplication>
#include <QSGImageNode>
#include <QSGTexture>
#include <QOpenGLContext>
#include <QQuickWindow>

#include <libdrm/drm_fourcc.h>

#include "kywlcom-window-thumbnail-item.h"
#include "discard-egl-pixmap-runnable.h"

KywlcomWindowThumbnailItem::KywlcomWindowThumbnailItem(QQuickItem *parent) : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    if(!m_display) {
        QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
        if (!native) {
            return;
        }

        m_display = reinterpret_cast<wl_display *>(
                native->nativeResourceForIntegration(QByteArrayLiteral("wl_display")));
        if (!m_display) {
            qWarning("Failed to get Wayland display.");
            return;
        }
    }
    connect(this, &QQuickItem::visibleChanged, this, [this]() {
        active(isVisible());
    });
}

KywlcomWindowThumbnailItem::~KywlcomWindowThumbnailItem()
{
}

QString KywlcomWindowThumbnailItem::uuid()
{
    return m_uuid;
}

void KywlcomWindowThumbnailItem::setUuid(QString &uuid)
{
    if (m_uuid != uuid) {
        m_uuid = uuid;
        active(!uuid.isEmpty());
        Q_EMIT uuidChanged();
    }
}

void KywlcomWindowThumbnailItem::BufferImportDmabuf()
{
    if(!m_thumbnail) {
        return;
    }
    Thumbnail *thum = qobject_cast<Thumbnail *>(sender());
    if (thum->flags() & Thumbnail::BufferFlag::Dmabuf) {
        createEglImage(thum);
    } else {
        imageFromMemfd(thum);
        if (map) {
            const QImage::Format qtFormat = thum->stride() / thum->size().width() == 3
                                            ? QImage::Format_RGB888
                                            : QImage::Format_ARGB32;

            QImage img(map, thum->size().width(), thum->size().height(), thum->stride(), qtFormat);
            m_qImage = img.copy();
        }
    }

    m_format = m_thumbnail->format();
    m_thumFlags = m_thumbnail->flags();
    if(window() && window()->isVisible()) {
        update();
    }
}
#define ADD_ATTRIB(name, value)                                                                    \
    do {                                                                                           \
        attribs[num_attribs++] = (name);                                                           \
        attribs[num_attribs++] = (value);                                                          \
        attribs[num_attribs] = EGL_NONE;                                                           \
    } while (0)

void KywlcomWindowThumbnailItem::createEglImage(Thumbnail *thumbnail)
{
    EGLDisplay display = EGL_NO_DISPLAY;
    display = static_cast<EGLDisplay>(
            QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("egldisplay"));

    if (display == EGL_NO_DISPLAY) {
        qWarning() << "egl get display failed ";
        return;
    }

    if (m_image) {
        if (m_thumFlags & Thumbnail::BufferFlag::Reused)
            return;
        static auto eglDestroyImageKHR =
                (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
        eglDestroyImageKHR(display, m_image);
    }

    EGLint attribs[20] = { EGL_NONE };
    int num_attribs = 0;

    ADD_ATTRIB(EGL_WIDTH, thumbnail->size().width());
    ADD_ATTRIB(EGL_HEIGHT, thumbnail->size().height());
    ADD_ATTRIB(EGL_LINUX_DRM_FOURCC_EXT, thumbnail->format());
    ADD_ATTRIB(EGL_DMA_BUF_PLANE0_FD_EXT, thumbnail->fd());
    ADD_ATTRIB(EGL_DMA_BUF_PLANE0_OFFSET_EXT, thumbnail->offset());
    ADD_ATTRIB(EGL_DMA_BUF_PLANE0_PITCH_EXT, thumbnail->stride());
    if (thumbnail->modifier() != DRM_FORMAT_MOD_INVALID) {
        ADD_ATTRIB(EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, thumbnail->modifier() & 0xFFFFFFFF);
        ADD_ATTRIB(EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, thumbnail->modifier() >> 32);
    }
    ADD_ATTRIB(EGL_IMAGE_PRESERVED_KHR, EGL_TRUE);

    m_format = thumbnail->format();

    static auto eglCreateImageKHR =
            (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    Q_ASSERT(eglCreateImageKHR);
    m_image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, attribs);
    if (m_image == EGL_NO_IMAGE_KHR) {
        qWarning() << "invalid eglCreateImageKHR" << glGetError();
    }
}

void KywlcomWindowThumbnailItem::imageFromMemfd(Thumbnail *thumbnail)
{
    if (m_thumFlags & Thumbnail::BufferFlag::Reused)
        return;

    dataSize = thumbnail->size().height() * thumbnail->stride() + thumbnail->offset();
    map = static_cast<uint8_t *>(mmap(nullptr, dataSize, PROT_READ, MAP_PRIVATE, thumbnail->fd(), 0));

    if (map == MAP_FAILED) {
        qWarning() << "Failed to mmap the memory: " << strerror(errno);
        return;
    }
}

QSGNode *KywlcomWindowThumbnailItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    QSGTexture *texture = nullptr;

    if (m_thumFlags & Thumbnail::BufferFlag::Dmabuf) {
        if (m_image == EGL_NO_IMAGE_KHR) {
            QImage q_image(width(), height(), QImage::Format_ARGB32_Premultiplied);
            q_image.fill(Qt::transparent);
            texture = window()->createTextureFromImage(q_image, QQuickWindow::TextureIsOpaque);
        } else {
            QOpenGLContext *context = window()->openglContext();
            if (!context || !context->isValid()) {
                qWarning() << "OpenGL context is not valid.";
                return nullptr;
            }

            if (!m_texture) {
                m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
                bool created = m_texture->create();
                Q_ASSERT(created);
            }

            static auto s_glEGLImageTargetTexture2DOES =
                    (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress(
                            "glEGLImageTargetTexture2DOES");
            if (!s_glEGLImageTargetTexture2DOES) {
                qWarning() << "glEGLImageTargetTexture2DOES is not available" << window();
                return nullptr;
            }
            m_texture->bind();
            s_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)m_image);

            m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
            m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
            m_texture->release();
            m_texture->setSize(boundingRect().width(), boundingRect().height());

            int textureId = m_texture->textureId();

            QQuickWindow::CreateTextureOption textureOption =
                    m_format == DRM_FORMAT_ARGB8888 ? QQuickWindow::TextureHasAlphaChannel
                                                    : QQuickWindow::TextureIsOpaque;
            texture = window()->createTextureFromNativeObject(QQuickWindow::NativeObjectTexture,
                                                              &textureId, 0 /*a vulkan thing?*/,
                                                             m_thumbnail->size(), textureOption);
        }
    } else {
        if (m_qImage.isNull()) {
            qWarning() << "m_qImage.isNull() " << strerror(errno);
            QImage errorImage(200, 200, QImage::Format_ARGB32_Premultiplied);
            errorImage.fill(Qt::red);
            m_qImage = errorImage;
        }
        texture = window()->createTextureFromImage(m_qImage, QQuickWindow::TextureIsOpaque);
    }
    if (m_needsRecreateTexture) {
        delete oldNode;
        oldNode = nullptr;
        m_needsRecreateTexture = false;
    }

    QSGImageNode *textureNode = static_cast<QSGImageNode *>(oldNode);
    if (!textureNode) {
        textureNode = window()->createImageNode();
        textureNode->setOwnsTexture(true);
        textureNode->setFiltering(QSGTexture::Linear);
    }
    textureNode->setTexture(texture);
    QSizeF scaleSize = textureNode->texture()->textureSize();
    if(m_fixHeight) {
        scaleSize.setHeight(boundingRect().height());
    }
    if(m_fixWidth) {
        scaleSize.setWidth(boundingRect().width());
    }
    if(!m_fixHeight && !m_fixWidth) {
        scaleSize = boundingRect().size();
    }
    const QSizeF size(texture->textureSize().scaled(scaleSize.toSize(), Qt::KeepAspectRatio));
    if(size != m_paintedSize) {
        m_paintedSize = size;
        Q_EMIT paintedSizeChanged();
    }
    QRect rect({ 0, 0 }, size.toSize());
    rect.moveCenter(boundingRect().toRect().center());
    textureNode->setRect(rect);

    return textureNode;
}

void KywlcomWindowThumbnailItem::releaseResources()
{
    if (window()) {
        window()->scheduleRenderJob(new DiscardEglPixmapRunnable(m_image, m_texture), QQuickWindow::NoStage);
        m_image = EGL_NO_IMAGE_KHR;
    }
}

void KywlcomWindowThumbnailItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data)
{
    switch (change) {
        case ItemSceneChange:
            m_needsRecreateTexture = true;
            releaseResources();
            break;
        default:
            break;
    }
    return QQuickItem::itemChange(change, data);
}

void KywlcomWindowThumbnailItem::active(bool active)
{
    if(active && !m_uuid.isEmpty()) {
        if(m_currentUuid == m_uuid || !isVisible()) {
            return;
        }
        if(!m_context) {
            m_context = new Context(m_display, Context::Capability::Thumbnail, this);
            m_context->start();
        }
        if(!m_thumbnail) {
            m_thumbnail = new Thumbnail(this);
            connect(m_thumbnail, &Thumbnail::bufferUpdate, this, &KywlcomWindowThumbnailItem::BufferImportDmabuf);
            connect(m_thumbnail, &Thumbnail::deleted, this, &KywlcomWindowThumbnailItem::thumbnailIsDeleted);

        }
        m_context->thumbnail_init(m_thumbnail, Thumbnail::Type::Toplevel, m_uuid, "", "true");
        m_active = true;
        m_currentUuid = m_uuid;
        m_thumbnailIsDeleted = false;
    } else {
        if(m_active && !m_thumbnailIsDeleted) {
            if (map) {
                munmap(map, dataSize);
                map = nullptr;
            }
            m_thumbnail->destory();
        }
    }
}

qreal KywlcomWindowThumbnailItem::paintedWidth() const
{
    return m_paintedSize.width();
}

qreal KywlcomWindowThumbnailItem::paintedHeight() const
{
    return m_paintedSize.height();
}

bool KywlcomWindowThumbnailItem::fixHeight() const
{
    return m_fixHeight;
}

void KywlcomWindowThumbnailItem::setFixHeight(bool fixHeight)
{
    if(m_fixHeight == fixHeight) {
        return;
    }
    m_fixHeight = fixHeight;
    Q_EMIT fixHeightChanged();
}

bool KywlcomWindowThumbnailItem::fixWidth() const
{
    return m_fixWidth;
}

void KywlcomWindowThumbnailItem::setFixWidth(bool fixWidth)
{
    if(m_fixWidth == fixWidth) {
        return;
    }
    m_fixWidth = fixWidth;
    Q_EMIT fixWidthChanged();
}

void KywlcomWindowThumbnailItem::componentComplete()
{
    active(isVisible());
    QQuickItem::componentComplete();
}

void KywlcomWindowThumbnailItem::thumbnailIsDeleted()
{
    m_thumbnailIsDeleted = true;

    m_thumbnail = nullptr;
    m_active = false;
    m_currentUuid = "";
}

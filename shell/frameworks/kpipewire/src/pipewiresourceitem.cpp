/*
    Render a PipeWire stream into a QtQuick scene as a standard Item
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pipewiresourceitem.h"
#include "glhelpers.h"
#include "logging.h"
#include "pipewiresourcestream.h"
#include "pwhelpers.h"

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QPainter>
#include <QQuickWindow>
#include <QRunnable>
#include <QSGImageNode>
#include <QSocketNotifier>
#include <QThread>
#include <qpa/qplatformnativeinterface.h>

#include <EGL/eglext.h>
#include <fcntl.h>
#include <libdrm/drm_fourcc.h>
#include <unistd.h>

static void pwInit()
{
    pw_init(nullptr, nullptr);
}
Q_COREAPP_STARTUP_FUNCTION(pwInit);

class PipeWireSourceItemPrivate
{
public:
    uint m_nodeId = 0;
    std::optional<uint> m_fd;
    std::function<QSGTexture *()> m_createNextTexture;
    std::unique_ptr<PipeWireSourceStream> m_stream;
    std::unique_ptr<QOpenGLTexture> m_texture;

    EGLImage m_image = nullptr;
    bool m_needsRecreateTexture = false;
    bool m_allowDmaBuf = true;
    bool m_ready = false;

    struct {
        QImage texture;
        std::optional<QPoint> position;
        QPoint hotspot;
        bool dirty = false;
    } m_cursor;
    std::optional<QRegion> m_damage;
};

class DiscardEglPixmapRunnable : public QRunnable
{
public:
    DiscardEglPixmapRunnable(EGLImageKHR image, QOpenGLTexture *texture)
        : m_image(image)
        , m_texture(texture)
    {
    }

    void run() override
    {
        if (m_image != EGL_NO_IMAGE_KHR) {
            eglDestroyImageKHR(eglGetCurrentDisplay(), m_image);
        }

        delete m_texture;
    }

private:
    const EGLImageKHR m_image;
    QOpenGLTexture *m_texture;
};

PipeWireSourceItem::PipeWireSourceItem(QQuickItem *parent)
    : QQuickItem(parent)
    , d(new PipeWireSourceItemPrivate)
{
    setFlag(ItemHasContents, true);
}

PipeWireSourceItem::~PipeWireSourceItem()
{
    if (d->m_fd) {
        close(*d->m_fd);
    }
}

void PipeWireSourceItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data)
{
    switch (change) {
    case ItemVisibleHasChanged:
        if (!isVisible()) {
            setReady(false);
        }
        if (d->m_stream) {
            d->m_stream->setActive(isVisible());
        }
        break;
    case ItemSceneChange:
        d->m_needsRecreateTexture = true;
        releaseResources();
        break;
    default:
        break;
    }

    QQuickItem::itemChange(change, data);
}

void PipeWireSourceItem::releaseResources()
{
    if (window() && (d->m_image || d->m_texture)) {
        window()->scheduleRenderJob(new DiscardEglPixmapRunnable(d->m_image, d->m_texture.release()), QQuickWindow::NoStage);
        d->m_image = EGL_NO_IMAGE_KHR;
    }
}

void PipeWireSourceItem::setFd(uint fd)
{
    if (fd == d->m_fd)
        return;

    if (d->m_fd) {
        close(*d->m_fd);
    }
    d->m_fd = fd;
    refresh();
    Q_EMIT fdChanged(fd);
}

void PipeWireSourceItem::resetFd()
{
    if (!d->m_fd.has_value()) {
        return;
    }

    setReady(false);
    close(*d->m_fd);
    d->m_fd.reset();
    d->m_stream.reset(nullptr);
    d->m_createNextTexture = [] {
        return nullptr;
    };
    Q_EMIT streamSizeChanged();
}

void PipeWireSourceItem::refresh()
{
    setReady(false);

    if (!isComponentComplete()) {
        return;
    }

    if (d->m_nodeId == 0) {
        releaseResources();
        d->m_stream.reset(nullptr);
        Q_EMIT streamSizeChanged();

        d->m_createNextTexture = [] {
            return nullptr;
        };
    } else {
        d->m_stream.reset(new PipeWireSourceStream(this));
        d->m_stream->setAllowDmaBuf(d->m_allowDmaBuf);
        Q_EMIT streamSizeChanged();
        connect(d->m_stream.get(), &PipeWireSourceStream::streamParametersChanged, this, &PipeWireSourceItem::streamSizeChanged);
        connect(d->m_stream.get(), &PipeWireSourceStream::streamParametersChanged, this, &PipeWireSourceItem::usingDmaBufChanged);

        d->m_stream->createStream(d->m_nodeId, d->m_fd.value_or(0));
        if (!d->m_stream->error().isEmpty()) {
            d->m_stream.reset(nullptr);
            d->m_nodeId = 0;
            return;
        }
        d->m_stream->setActive(isVisible());

        connect(d->m_stream.get(), &PipeWireSourceStream::frameReceived, this, &PipeWireSourceItem::processFrame);
        connect(d->m_stream.get(), &PipeWireSourceStream::stateChanged, this, &PipeWireSourceItem::stateChanged);
    }
    Q_EMIT stateChanged();
}

void PipeWireSourceItem::setNodeId(uint nodeId)
{
    if (nodeId == d->m_nodeId)
        return;

    d->m_nodeId = nodeId;
    refresh();
    Q_EMIT nodeIdChanged(nodeId);
}

class PipeWireRenderNode : public QSGNode
{
public:
    QSGImageNode *screenNode(QQuickWindow *window)
    {
        if (!m_screenNode) {
            m_screenNode = window->createImageNode();
            appendChildNode(m_screenNode);
        }
        return m_screenNode;
    }
    QSGImageNode *cursorNode(QQuickWindow *window)
    {
        if (!m_cursorNode) {
            m_cursorNode = window->createImageNode();
            appendChildNode(m_cursorNode);
        }
        return m_cursorNode;
    }

    QSGImageNode *damageNode(QQuickWindow *window)
    {
        if (!m_damageNode) {
            m_damageNode = window->createImageNode();
            appendChildNode(m_damageNode);
        }
        return m_damageNode;
    }

    void discardCursor()
    {
        if (m_cursorNode) {
            removeChildNode(m_cursorNode);
            delete m_cursorNode;
            m_cursorNode = nullptr;
        }
    }

    void discardDamage()
    {
        if (m_damageNode) {
            removeChildNode(m_damageNode);
            delete m_damageNode;
            m_damageNode = nullptr;
        }
    }

private:
    QSGImageNode *m_screenNode = nullptr;
    QSGImageNode *m_cursorNode = nullptr;
    QSGImageNode *m_damageNode = nullptr;
};

QSGNode *PipeWireSourceItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    if (Q_UNLIKELY(!d->m_createNextTexture)) {
        return node;
    }

    auto texture = d->m_createNextTexture();
    if (!texture) {
        delete node;
        return nullptr;
    }

    auto pwNode = static_cast<PipeWireRenderNode *>(node);
    if (!pwNode) {
        pwNode = new PipeWireRenderNode;
    }

    QSGImageNode *screenNode = pwNode->screenNode(window());
    screenNode->setTexture(texture);
    screenNode->setOwnsTexture(true);

    const auto br = boundingRect().toRect();
    QRect rect({0, 0}, texture->textureSize().scaled(br.size(), Qt::KeepAspectRatio));
    rect.moveCenter(br.center());
    screenNode->setRect(rect);

    if (!d->m_cursor.position.has_value() || d->m_cursor.texture.isNull()) {
        pwNode->discardCursor();
    } else {
        QSGImageNode *cursorNode = pwNode->cursorNode(window());
        if (d->m_cursor.dirty || !cursorNode->texture()) {
            cursorNode->setTexture(window()->createTextureFromImage(d->m_cursor.texture));
            cursorNode->setOwnsTexture(true);
            d->m_cursor.dirty = false;
        }
        const qreal scale = qreal(rect.width()) / texture->textureSize().width();
        cursorNode->setRect(QRectF{rect.topLeft() + (d->m_cursor.position.value() * scale), d->m_cursor.texture.size() * scale});
        Q_ASSERT(cursorNode->texture());
    }

    if (!d->m_damage || d->m_damage->isEmpty()) {
        pwNode->discardDamage();
    } else {
        auto *damageNode = pwNode->damageNode(window());
        QImage damageImage(texture->textureSize(), QImage::Format_RGBA64_Premultiplied);
        damageImage.fill(Qt::transparent);
        QPainter p(&damageImage);
        p.setBrush(Qt::red);
        for (auto rect : *d->m_damage) {
            p.drawRect(rect);
        }
        damageNode->setTexture(window()->createTextureFromImage(damageImage));
        damageNode->setOwnsTexture(true);
        damageNode->setRect(rect);
        Q_ASSERT(damageNode->texture());
    }
    return pwNode;
}

QString PipeWireSourceItem::error() const
{
    return d->m_stream->error();
}

void PipeWireSourceItem::processFrame(const PipeWireFrame &frame)
{
    d->m_damage = frame.damage;

    if (frame.cursor) {
        d->m_cursor.position = frame.cursor->position;
        d->m_cursor.hotspot = frame.cursor->hotspot;
        if (!frame.cursor->texture.isNull()) {
            d->m_cursor.dirty = true;
            d->m_cursor.texture = frame.cursor->texture;
        }
    } else {
        d->m_cursor.position = std::nullopt;
        d->m_cursor.hotspot = {};
    }

    if (frame.dmabuf) {
        updateTextureDmaBuf(*frame.dmabuf, frame.format);
    } else if (frame.dataFrame) {
        updateTextureImage(frame.dataFrame);
    }

    if (window() && window()->isVisible()) {
        update();
    }
}

void PipeWireSourceItem::updateTextureDmaBuf(const DmaBufAttributes &attribs, spa_video_format format)
{
    if (!window()) {
        qCWarning(PIPEWIRE_LOGGING) << "Window not available" << this;
        return;
    }

    const auto openglContext = static_cast<QOpenGLContext *>(window()->rendererInterface()->getResource(window(), QSGRendererInterface::OpenGLContextResource));
    if (!openglContext || !d->m_stream) {
        qCWarning(PIPEWIRE_LOGGING) << "need a window and a context" << window();
        return;
    }

    d->m_createNextTexture = [this, format, attribs]() -> QSGTexture * {
        const EGLDisplay display = static_cast<EGLDisplay>(QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("egldisplay"));
        if (d->m_image) {
            eglDestroyImageKHR(display, d->m_image);
        }
        const auto size = d->m_stream->size();
        d->m_image = GLHelpers::createImage(display, attribs, PipeWireSourceStream::spaVideoFormatToDrmFormat(format), size, nullptr);
        if (d->m_image == EGL_NO_IMAGE_KHR) {
            d->m_stream->renegotiateModifierFailed(format, attribs.modifier);
            return nullptr;
        }
        if (!d->m_texture) {
            d->m_texture.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
            bool created = d->m_texture->create();
            Q_ASSERT(created);
        }

        GLHelpers::initDebugOutput();
        d->m_texture->bind();

        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)d->m_image);

        d->m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
        d->m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        d->m_texture->release();
        d->m_texture->setSize(size.width(), size.height());

        int textureId = d->m_texture->textureId();
        QQuickWindow::CreateTextureOption textureOption =
            format == SPA_VIDEO_FORMAT_ARGB || format == SPA_VIDEO_FORMAT_BGRA ? QQuickWindow::TextureHasAlphaChannel : QQuickWindow::TextureIsOpaque;
        return QNativeInterface::QSGOpenGLTexture::fromNative(textureId, window(), size, textureOption);
    };

    setReady(true);
}

void PipeWireSourceItem::updateTextureImage(const std::shared_ptr<PipeWireFrameData> &data)
{
    if (!window()) {
        qCWarning(PIPEWIRE_LOGGING) << "pass";
        return;
    }

    d->m_createNextTexture = [this, data] {
        return window()->createTextureFromImage(data->toImage(), QQuickWindow::TextureIsOpaque);
    };

    setReady(true);
}

void PipeWireSourceItem::componentComplete()
{
    QQuickItem::componentComplete();
    if (d->m_nodeId != 0) {
        refresh();
    }
}

PipeWireSourceItem::StreamState PipeWireSourceItem::state() const
{
    if (!d->m_stream) {
        return StreamState::Unconnected;
    }
    switch (d->m_stream->state()) {
    case PW_STREAM_STATE_ERROR:
        return StreamState::Error;
    case PW_STREAM_STATE_UNCONNECTED:
        return StreamState::Unconnected;
    case PW_STREAM_STATE_CONNECTING:
        return StreamState::Connecting;
    case PW_STREAM_STATE_PAUSED:
        return StreamState::Paused;
    case PW_STREAM_STATE_STREAMING:
        return StreamState::Streaming;
    default:
        return StreamState::Error;
    }
}

uint PipeWireSourceItem::fd() const
{
    return d->m_fd.value_or(0);
}

uint PipeWireSourceItem::nodeId() const
{
    return d->m_nodeId;
}

QSize PipeWireSourceItem::streamSize() const
{
    if (!d->m_stream) {
        return QSize();
    }
    return d->m_stream->size();
}

bool PipeWireSourceItem::usingDmaBuf() const
{
    return d->m_stream && d->m_stream->usingDmaBuf();
}

bool PipeWireSourceItem::allowDmaBuf() const
{
    return d->m_stream && d->m_stream->allowDmaBuf();
}

void PipeWireSourceItem::setAllowDmaBuf(bool allowed)
{
    d->m_allowDmaBuf = allowed;
    if (d->m_stream) {
        d->m_stream->setAllowDmaBuf(allowed);
    }
}

void PipeWireSourceItem::setReady(bool ready)
{
    if (d->m_ready != ready) {
        d->m_ready = ready;
        Q_EMIT readyChanged();
    }
}

bool PipeWireSourceItem::isReady() const
{
    return d->m_ready;
}

#include "moc_pipewiresourceitem.cpp"

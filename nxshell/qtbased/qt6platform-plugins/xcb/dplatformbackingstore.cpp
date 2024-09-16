// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplatformbackingstore.h"
#include "dplatformwindowhook.h"
#include "vtablehook.h"
#ifdef Q_OS_LINUX
#include "dxcbwmsupport.h"
#endif

#include "qxcbbackingstore.h"

#include <QDebug>
#include <QPainter>
#include <QEvent>
#include <QWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainterPathStroker>
#include <QGuiApplication>
#include <QVariantAnimation>
#include <QTimer>
#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#include <private/qwidgetwindow_qpa_p.h>
#else
#include <private/qwidgetwindow_p.h>
#endif

#ifndef QT_NO_OPENGL
#include <qopengl.h>
#include <qopenglcontext.h>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
#include <QtGui/private/qopengltextureblitter_p.h>
#else
#include <QtGui/qopengltextureblitter.h>
#endif

#include <QtGui/qpa/qplatformgraphicsbuffer.h>
#include <QtGui/qpa/qplatformgraphicsbufferhelper.h>
#endif

#include <qpa/qplatformscreen.h>
#include <qpa/qplatformcursor.h>
#include <qpa/qplatformnativeinterface.h>

DPP_BEGIN_NAMESPACE

PUBLIC_CLASS(QMouseEvent, WindowEventListener);
PUBLIC_CLASS(QWheelEvent, WindowEventListener);
PUBLIC_CLASS(QResizeEvent, WindowEventListener);
PUBLIC_CLASS(QWidget, WindowEventListener);
PUBLIC_CLASS(QWindow, WindowEventListener);

PUBLIC_CLASS(QXcbWindow, DPlatformBackingStore);

class WindowEventListener : public QObject
{
    Q_OBJECT

public:
    explicit WindowEventListener(DPlatformBackingStore *store)
        : QObject(0)
        , m_store(store)
    {
        cursorAnimation.setDuration(50);
        cursorAnimation.setEasingCurve(QEasingCurve::InExpo);

        connect(&cursorAnimation, &QVariantAnimation::valueChanged,
                this, &WindowEventListener::onAnimationValueChanged);

        startAnimationTimer.setSingleShot(true);
        startAnimationTimer.setInterval(300);

        connect(&startAnimationTimer, &QTimer::timeout,
                this, &WindowEventListener::startAnimation);
    }

    ~WindowEventListener()
    {
        const QWidgetWindow *widgetWindow = m_store->widgetWindow();

        QWidget *widget = widgetWindow->widget();

        if (widget) {
            VtableHook::clearGhostVtable(widget);
        } else {
            VtableHook::clearGhostVtable(m_store->window());
        }
    }

public slots:
    void updateWindowBlurAreasForWM()
    {
        m_store->updateWindowBlurAreasForWM();
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE
    {
        QWindow *window = qobject_cast<QWindow*>(obj);

        if (!window)
            return false;

        const QRect &window_geometry = window->geometry();
//        qDebug() << obj << event->type() << window_geometry;

        switch ((int)event->type()) {
        case QEvent::Wheel: {
            DQWheelEvent *e = static_cast<DQWheelEvent*>(event);

            if (!window_geometry.contains(e->globalPos()))
                return true;

            e->p -= m_store->windowOffset() / window->devicePixelRatio();

            break;
        }
        case QEvent::MouseMove:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease: {
            DQMouseEvent *e = static_cast<DQMouseEvent*>(event);

            if (Q_LIKELY(e->source() != Qt::MouseEventSynthesizedByQt)) {
                e->l -= m_store->windowOffset() / window->devicePixelRatio();
                e->w -= m_store->windowOffset() / window->devicePixelRatio();
            }

            if (window->flags().testFlag(Qt::Popup) || window->flags().testFlag(Qt::BypassWindowManagerHint))
                break;

            const QRect &window_visible_rect = QRect(m_store->windowValidRect.topLeft() + window_geometry.topLeft(),
                                                     m_store->windowValidRect.size() / window->devicePixelRatio());

            if (window->minimumSize() == window->maximumSize() || !m_store->m_enableSystemResize)
                goto skip_set_cursor;

            if (!leftButtonPressed && (!window_visible_rect.contains(e->globalPos())
                    || !m_store->m_clipPath.contains(e->windowPos() * window->devicePixelRatio()))) {
                if (event->type() == QEvent::MouseMove) {
                    bool isFixedWidth = window->minimumWidth() == window->maximumWidth();
                    bool isFixedHeight = window->minimumHeight() == window->maximumHeight();

                    Utility::CornerEdge mouseCorner;
                    QRect cornerRect;
                    const QRect window_real_geometry = window_visible_rect
                            + QMargins(MOUSE_MARGINS, MOUSE_MARGINS, MOUSE_MARGINS, MOUSE_MARGINS);

                    if (isFixedWidth || isFixedHeight)
                        goto set_edge;

                    /// begin set cursor corner type
                    cornerRect.setSize(QSize(MOUSE_MARGINS * 2, MOUSE_MARGINS * 2));
                    cornerRect.moveTopLeft(window_real_geometry.topLeft());

                    if (cornerRect.contains(e->globalPos())) {
                        mouseCorner = Utility::TopLeftCorner;

                        goto set_cursor;
                    }

                    cornerRect.moveTopRight(window_real_geometry.topRight());

                    if (cornerRect.contains(e->globalPos())) {
                        mouseCorner = Utility::TopRightCorner;

                        goto set_cursor;
                    }

                    cornerRect.moveBottomRight(window_real_geometry.bottomRight());

                    if (cornerRect.contains(e->globalPos())) {
                        mouseCorner = Utility::BottomRightCorner;

                        goto set_cursor;
                    }

                    cornerRect.moveBottomLeft(window_real_geometry.bottomLeft());

                    if (cornerRect.contains(e->globalPos())) {
                        mouseCorner = Utility::BottomLeftCorner;

                        goto set_cursor;
                    }
set_edge:
                    /// begin set cursor edge type
                    if (e->globalX() <= window_visible_rect.x()) {
                        if (isFixedWidth)
                            goto skip_set_cursor;

                        mouseCorner = Utility::LeftEdge;
                    } else if (e->globalX() < window_visible_rect.right()) {
                        if (isFixedHeight)
                            goto skip_set_cursor;

                        if (e->globalY() <= window_visible_rect.y()) {
                            mouseCorner = Utility::TopEdge;
                        } else if (!isFixedWidth || e->globalY() >= window_visible_rect.bottom()) {
                            mouseCorner = Utility::BottomEdge;
                        } else {
                            goto skip_set_cursor;
                        }
                    } else if (!isFixedWidth && (!isFixedHeight || e->globalX() >= window_visible_rect.right())) {
                        mouseCorner = Utility::RightEdge;
                    } else {
                        goto skip_set_cursor;
                    }
set_cursor:
                    Utility::setWindowCursor(window->winId(), mouseCorner);

                    if (qApp->mouseButtons() == Qt::LeftButton) {
                        Utility::startWindowSystemResize(window->winId(), mouseCorner);

                        cancelAdsorbCursor();
                    } else {
                        adsorbCursor(mouseCorner);
                    }
                } else if (event->type() == QEvent::MouseButtonRelease) {
                    Utility::cancelWindowMoveResize(window->winId());
                }

                return true;
            }
skip_set_cursor:
            if (e->buttons() == Qt::LeftButton) {
                if (e->type() == QEvent::MouseButtonPress)
                    setLeftButtonPressed(true);
                else if (e->type() == QEvent::MouseButtonRelease)
                    setLeftButtonPressed(false);
            } else {
                setLeftButtonPressed(false);
            }

            qApp->setOverrideCursor(window->cursor());

            cancelAdsorbCursor();
            canAdsorbCursor = m_store->m_enableSystemResize;

            break;
        }
        case QEvent::Resize: {
            const QSize &old_size = window->property("_dxcb_window_old_size").toSize();
            const QSize &new_size = window->handle()->geometry().size();

            if (old_size == new_size)
                return true;

            window->setProperty("_dxcb_window_old_size", new_size);

            DQResizeEvent *e = static_cast<DQResizeEvent*>(event);

            e->s = window->size();
            break;
        }
        case QEvent::Enter:
            canAdsorbCursor = m_store->m_enableSystemResize;

            break;
        case QEvent::Leave:
            canAdsorbCursor = false;
            cancelAdsorbCursor();

            break;
        case QEvent::DynamicPropertyChange: {
            QDynamicPropertyChangeEvent *e = static_cast<QDynamicPropertyChangeEvent*>(event);

            if (e->propertyName() == netWmStates) {
                m_store->onWindowStateChanged();
            } else if (e->propertyName() == windowRadius) {
                m_store->updateWindowRadius();
            } else if (e->propertyName() == borderWidth) {
                m_store->updateBorderWidth();
            } else if (e->propertyName() == borderColor) {
                m_store->updateBorderColor();
            } else if (e->propertyName() == shadowRadius) {
                m_store->updateShadowRadius();
            } else if (e->propertyName() == shadowOffset) {
                m_store->updateShadowOffset();
            } else if (e->propertyName() == shadowColor) {
                m_store->updateShadowColor();
            } else if (e->propertyName() == clipPath) {
                m_store->updateUserClipPath();
            } else if (e->propertyName() == frameMask) {
                m_store->updateFrameMask();
            } else if (e->propertyName() == translucentBackground) {
                m_store->updateTranslucentBackground();
            } else if (e->propertyName() == enableSystemResize) {
                m_store->updateEnableSystemResize();

                if (!m_store->m_enableSystemMove) {
                    qApp->setOverrideCursor(window->cursor());

                    cancelAdsorbCursor();
                    canAdsorbCursor = false;

                    Utility::cancelWindowMoveResize(window->winId());
                }
            } else if (e->propertyName() == enableSystemMove) {
                m_store->updateEnableSystemMove();

                if (!m_store->m_enableSystemMove)
                    Utility::cancelWindowMoveResize(window->winId());

                updateStealMoveEvent();
            } else if (e->propertyName() == enableBlurWindow) {
                m_store->updateEnableBlurWindow();
            } else if (e->propertyName() == windowBlurAreas) {
                m_store->updateWindowBlurAreas();
            } else if (e->propertyName() == windowBlurPaths) {
                m_store->updateWindowBlurPaths();
            } else if (e->propertyName() == autoInputMaskByClipPath) {
                m_store->updateAutoInputMaskByClipPath();
            }

            break;
        }
        default: break;
        }

        return false;
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        Q_UNUSED(event);

        ///TODO: Warning: System move finished no mouse release event
        Utility::startWindowSystemMove(reinterpret_cast<QWidget*>(this)->winId());
    }

    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE
    {
        if (event->timerId() == m_store->updateShadowTimer.timerId()) {
            m_store->repaintWindowShadow();
        }
    }

private:
    void setLeftButtonPressed(bool pressed)
    {
        if (leftButtonPressed == pressed)
            return;

        if (!pressed)
            Utility::cancelWindowMoveResize(m_store->window()->winId());

        leftButtonPressed = pressed;

        updateStealMoveEvent();
    }

    void updateStealMoveEvent()
    {
        const QWidgetWindow *widgetWindow = m_store->widgetWindow();

        QWidget *widget = widgetWindow->widget();

        if (widget) {
            if (leftButtonPressed && m_store->m_enableSystemMove) {
                VtableHook::overrideVfptrFun(static_cast<DQWidget*>(widget), &DQWidget::mouseMoveEvent,
                                             this, &WindowEventListener::mouseMoveEvent);
            } else {
                VtableHook::resetVfptrFun(static_cast<DQWidget*>(widget), &DQWidget::mouseMoveEvent);
            }
        } else {
            QWindow *window = m_store->window();

            if (leftButtonPressed && m_store->m_enableSystemMove) {
                VtableHook::overrideVfptrFun(static_cast<DQWindow*>(window), &DQWindow::mouseMoveEvent,
                                             this, &WindowEventListener::mouseMoveEvent);
            } else {
                VtableHook::resetVfptrFun(static_cast<DQWindow*>(window), &DQWindow::mouseMoveEvent);
            }
        }
    }

    void adsorbCursor(Utility::CornerEdge cornerEdge)
    {
        lastCornerEdge = cornerEdge;

        if (!canAdsorbCursor)
            return;

        if (cursorAnimation.state() == QVariantAnimation::Running)
            return;

        startAnimationTimer.start();
    }

    void cancelAdsorbCursor()
    {
        QSignalBlocker blocker(&startAnimationTimer);
        Q_UNUSED(blocker)
        startAnimationTimer.stop();
        cursorAnimation.stop();
    }

    void onAnimationValueChanged(const QVariant &value)
    {
        QCursor::setPos(value.toPoint());
    }

    void startAnimation()
    {
        QPoint cursorPos = QCursor::pos();
        QPoint toPos = cursorPos;
        const QRect geometry = QRect(m_store->windowValidRect.topLeft() + m_store->window()->position(),
                                     m_store->windowValidRect.size() / m_store->window()->devicePixelRatio()).adjusted(-1, -1, 1, 1);

        switch (lastCornerEdge) {
        case Utility::TopLeftCorner:
            toPos = geometry.topLeft();
            break;
        case Utility::TopEdge:
            toPos.setY(geometry.y());
            break;
        case Utility::TopRightCorner:
            toPos = geometry.topRight();
            break;
        case Utility::RightEdge:
            toPos.setX(geometry.right());
            break;
        case Utility::BottomRightCorner:
            toPos = geometry.bottomRight();
            break;
        case Utility::BottomEdge:
            toPos.setY(geometry.bottom());
            break;
        case Utility::BottomLeftCorner:
            toPos = geometry.bottomLeft();
            break;
        case Utility::LeftEdge:
            toPos.setX(geometry.x());
            break;
        default:
            break;
        }

        const QPoint &tmp = toPos - cursorPos;

        if (qAbs(tmp.x()) < 3 && qAbs(tmp.y()) < 3)
            return;

        canAdsorbCursor = false;

        cursorAnimation.setStartValue(cursorPos);
        cursorAnimation.setEndValue(toPos);
        cursorAnimation.start();
    }

    /// mouse left button is pressed in window valid geometry
    bool leftButtonPressed = false;

    bool canAdsorbCursor = false;
    Utility::CornerEdge lastCornerEdge;
    QTimer startAnimationTimer;
    QVariantAnimation cursorAnimation;

    DPlatformBackingStore *m_store;
};

//class DXcbShmGraphicsBuffer : public QPlatformGraphicsBuffer
//{
//public:
//    DXcbShmGraphicsBuffer(QImage *image)
//        : QPlatformGraphicsBuffer(image->size(), QImage::toPixelFormat(image->format()))
//        , m_access_lock(QPlatformGraphicsBuffer::None)
//        , m_image(image)
//    { }

//    bool doLock(AccessTypes access, const QRect &rect) Q_DECL_OVERRIDE
//    {
//        Q_UNUSED(rect);
//        if (access & ~(QPlatformGraphicsBuffer::SWReadAccess | QPlatformGraphicsBuffer::SWWriteAccess))
//            return false;

//        m_access_lock |= access;
//        return true;
//    }
//    void doUnlock() Q_DECL_OVERRIDE { m_access_lock = None; }

//    const uchar *data() const Q_DECL_OVERRIDE { return m_image.bits(); }
//    uchar *data() Q_DECL_OVERRIDE { return m_image.bits(); }
//    int bytesPerLine() const Q_DECL_OVERRIDE { return m_image.bytesPerLine(); }

//    Origin origin() const Q_DECL_OVERRIDE { return QPlatformGraphicsBuffer::OriginTopLeft; }

//private:
//    AccessTypes m_access_lock;
//    QImage *m_image;
//};

DPlatformBackingStore::DPlatformBackingStore(QWindow *window, QXcbBackingStore *proxy)
    : QPlatformBackingStore(window)
    , m_proxy(proxy),
#ifndef QT_NO_OPENGL
    m_textureId(0),
    m_blitter(0)
#endif
{

    m_eventListener = new WindowEventListener(this);
    m_windowHook = DPlatformWindowHook::getHookByWindow(window->handle());
    shadowPixmap.fill(Qt::transparent);

#ifdef Q_OS_LINUX
    m_enableShadow = DXcbWMSupport::instance()->hasWindowAlpha();

    QObject::connect(DXcbWMSupport::instance(), &DXcbWMSupport::hasCompositeChanged,
                     m_eventListener, [this, window] (bool hasComposite) {
        m_enableShadow = hasComposite;
        updateWindowMargins();
        updateClipPath();
        doDelayedUpdateWindowShadow();
    });
#endif

    initUserProperties();
    //! Warning: At this point you must be initialized window Margins and window Extents
    updateWindowMargins();
//    updateFrameExtents();

    VtableHook::overrideVfptrFun(static_cast<QXcbWindow*>(window->handle()), &QXcbWindowEventListener::handlePropertyNotifyEvent,
                                 this, &DPlatformBackingStore::handlePropertyNotifyEvent);

    QObject::connect(window, &QWindow::windowStateChanged,
                     m_eventListener, [this] {
        updateWindowMargins(false);
    });

    window->installEventFilter(m_eventListener);
}

DPlatformBackingStore::~DPlatformBackingStore()
{
    delete m_proxy;
    delete m_eventListener;

//    if (m_graphicsBuffer)
//        delete m_graphicsBuffer;

    VtableHook::clearGhostVtable(static_cast<QXcbWindowEventListener*>(static_cast<QXcbWindow*>(window()->handle())));
}

QPaintDevice *DPlatformBackingStore::paintDevice()
{
    return &m_image;
}

static QColor colorBlend(const QColor &color1, const QColor &color2)
{
    QColor c2 = color2.toRgb();

    if (c2.alpha() >= 255)
        return c2;

    QColor c1 = color1.toRgb();
    qreal c1_weight = 1 - c2.alphaF();

    int r = c1_weight * c1.red() + c2.alphaF() * c2.red();
    int g = c1_weight * c1.green() + c2.alphaF() * c2.green();
    int b = c1_weight * c1.blue() + c2.alphaF() * c2.blue();

    return QColor(r, g, b);
}

void DPlatformBackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(region)

    if (!m_proxy->paintDevice())
        return;

    const QPoint &windowOffset = this->windowOffset();
    QRegion tmp_region;

    QPainter pa(m_proxy->paintDevice());

    pa.setCompositionMode(QPainter::CompositionMode_Source);
#ifdef Q_OS_LINUX
    if (DXcbWMSupport::instance()->hasWindowAlpha())
#endif
    pa.setRenderHint(QPainter::Antialiasing);

    if (m_enableShadow) {
        pa.setClipPath(m_windowClipPath);

        for (const QRect &rect : region.rects()) {
            const QRect &tmp_rect = rect.translated(windowOffset);

            pa.drawImage(tmp_rect, m_image, rect);

            if (tmp_rect.size() == m_size) {
                tmp_region += QRect(QPoint(0, 0), m_size);
            } else {
                tmp_region += tmp_rect.united(QRect(windowOffset, m_image.size()));
            }
        }
    } else {
        pa.setPen(m_borderColor);

        QColor border_color = m_borderColor;

#ifdef Q_OS_LINUX
        if (!DXcbWMSupport::instance()->hasWindowAlpha())
            border_color = colorBlend(QColor("#e0e0e0"), m_borderColor);
#endif
        tmp_region += QRect(QPoint(0, 0), m_size);

        if (m_borderWidth > 0) {
            QPainterPath path;

            path.addRect(tmp_region.rects().first());
            path -= m_windowClipPath;
            pa.fillPath(path, border_color);
            pa.setClipPath(m_windowClipPath);
        }

        pa.drawImage(windowOffset, m_image);
    }

    pa.end();

//    qDebug() << "flush" << window << tmp_region << offset;

    DPlatformWindowHook *window_hook = m_windowHook;

    if (window_hook)
        window_hook->setWindowMargins(QMargins(0, 0, 0, 0));

    m_proxy->flush(window, tmp_region, offset);

    if (window_hook)
        window_hook->setWindowMargins(windowMargins);
}



#ifndef QT_NO_OPENGL
static inline QRect deviceRect(const QRect &rect, QWindow *window)
{
    QRect deviceRect(rect.topLeft() * window->devicePixelRatio(),
                     rect.size() * window->devicePixelRatio());
    return deviceRect;
}

static QRegion deviceRegion(const QRegion &region, QWindow *window, const QPoint &offset)
{
    if (offset.isNull() && window->devicePixelRatio() <= 1)
        return region;

    QVector<QRect> rects;
    const QVector<QRect> regionRects = region.rects();
    rects.reserve(regionRects.count());
    foreach (const QRect &rect, regionRects)
        rects.append(deviceRect(rect.translated(offset), window));

    QRegion deviceRegion;
    deviceRegion.setRects(rects.constData(), rects.count());
    return deviceRegion;
}

static inline QRect toBottomLeftRect(const QRect &topLeftRect, int windowHeight)
{
    return QRect(topLeftRect.x(), windowHeight - topLeftRect.bottomRight().y() - 1,
                 topLeftRect.width(), topLeftRect.height());
}




static void blitTextureForWidget(const QPlatformTextureList *textures, int idx,
        QWindow *window, const QRect &deviceWindowRect,
        QOpenGLTextureBlitter *blitter, const QPoint &offset)
{
    Q_UNUSED(deviceWindowRect)

    //original clipRect is wrong because of frame margins
    //const QRect clipRect = textures->clipRect(idx);
    QRect rectInWindow = textures->geometry(idx);

    const QRect clipRect = QRect(QPoint(), rectInWindow.size());
    if (clipRect.isEmpty())
        return;

    // relative to the TLW, not necessarily our window (if the flush is for a native child widget), have to adjust
    rectInWindow.translate(-offset);

    const QRect clippedRectInWindow = rectInWindow & clipRect.translated(rectInWindow.topLeft());
    const QRect srcRect = toBottomLeftRect(clipRect, rectInWindow.height());

    const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(
            deviceRect(clippedRectInWindow, window),
            clippedRectInWindow);
            //deviceWindowRect);

    const QMatrix3x3 source = QOpenGLTextureBlitter::sourceTransform(deviceRect(srcRect, window),
            deviceRect(rectInWindow, window).size(),
            QOpenGLTextureBlitter::OriginBottomLeft);

    blitter->blit(textures->textureId(idx), target, source);
}


#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
void DPlatformBackingStore::composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                     QPlatformTextureList *textures, QOpenGLContext *context)
{
    Q_UNUSED(textures);
    Q_UNUSED(context);

    flush(window, region, offset);
}
#else
void DPlatformBackingStore::composeAndFlushHelper(QWindow *window, const QRegion &region,
        const QPoint &offset,
        QPlatformTextureList *textures, QOpenGLContext *context,
        bool translucentBackground)
{
    Q_UNUSED(region)
    //if (!qt_window_private(window)->receivedExpose)
        //return;

    if (!context->makeCurrent(window)) {
        qWarning("composeAndFlush: makeCurrent() failed");
        return;
    }

    //QWindowPrivate::get(window)->lastComposeTime.start();

    QPoint reversedOrigin(windowMargins.left(), windowMargins.bottom()); // reversed

    QOpenGLFunctions *funcs = context->functions();
    funcs->glViewport(reversedOrigin.x(), reversedOrigin.y(),
            m_windowSize.width() * window->devicePixelRatio(),
            m_windowSize.height() * window->devicePixelRatio());
    funcs->glClearColor(0, 0, 0, translucentBackground ? 0 : 1);
    funcs->glClear(GL_COLOR_BUFFER_BIT);

    if (!m_blitter) {
        m_blitter = new QOpenGLTextureBlitter;
        m_blitter->create();
    }

    m_blitter->bind();

    const QRect deviceWindowRect = deviceRect(QRect(windowOffset(), m_windowSize), window);

    // Textures for renderToTexture widgets.
    for (int i = 0; i < textures->count(); ++i) {
        if (!textures->flags(i).testFlag(QPlatformTextureList::StacksOnTop))
            blitTextureForWidget(textures, i, window, deviceWindowRect, m_blitter, offset);
    }

    // Backingstore texture with the normal widgets.
    GLuint textureId = 0;
    QOpenGLTextureBlitter::Origin origin = QOpenGLTextureBlitter::OriginTopLeft;
#if 0
    if (QPlatformGraphicsBuffer *graphicsBuffer = this->graphicsBuffer()) {
        if (graphicsBuffer->size() != m_textureSize) {
            if (m_textureId)
                funcs->glDeleteTextures(1, &m_textureId);
            funcs->glGenTextures(1, &m_textureId);
            funcs->glBindTexture(GL_TEXTURE_2D, m_textureId);
            QOpenGLContext *ctx = QOpenGLContext::currentContext();
            if (!ctx->isOpenGLES() || ctx->format().majorVersion() >= 3) {
                funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
                funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            }
            funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            if (QPlatformGraphicsBufferHelper::lockAndBindToTexture(graphicsBuffer, &m_needsSwizzle, &m_premultiplied)) {
                m_textureSize = graphicsBuffer->size();
            } else {
                m_textureSize = QSize(0,0);
            }

            graphicsBuffer->unlock();
        } else if (!region.isEmpty()){
            funcs->glBindTexture(GL_TEXTURE_2D, m_textureId);
            QPlatformGraphicsBufferHelper::lockAndBindToTexture(graphicsBuffer, &m_needsSwizzle, &m_premultiplied);
        }

        if (graphicsBuffer->origin() == QPlatformGraphicsBuffer::OriginBottomLeft)
            origin = QOpenGLTextureBlitter::OriginBottomLeft;
        textureId = m_textureId;
    }
    else
#endif
    {
        TextureFlags flags = 0;
        //I don't know why region is empty
        //textureId = toTexture(deviceRegion(region, window, offset + windowOffset()), &m_textureSize, &flags);
        QRegion region(0, 0, m_windowSize.width(), m_windowSize.height());
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    textureId = toTexture(deviceRegion(region, window, offset), &m_textureSize, false);
#else
    textureId = toTexture(deviceRegion(region, window, offset), &m_textureSize, &flags);
#endif
        m_needsSwizzle = (flags & TextureSwizzle) != 0;
        m_premultiplied = (flags & TexturePremultiplied) != 0;
        if (flags & TextureFlip)
            origin = QOpenGLTextureBlitter::OriginBottomLeft;
    }

    funcs->glEnable(GL_BLEND);
    if (m_premultiplied)
        funcs->glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    else
        funcs->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

    if (textureId) {
        if (m_needsSwizzle)
#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
            m_blitter->setSwizzleRB(true);
#else
            m_blitter->setRedBlueSwizzle(true);
#endif
        // The backingstore is for the entire tlw.
        // In case of native children offset tells the position relative to the tlw.
        // reverse translate backward for widgets
        const QRect srcRect = toBottomLeftRect(deviceWindowRect.translated(offset-windowOffset()), m_textureSize.height());
        const QMatrix3x3 source = QOpenGLTextureBlitter::sourceTransform(srcRect,
                                                                         m_textureSize,
                                                                         origin);
        m_blitter->blit(textureId, QMatrix4x4(), source);
        if (m_needsSwizzle)
#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
            m_blitter->setSwizzleRB(false);
#else
            m_blitter->setRedBlueSwizzle(false);
#endif
    }

    // Textures for renderToTexture widgets that have WA_AlwaysStackOnTop set.
    for (int i = 0; i < textures->count(); ++i) {
        if (textures->flags(i).testFlag(QPlatformTextureList::StacksOnTop))
            blitTextureForWidget(textures, i, window, deviceWindowRect, m_blitter, offset);
    }

    funcs->glDisable(GL_BLEND);

    m_blitter->release();

    context->swapBuffers(window);
}

void DPlatformBackingStore::composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                                       QPlatformTextureList *textures, QOpenGLContext *context,
                                       bool translucentBackground)
{
    if (textures != nullptr) {
        composeAndFlushHelper(window, region, offset, textures, context, translucentBackground);
    }
}
#endif

QImage DPlatformBackingStore::toImage() const
{
    return m_image;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
GLuint DPlatformBackingStore::toTexture(const QRegion &dirtyRegion, QSize *textureSize) const
{
    return m_proxy->toTexture(dirtyRegion, textureSize);
}
#elif QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
GLuint DPlatformBackingStore::toTexture(const QRegion &dirtyRegion, QSize *textureSize, bool *needsSwizzle) const
{
    return QPlatformBackingStore::toTexture(dirtyRegion, textureSize, needsSwizzle);
}
#else
GLuint DPlatformBackingStore::toTexture(const QRegion &dirtyRegion, QSize *textureSize, TextureFlags *flags) const
{
    return QPlatformBackingStore::toTexture(dirtyRegion, textureSize, flags);
}
#endif
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
QPlatformGraphicsBuffer *DPlatformBackingStore::graphicsBuffer() const
{
//    return m_graphicsBuffer;
    return m_proxy->graphicsBuffer();
}
#endif

void DPlatformBackingStore::resize(const QSize &size, const QRegion &staticContents)
{
//    qDebug() << "resize" << size << staticContents;
#if QT_VERSION > QT_VERSION_CHECK(5, 5, 1)
    if (size == m_image.size())
        return;

    m_image = QImage(size, QImage::Format_ARGB32_Premultiplied);
#else
    const int dpr = int(window()->devicePixelRatio());
    const QSize xSize = size * dpr;
    if (xSize == m_image.size() && dpr == m_image.devicePixelRatio())
        return;

    m_image = QImage(xSize, QImage::Format_ARGB32_Premultiplied);
    m_image.setDevicePixelRatio(dpr);
#endif

//    if (m_graphicsBuffer)
//        delete m_graphicsBuffer;

//    m_graphicsBuffer = new DXcbShmGraphicsBuffer(&m_image);

    m_windowSize = size;
    m_size = QSize(size.width() + windowMargins.left() + windowMargins.right(),
                   size.height() + windowMargins.top() + windowMargins.bottom());

    m_proxy->resize(m_size, staticContents);

    updateClipPath();
    //! TODO: update window margins
    //    updateWindowMargins();

    if (isUserSetClipPath) {
        if (shadowPixmap.isNull()) {
            updateWindowShadow();
        }

        if (!m_windowClipPath.isEmpty())
            updateWindowBlurAreasForWM();
    } else {
        updateWindowShadow();

        if (!m_blurAreaList.isEmpty() || !m_blurPathList.isEmpty() || m_enableBlurWindow)
            updateWindowBlurAreasForWM();
    }

    updateInputShapeRegion();
    paintWindowShadow();
}

void DPlatformBackingStore::beginPaint(const QRegion &region)
{
    if (m_translucentBackground) {
        QPainter p(paintDevice());
        p.setCompositionMode(QPainter::CompositionMode_Source);
        const QVector<QRect> rects = region.rects();
        const QColor blank = Qt::transparent;
        for (QVector<QRect>::const_iterator it = rects.begin(); it != rects.end(); ++it) {
            const QRect &rect = *it;
            p.fillRect(rect, blank);
        }
    }
}

void DPlatformBackingStore::endPaint()
{
//    m_proxy->endPaint();

//    qDebug() << "end paint";
}

void DPlatformBackingStore::initUserProperties()
{
    updateWindowRadius();
    updateBorderWidth();
    updateBorderColor();
    updateUserClipPath();
    updateFrameMask();
    updateShadowRadius();
    updateShadowOffset();
    updateShadowColor();
    updateTranslucentBackground();
    updateEnableSystemMove();
    updateEnableSystemResize();
    updateEnableBlurWindow();
    updateWindowBlurAreas();
    updateWindowBlurPaths();
    updateAutoInputMaskByClipPath();
}

bool DPlatformBackingStore::updateWindowMargins(bool repaintShadow)
{
    Qt::WindowState state = window()->windowState();

    const QMargins old_margins = windowMargins;
    const QRect &window_geometry = window()->handle()->geometry();

    if (state == Qt::WindowMaximized || state == Qt::WindowFullScreen) {
        setWindowMargins(QMargins(0, 0, 0, 0));
    } else if (state != Qt::WindowMinimized) {
        setWindowMargins(QMargins(qMax(getShadowRadius() - getShadowOffset().x(), m_borderWidth),
                                  qMax(getShadowRadius() - getShadowOffset().y(), m_borderWidth),
                                  qMax(getShadowRadius() + getShadowOffset().x(), m_borderWidth),
                                  qMax(getShadowRadius() + getShadowOffset().y(), m_borderWidth)));
    }

    if (repaintShadow && old_margins != windowMargins) {
        window()->handle()->setGeometry(window_geometry);

        repaintWindowShadow();

        return true;
    }

    return false;
}

void DPlatformBackingStore::updateFrameExtents()
{
    QMargins extentsMargins = windowMargins;

    if (canUseClipPath() && !isUserSetClipPath) {
        extentsMargins -= m_borderWidth;
    }

    Utility::setFrameExtents(window()->winId(), extentsMargins);
}

void DPlatformBackingStore::updateInputShapeRegion()
{
    if (!windowGeometry().isValid())
        return;

    int mouse_margins;

    if (m_enableShadow)
        mouse_margins = canResize() ? MOUSE_MARGINS : 0;
    else
        mouse_margins = m_borderWidth;

    // clear old state
    Utility::setShapeRectangles(window()->winId(), QRegion(), true);
    Utility::setShapeRectangles(window()->winId(), QRegion(), false);

    if (m_autoInputMaskByClipPath && (isUserSetClipPath || getWindowRadius() > 0)) {
        QPainterPath p;

        if (Q_LIKELY(mouse_margins > 0)) {
            QPainterPathStroker stroker;
            stroker.setJoinStyle(Qt::MiterJoin);
            stroker.setWidth(mouse_margins * 2);
            p = stroker.createStroke(m_windowClipPath);
            p = p.united(m_windowClipPath);
            p.translate(-0.5, -0.5);
        } else {
            p = m_windowClipPath;
        }

        Utility::setShapePath(window()->winId(), p
                      #ifdef Q_OS_LINUX
                              , DXcbWMSupport::instance()->hasWindowAlpha()
                      #endif
                              );
    } else {
        QRegion region(windowGeometry().adjusted(-mouse_margins, -mouse_margins, mouse_margins, mouse_margins));
        Utility::setShapeRectangles(window()->winId(), region
                       #ifdef Q_OS_LINUX
                               , DXcbWMSupport::instance()->hasWindowAlpha()
                       #endif
                               );
    }
}

void DPlatformBackingStore::updateWindowRadius()
{
    const QVariant &v = window()->property(windowRadius);

    if (!v.isValid()) {
        window()->setProperty(windowRadius, m_windowRadius);

        return;
    }

    bool ok;
    int radius = v.toInt(&ok);

    if (ok && radius != m_windowRadius) {
        m_windowRadius = radius;
        isUserSetWindowRadius = true;

        updateClipPath();
    }
}

void DPlatformBackingStore::updateBorderWidth()
{
    const QVariant &v = window()->property(borderWidth);

    if (!v.isValid()) {
        window()->setProperty(borderWidth, m_borderWidth);

        return;
    }

    bool ok;
    int width = v.toInt(&ok);

    if (ok && width != m_borderWidth) {
        m_borderWidth = width;

        updateFrameExtents();

        if (!updateWindowMargins())
            doDelayedUpdateWindowShadow();
    }
}

void DPlatformBackingStore::updateBorderColor()
{
    const QVariant &v = window()->property(borderColor);

    if (!v.isValid()) {
        window()->setProperty(borderColor, m_borderColor);

        return;
    }

    const QColor &color = qvariant_cast<QColor>(v);

    if (color.isValid() && m_borderColor != color) {
        m_borderColor = color;

        doDelayedUpdateWindowShadow();
    }
}

void DPlatformBackingStore::updateUserClipPath()
{
    const QVariant &v = window()->property(clipPath);

    if (!v.isValid()) {
        return;
    }

    QPainterPath path;

    path = qvariant_cast<QPainterPath>(v);

    if (!isUserSetClipPath && path.isEmpty())
        return;

    isUserSetClipPath = !path.isEmpty();

    if (path.isEmpty())
        updateClipPath();
    else
        setClipPah(path * window()->devicePixelRatio());
}

void DPlatformBackingStore::updateClipPath()
{
    if (!isUserSetClipPath) {
        QPainterPath path;

        if (canUseClipPath())
            path.addRoundedRect(QRect(QPoint(0, 0), m_windowSize), getWindowRadius(), getWindowRadius());
        else
            path.addRect(0, 0, m_windowSize.width(), m_windowSize.height());

        setClipPah(path);
    }
}

void DPlatformBackingStore::updateFrameMask()
{
    const QVariant &v = window()->property(frameMask);

    if (!v.isValid()) {
        return;
    }

    QRegion region = qvariant_cast<QRegion>(v);

    static_cast<QXcbWindow*>(window()->handle())->QXcbWindow::setMask(region * window()->devicePixelRatio());

    isUserSetFrameMask = !region.isEmpty();
}

void DPlatformBackingStore::updateShadowRadius()
{
    const QVariant &v = window()->property(shadowRadius);

    if (!v.isValid()) {
        window()->setProperty(shadowRadius, m_shadowRadius);

        return;
    }

    bool ok;
    int radius = qMax(v.toInt(&ok), 0);

    if (ok && radius != m_shadowRadius) {
        m_shadowRadius = radius;

        updateWindowMargins();

        if (m_enableShadow)
            doDelayedUpdateWindowShadow();
    }
}

void DPlatformBackingStore::updateShadowOffset()
{
    const QVariant &v = window()->property(shadowOffset);

    if (!v.isValid()) {
        window()->setProperty(shadowOffset, m_shadowOffset);

        return;
    }

    const QPoint &offset = v.toPoint();

    if (offset != m_shadowOffset) {
        m_shadowOffset = offset;

        updateWindowMargins();

        if (m_enableShadow)
            doDelayedUpdateWindowShadow();
    }
}

void DPlatformBackingStore::updateShadowColor()
{
    const QVariant &v = window()->property(shadowColor);

    if (!v.isValid()) {
        window()->setProperty(shadowColor, m_shadowColor);

        return;
    }

    const QColor &color = qvariant_cast<QColor>(v);

    if (color.isValid() && m_shadowColor != color) {
        m_shadowColor = color;

        if (m_enableShadow)
            doDelayedUpdateWindowShadow();
    }
}

void DPlatformBackingStore::updateTranslucentBackground()
{
    const QVariant &v = window()->property(translucentBackground);

    if (!v.isValid()) {
        window()->setProperty(translucentBackground, m_translucentBackground);

        return;
    }

    m_translucentBackground = v.toBool();
}

void DPlatformBackingStore::updateEnableSystemResize()
{
    const QVariant &v = window()->property(enableSystemResize);

    if (!v.isValid()) {
        window()->setProperty(enableSystemResize, m_enableSystemResize);

        return;
    }

    if (m_enableSystemResize == v.toBool())
        return;

    m_enableSystemResize = v.toBool();

    updateInputShapeRegion();
}

void DPlatformBackingStore::updateEnableSystemMove()
{
    const QVariant &v = window()->property(enableSystemMove);

    if (!v.isValid()) {
        window()->setProperty(enableSystemMove, m_enableSystemMove);

        return;
    }

    m_enableSystemMove = v.toBool();
}

void DPlatformBackingStore::updateEnableBlurWindow()
{
    const QVariant &v = window()->property(enableBlurWindow);

    if (!v.isValid()) {
        window()->setProperty(enableBlurWindow, m_enableBlurWindow);

        return;
    }

    if (m_enableBlurWindow != v.toBool()) {
        m_enableBlurWindow = v.toBool();

#ifdef Q_OS_LINUX
        if (m_enableBlurWindow) {
            QObject::connect(DXcbWMSupport::instance(), &DXcbWMSupport::windowManagerChanged,
                             m_eventListener, &WindowEventListener::updateWindowBlurAreasForWM);
        } else {
            QObject::disconnect(DXcbWMSupport::instance(), &DXcbWMSupport::windowManagerChanged,
                                m_eventListener, &WindowEventListener::updateWindowBlurAreasForWM);
        }
#endif

        updateWindowBlurAreasForWM();
    }
}

void DPlatformBackingStore::updateWindowBlurAreas()
{
    const QVariant &v = window()->property(windowBlurAreas);
    const QVector<quint32> &tmpV = qvariant_cast<QVector<quint32>>(v);
    const QVector<Utility::BlurArea> &a = *(reinterpret_cast<const QVector<Utility::BlurArea>*>(&tmpV));

    if (a.isEmpty() && m_blurAreaList.isEmpty())
        return;

    m_blurAreaList = a;

    updateWindowBlurAreasForWM();
}

void DPlatformBackingStore::updateWindowBlurPaths()
{
    const QVariant &v = window()->property(windowBlurPaths);
    const QList<QPainterPath> paths = qvariant_cast<QList<QPainterPath>>(v);

    if (paths.isEmpty() && m_blurPathList.isEmpty())
        return;

    m_blurPathList = paths;

    updateWindowBlurAreasForWM();
}

void DPlatformBackingStore::updateAutoInputMaskByClipPath()
{
    const QVariant &v = window()->property(autoInputMaskByClipPath);

    if (!v.isValid()) {
        window()->setProperty(autoInputMaskByClipPath, m_autoInputMaskByClipPath);

        return;
    }

    if (m_autoInputMaskByClipPath != v.toBool()) {
        m_autoInputMaskByClipPath = v.toBool();
    }
}

void DPlatformBackingStore::setWindowMargins(const QMargins &margins)
{
    if (windowMargins == margins)
        return;

    windowMargins = margins;
    m_windowClipPath = m_clipPath.translated(windowOffset());

    DPlatformWindowHook *hook = m_windowHook;

    if (hook) {
        hook->setWindowMargins(margins, true);
    }

    const QSize &tmp_size = windowGeometry().size();

    m_size = QSize(tmp_size.width() + windowMargins.left() + windowMargins.right(),
                   tmp_size.height() + windowMargins.top() + windowMargins.bottom());

    if (m_size.isValid()) {
        m_proxy->resize(m_size, QRegion());
    }

    updateInputShapeRegion();
    updateFrameExtents();

    if (!m_blurAreaList.isEmpty() || !m_blurPathList.isEmpty() || m_enableBlurWindow)
        updateWindowBlurAreasForWM();

    window()->setProperty(frameMargins, QVariant::fromValue(windowMargins));
}

void DPlatformBackingStore::setClipPah(const QPainterPath &path)
{
    if (m_clipPath != path) {
        m_clipPath = path;
        m_windowClipPath = m_clipPath.translated(windowOffset());
        windowValidRect = m_clipPath.boundingRect().toRect();

        updateInputShapeRegion();

        if (isUserSetClipPath) {
            updateWindowBlurAreasForWM();
            doDelayedUpdateWindowShadow();
        }
    }
}

void DPlatformBackingStore::paintWindowShadow(QRegion region)
{
    if (!m_proxy->paintDevice() || shadowPixmap.isNull())
        return;

    QPainter pa;

    /// begin paint window drop shadow
    pa.begin(m_proxy->paintDevice());
    pa.setCompositionMode(QPainter::CompositionMode_Source);
    pa.drawPixmap(0, 0, shadowPixmap);
    pa.end();

    DPlatformWindowHook *window_hook = m_windowHook;

    if (window_hook)
        window_hook->setWindowMargins(QMargins(0, 0, 0, 0));

    if (region.isEmpty()) {
        region += QRect(windowMargins.left(), 0, m_windowSize.width(), windowMargins.top());
        region += QRect(windowMargins.left(), windowMargins.top() + m_windowSize.height(), m_windowSize.width(), windowMargins.bottom());
        region += QRect(0, 0, windowMargins.left(), m_size.height());
        region += QRect(windowMargins.left() + m_windowSize.width(), 0, windowMargins.right(), m_size.height());
    }

    m_proxy->flush(window(), region, QPoint(0, 0));

    if (window_hook)
        window_hook->setWindowMargins(windowMargins);
    /// end
}

void DPlatformBackingStore::repaintWindowShadow()
{
    updateShadowTimer.stop();
    shadowPixmap = QPixmap();

    if (windowMargins.isNull())
        return;

    updateWindowShadow();
    paintWindowShadow(QRegion(0, 0, m_size.width(), m_size.height()));

    if (!shadowPixmap.isNull())
        flush(window(), QRect(QPoint(0, 0), m_windowSize), QPoint(0, 0));
}

inline QSize margins2Size(const QMargins &margins)
{
    return QSize(margins.left() + margins.right(),
                 margins.top() + margins.bottom());
}

void DPlatformBackingStore::updateWindowShadow()
{
    if (m_image.isNull() || !m_enableShadow)
        return;

    bool paintShadow = isUserSetClipPath || shadowPixmap.isNull();

    if (!paintShadow) {
        QSize margins_size = margins2Size(windowMargins + getWindowRadius() + m_borderWidth);

        if (margins_size.width() > qMin(m_size.width(), shadowPixmap.width())
                || margins_size.height() > qMin(m_size.height(), shadowPixmap.height())) {
            paintShadow = true;
        }
    }

    if (paintShadow) {
        int shadow_radius = qMax(getShadowRadius(), m_borderWidth);

        QImage image;

        if (shadow_radius > m_borderWidth) {
            QPixmap pixmap(m_size - QSize(2 * shadow_radius, 2 * shadow_radius));

            if (pixmap.isNull())
                return;

            pixmap.fill(Qt::transparent);

            QPainter pa(&pixmap);

            pa.fillPath(m_clipPath, m_shadowColor);
            pa.end();

            image = Utility::dropShadow(pixmap, shadow_radius, m_shadowColor);
        } else {
            image = QImage(m_size, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
        }

        /// begin paint window border;
        QPainter pa;
        pa.begin(&image);

        if (m_borderWidth > 0) {
            QPainterPathStroker pathStroker;

            pathStroker.setWidth(m_borderWidth * 2);

            QTransform transform = pa.transform();
            const QRectF &clipRect = m_clipPath.boundingRect();

            transform.translate(windowMargins.left() + 2, windowMargins.top() + 2);
            transform.scale((clipRect.width() - 4) / clipRect.width(),
                            (clipRect.height() - 4) / clipRect.height());

            pa.setRenderHint(QPainter::Antialiasing);
            pa.fillPath(pathStroker.createStroke(m_windowClipPath), m_borderColor);
            pa.setCompositionMode(QPainter::CompositionMode_Clear);
            pa.setRenderHint(QPainter::Antialiasing, false);
            pa.setTransform(transform);
        }

        pa.fillPath(m_clipPath, Qt::transparent);
        pa.end();
        /// end

        shadowPixmap = QPixmap::fromImage(image);
    } else {
        shadowPixmap = QPixmap::fromImage(Utility::borderImage(shadowPixmap, windowMargins + getWindowRadius(), m_size));
    }
}

bool DPlatformBackingStore::updateWindowBlurAreasForWM()
{
    const QRect &windowValidRect = windowGeometry();

    if (windowValidRect.isEmpty())
        return false;

    xcb_window_t top_level_w = Utility::getNativeTopLevelWindow(window()->winId());
    QPoint offset;

    if (top_level_w != window()->winId()) {
        offset = Utility::translateCoordinates(QPoint(0, 0), window()->winId(), top_level_w);
    }

    QVector<Utility::BlurArea> newAreas;
    qreal device_pixel_ratio = window()->devicePixelRatio();

    if (m_enableBlurWindow) {
        Utility::BlurArea area;

        area.x = windowValidRect.x() << offset.x();
        area.y = windowValidRect.y() << offset.y();
        area.width = windowValidRect.width();
        area.height = windowValidRect.height();
        area.xRadius = getWindowRadius();
        area.yRaduis = getWindowRadius();

        newAreas.append(std::move(area));
    } else {
        newAreas.reserve(m_blurAreaList.size());

        foreach (Utility::BlurArea area, m_blurAreaList) {
            area *= device_pixel_ratio;

            if (area.x < 0) {
                area.width += area.x;
                area.x = 0;
            }

            if (area.y < 0) {
                area.height += area.y;
                area.y = 0;
            }

            area.x += windowValidRect.x();
            area.y += windowValidRect.y();
            area.width = qMin(area.x + area.width, windowValidRect.right() + 1) - area.x;
            area.height = qMin(area.y + area.height, windowValidRect.bottom() + 1) - area.y;
            area.x += offset.x();
            area.y += offset.y();

            newAreas.append(std::move(area));
        }
    }

    if ((isUserSetClipPath && !m_windowClipPath.isEmpty()) || !m_blurPathList.isEmpty()) {
        QList<QPainterPath> newPathList;

        newPathList.reserve(m_blurPathList.size());

        foreach (QPainterPath path, m_blurPathList) {
            path *= device_pixel_ratio;

            if (m_windowClipPath.isEmpty())
                newPathList << path.translated(windowValidRect.topLeft() + offset);
            else
                newPathList << path.translated(windowValidRect.topLeft()).intersected(m_windowClipPath).translated(offset);
        }

        foreach (const Utility::BlurArea &area, newAreas) {
            QPainterPath path;

            path.addRoundedRect(area.x, area.y, area.width, area.height, area.xRadius, area.yRaduis);

            if (m_windowClipPath.isEmpty())
                newPathList << path;
            else
                newPathList << path.intersected(m_windowClipPath.translated(offset));
        }

        return Utility::blurWindowBackgroundByPaths(top_level_w, newPathList);
    }

    return Utility::blurWindowBackground(top_level_w, newAreas);
}

void DPlatformBackingStore::doDelayedUpdateWindowShadow(int delaye)
{
    if (m_eventListener)
        updateShadowTimer.start(delaye, m_eventListener);
}

bool DPlatformBackingStore::isWidgetWindow(const QWindow *window)
{
    return window->metaObject()->className() == QStringLiteral("QWidgetWindow");
}

QWidgetWindow *DPlatformBackingStore::widgetWindow() const
{
    return static_cast<QWidgetWindow*>(window());
}

int DPlatformBackingStore::getWindowRadius() const
{
#ifdef Q_OS_LINUX
    return (isUserSetWindowRadius || DXcbWMSupport::instance()->hasWindowAlpha()) ? m_windowRadius : 0;
#else
    return m_windowRadius;
#endif
}

bool DPlatformBackingStore::canUseClipPath() const
{
    QXcbWindow::NetWmStates states = (QXcbWindow::NetWmStates)window()->property(netWmStates).toInt();

    if (states & (QXcbWindow::NetWmStateFullScreen | QXcbWindow::NetWmStateMaximizedHorz | QXcbWindow::NetWmStateMaximizedVert)) {
        return false;
    }

    return true;
}

bool DPlatformBackingStore::canResize() const
{
    return m_enableSystemResize
            && !window()->flags().testFlag(Qt::Popup)
            && window()->minimumSize() != window()->maximumSize();
}

void DPlatformBackingStore::onWindowStateChanged()
{
    updateClipPath();
    updateFrameExtents();
    doDelayedUpdateWindowShadow();
}

void DPlatformBackingStore::handlePropertyNotifyEvent(const xcb_property_notify_event_t *event)
{
    DQXcbWindow *window = static_cast<DQXcbWindow*>(reinterpret_cast<QXcbWindowEventListener*>(this));
    QWindow *ww = window->window();

    Qt::WindowState oldState = ww->windowState();

    window->QXcbWindow::handlePropertyNotifyEvent(event);

    if (window->m_windowState != oldState) {
        ww->setWindowState(window->m_windowState);
    }

    if (event->window == window->xcb_window()
            && event->atom == window->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_STATE))) {
        QXcbWindow::NetWmStates states = window->netWmStates();

        ww->setProperty(netWmStates, (int)states);
    }
}

DPP_END_NAMESPACE

#include "dplatformbackingstore.moc"

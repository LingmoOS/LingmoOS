// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dframewindow.h"
#include "dplatformwindowhelper.h"
#include "dplatformintegration.h"

#ifdef Q_OS_LINUX
#include "dwmsupport.h"
#include "qxcbwindow.h"
#include "qxcbscreen.h"

#include <xcb/damage.h>
#include <xcb/composite.h>
#endif

#include <QPainter>
#include <QGuiApplication>
#include <QLibrary>
#include <QDebug>

#include <private/qguiapplication_p.h>
#include <private/qpaintdevicewindow_p.h>
#include <qpa/qplatformbackingstore.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformcursor.h>
#include <qpa/qplatformgraphicsbuffer.h>

#ifdef Q_OS_LINUX
#include <cairo-xlib.h>
#endif

DPP_BEGIN_NAMESPACE

class Cairo
{
public:
    Cairo()
    {
        // fix found lib was libcairo2-dev without version number
        cairo = new QLibrary("cairo", "2");

        if (!cairo->load()) {
            delete cairo;
            cairo = nullptr;
            return;
        }

#define INIT_FUN(Name) Name = reinterpret_cast<decltype(Name)>(cairo->resolve(#Name)); Q_ASSERT(Name)

        INIT_FUN(cairo_image_surface_create_for_data);
        INIT_FUN(cairo_create);
        INIT_FUN(cairo_surface_mark_dirty);
        INIT_FUN(cairo_set_source_rgb);
        INIT_FUN(cairo_set_source_surface);
        INIT_FUN(cairo_set_operator);
        INIT_FUN(cairo_move_to);
        INIT_FUN(cairo_line_to);
        INIT_FUN(cairo_curve_to);
        INIT_FUN(cairo_clip);
        INIT_FUN(cairo_rectangle);
        INIT_FUN(cairo_fill);
        INIT_FUN(cairo_paint);
        INIT_FUN(cairo_destroy);
        INIT_FUN(cairo_surface_destroy);
        INIT_FUN(cairo_xlib_surface_set_drawable);
        INIT_FUN(cairo_xlib_surface_create);
        INIT_FUN(cairo_xlib_surface_get_width);
        INIT_FUN(cairo_xlib_surface_get_height);
    }

    ~Cairo()
    {
        if (cairo)
            delete cairo;
    }

    bool isValid() const
    {
        return cairo;
    }

    cairo_surface_t *(*cairo_image_surface_create_for_data)(unsigned char *data, cairo_format_t  format, int width, int height, int stride);
    cairo_t *(*cairo_create)(cairo_surface_t *target);
    void (*cairo_surface_mark_dirty)(cairo_surface_t *surface);
    void (*cairo_set_source_rgb)(cairo_t *cr, double red, double green, double blue);
    void (*cairo_set_source_surface)(cairo_t *cr, cairo_surface_t *surface, double x, double y);
    void (*cairo_set_operator)(cairo_t *cr, cairo_operator_t op);
    void (*cairo_move_to)(cairo_t *cr, double x, double y);
    void (*cairo_line_to)(cairo_t *cr, double x, double y);
    void (*cairo_curve_to)(cairo_t *cr, double x1, double y1, double x2, double y2, double x3, double y3);
    void (*cairo_clip)(cairo_t *cr);
    void (*cairo_rectangle)(cairo_t *cr, double x, double y, double width, double height);
    void (*cairo_fill)(cairo_t *cr);
    void (*cairo_paint)(cairo_t *cr);
    void (*cairo_destroy)(cairo_t *cr);
    void (*cairo_surface_destroy)(cairo_surface_t *surface);
    void (*cairo_xlib_surface_set_drawable)(cairo_surface_t *surface, Drawable drawable, int width, int height);
    cairo_surface_t *(*cairo_xlib_surface_create)(Display *dpy, Drawable drawable, Visual *visual, int width, int height);
    int (*cairo_xlib_surface_get_width)(cairo_surface_t *surface);
    int (*cairo_xlib_surface_get_height)(cairo_surface_t *surface);

private:
    QLibrary *cairo = nullptr;
};

Q_GLOBAL_STATIC(Cairo, __cairo)

class DFrameWindowPrivate : public QPaintDeviceWindowPrivate
{
    Q_DECLARE_PUBLIC(DFrameWindow)
public:

    void resize(const QSize &size)
    {
        Q_Q(DFrameWindow);

        if (this->size == size)
            return;

        this->size = size;

        q->platformBackingStore->resize(size, QRegion());
        q->update();
        q->drawNativeWindowXPixmap();
    }

    void beginPaint(const QRegion &region) Q_DECL_OVERRIDE
    {
        Q_Q(DFrameWindow);

        if (!q->m_redirectContent) {
            if (size != q->handle()->QPlatformWindow::geometry().size()) {
                size = q->handle()->QPlatformWindow::geometry().size();
                q->platformBackingStore->resize(size, QRegion());
                markWindowAsDirty();
            }

            q->platformBackingStore->beginPaint(region * q->devicePixelRatio());
        }
    }

    void endPaint() Q_DECL_OVERRIDE
    {
        Q_Q(DFrameWindow);

        if (!q->m_redirectContent)
            q->platformBackingStore->endPaint();
    }

    void flush(const QRegion &region) Q_DECL_OVERRIDE
    {
        Q_Q(DFrameWindow);

        if (!q->m_redirectContent) {
            return q->platformBackingStore->flush(q, region * q->devicePixelRatio(), QPoint(0, 0));
        }

        flushArea += region * q->devicePixelRatio();

        if (flushTimer > 0) {
            return;
        }

        flushTimer = q->startTimer(8, Qt::PreciseTimer);
    }

    QSize size;
    int flushTimer = 0;
    QRegion flushArea;
};

QList<DFrameWindow*> DFrameWindow::frameWindowList;

DFrameWindow::DFrameWindow(QWindow *content)
    : QPaintDeviceWindow(*new DFrameWindowPrivate(), 0)
    , platformBackingStore(QGuiApplicationPrivate::platformIntegration()->createPlatformBackingStore(this))
    , m_redirectContent(DPlatformWindowHelper::windowRedirectContent(content))
#ifdef Q_OS_LINUX
    , m_contentWindow(content)
    , nativeWindowXPixmap(XCB_PIXMAP_NONE)
#endif
{
    setSurfaceType(QSurface::RasterSurface);

    QSurfaceFormat f = this->format();
    f.setAlphaBufferSize(8);
    setFormat(f);

    m_cursorAnimation.setDuration(50);
    m_cursorAnimation.setEasingCurve(QEasingCurve::InExpo);

    connect(&m_cursorAnimation, &QVariantAnimation::valueChanged,
            this, [this] (const QVariant &value) {
        qApp->primaryScreen()->handle()->cursor()->setPos(value.toPoint());
    });

    m_startAnimationTimer.setSingleShot(true);
    m_startAnimationTimer.setInterval(300);

    connect(&m_startAnimationTimer, &QTimer::timeout,
            this, &DFrameWindow::startCursorAnimation);

    updateContentMarginsHint();

    frameWindowList.append(this);

    connect(this, &DFrameWindow::windowStateChanged,
            this, &DFrameWindow::updateMask);
    connect(&m_updateShadowTimer, &QTimer::timeout,
            this, &DFrameWindow::updateShadow);
}

DFrameWindow::~DFrameWindow()
{
    frameWindowList.removeOne(this);

#ifdef Q_OS_LINUX
    if (nativeWindowXSurface)
        __cairo->cairo_surface_destroy(nativeWindowXSurface);

    if (nativeWindowXPixmap != XCB_PIXMAP_NONE)
        xcb_free_pixmap(DPlatformIntegration::xcbConnection()->xcb_connection(), nativeWindowXPixmap);
#endif

    delete platformBackingStore;
}

QWindow *DFrameWindow::contentWindow() const
{
    return m_contentWindow.data();
}

int DFrameWindow::shadowRadius() const
{
    return m_shadowRadius;
}

void DFrameWindow::setShadowRadius(int radius)
{
    if (m_shadowRadius == radius)
        return;

    m_shadowRadius = radius;

    updateContentMarginsHint();
}

QPoint DFrameWindow::shadowOffset() const
{
    return m_shadowOffset;
}

void DFrameWindow::setShadowOffset(const QPoint &offset)
{
    if (m_shadowOffset == offset)
        return;

    m_shadowOffset = offset;

    updateContentMarginsHint();
}

QColor DFrameWindow::shadowColor() const
{
    return m_shadowColor;
}

void DFrameWindow::setShadowColor(const QColor &color)
{
    if (m_shadowColor == color)
        return;

    m_shadowColor = color;

    updateShadowAsync();
}

int DFrameWindow::borderWidth() const
{
    return m_borderWidth;
}

void DFrameWindow::setBorderWidth(int width)
{
    if (m_borderWidth == width)
        return;

    m_borderWidth = width;

    updateContentMarginsHint();
}

QColor DFrameWindow::borderColor() const
{
    return m_borderColor;
}

void DFrameWindow::setBorderColor(const QColor &color)
{
    if (m_borderColor == color)
        return;

    m_borderColor = color;

    updateShadowAsync();
}

QPainterPath DFrameWindow::contentPath() const
{
    return m_clipPathOfContent;
}

inline static QSize margins2Size(const QMargins &margins)
{
    return QSize(margins.left() + margins.right(),
                 margins.top() + margins.bottom());
}

void DFrameWindow::setContentPath(const QPainterPath &path)
{
    setContentPath(path, false);
}

void DFrameWindow::setContentRoundedRect(const QRect &rect, int radius)
{
    QPainterPath path;

    path.addRoundedRect(rect, radius, radius);
    m_contentGeometry = rect.translated(contentOffsetHint());

    setContentPath(path, true, radius);
}

QMargins DFrameWindow::contentMarginsHint() const
{
    return m_contentMarginsHint;
}

QPoint DFrameWindow::contentOffsetHint() const
{
    return QPoint(m_contentMarginsHint.left(), m_contentMarginsHint.top());
}

bool DFrameWindow::isClearContentAreaForShadowPixmap() const
{
    return m_clearContent;
}

void DFrameWindow::setClearContentAreaForShadowPixmap(bool clear)
{
    if (m_clearContent == clear)
        return;

    m_clearContent = clear;

    if (clear && !m_shadowImage.isNull()) {
        QPainter pa(&m_shadowImage);

        pa.setCompositionMode(QPainter::CompositionMode_Clear);
        pa.setRenderHint(QPainter::Antialiasing);
        pa.fillPath(m_clipPathOfContent.translated(QPoint(m_shadowRadius, m_shadowRadius) - m_shadowOffset) * devicePixelRatio(), Qt::transparent);
        pa.end();
    }
}

bool DFrameWindow::isEnableSystemResize() const
{
    return m_enableSystemResize;
}

void DFrameWindow::setEnableSystemResize(bool enable)
{
    m_enableSystemResize = enable;

    if (!m_enableSystemResize)
        Utility::cancelWindowMoveResize(Utility::getNativeTopLevelWindow(winId()));
}

bool DFrameWindow::isEnableSystemMove() const
{
#ifdef Q_OS_LINUX
    if (!m_enableSystemMove)
        return false;

    quint32 hints = DXcbWMSupport::getMWMFunctions(Utility::getNativeTopLevelWindow(winId()));

    return (hints == DXcbWMSupport::MWM_FUNC_ALL || hints & DXcbWMSupport::MWM_FUNC_MOVE);
#endif

    return m_enableSystemMove;
}

void DFrameWindow::setEnableSystemMove(bool enable)
{
    m_enableSystemMove = enable;

    if (!m_enableSystemMove) {
        setCursor(Qt::ArrowCursor);

        cancelAdsorbCursor();
        m_canAdsorbCursor = false;

        Utility::cancelWindowMoveResize(Utility::getNativeTopLevelWindow(winId()));
    }
}

void DFrameWindow::disableRepaintShadow()
{
    m_canUpdateShadow = false;
}

void DFrameWindow::enableRepaintShadow()
{
    m_canUpdateShadow = true;
}

bool DFrameWindow::redirectContent() const
{
    return m_redirectContent;
}

void DFrameWindow::paintEvent(QPaintEvent *)
{
    if (m_redirectContent) {
#ifdef Q_OS_LINUX
        drawNativeWindowXPixmap();
#endif
    } else {
        drawShadowTo(this);
    }
}

void DFrameWindow::showEvent(QShowEvent *event)
{
    // Set frame extents
    Utility::setFrameExtents(winId(), contentMarginsHint() * devicePixelRatio());
    updateShadow();
    QPaintDeviceWindow::showEvent(event);
}

void DFrameWindow::mouseMoveEvent(QMouseEvent *event)
{
    // 无论如何，在已进入move resize时都不再进行后面的处理，防止触屏移动窗口时会误触进入到窗口resize状态
    if (m_isSystemMoveResizeState && qApp->mouseButtons() == Qt::LeftButton) {
        Utility::updateMousePointForWindowMove(Utility::getNativeTopLevelWindow(winId()));

        return;
    }

    if (event->source() == Qt::MouseEventSynthesizedByQt && qApp->mouseButtons() == Qt::LeftButton
            && m_clipPathOfContent.contains(event->pos() - contentOffsetHint())) {
        if (!isEnableSystemMove())
            return;

        ///TODO: Warning: System move finished no mouse release event
        Utility::startWindowSystemMove(Utility::getNativeTopLevelWindow(winId()));
        m_isSystemMoveResizeState = true;

        return;
    }

    unsetCursor();

    if (!canResize())
        return;

    if (qApp->mouseButtons() != Qt::LeftButton && m_contentGeometry.contains(event->pos())) {
        return;
    }

    bool isFixedWidth = minimumWidth() == maximumWidth();
    bool isFixedHeight = minimumHeight() == maximumHeight();

    Utility::CornerEdge mouseCorner;
    QRect cornerRect;
    const QRect window_real_geometry = m_contentGeometry + QMargins(MOUSE_MARGINS, MOUSE_MARGINS, MOUSE_MARGINS, MOUSE_MARGINS);

    if (isFixedWidth || isFixedHeight)
        goto set_edge;

    /// begin set cursor corner type
    cornerRect.setSize(QSize(MOUSE_MARGINS * 2, MOUSE_MARGINS * 2));
    cornerRect.moveTopLeft(window_real_geometry.topLeft());

    if (cornerRect.contains(event->pos())) {
        mouseCorner = Utility::TopLeftCorner;

        goto set_cursor;
    }

    cornerRect.moveTopRight(window_real_geometry.topRight());

    if (cornerRect.contains(event->pos())) {
        mouseCorner = Utility::TopRightCorner;

        goto set_cursor;
    }

    cornerRect.moveBottomRight(window_real_geometry.bottomRight());

    if (cornerRect.contains(event->pos())) {
        mouseCorner = Utility::BottomRightCorner;

        goto set_cursor;
    }

    cornerRect.moveBottomLeft(window_real_geometry.bottomLeft());

    if (cornerRect.contains(event->pos())) {
        mouseCorner = Utility::BottomLeftCorner;

        goto set_cursor;
    }
set_edge:
    /// begin set cursor edge type
    if (event->x() <= m_contentGeometry.x()) {
        if (isFixedWidth)
            goto skip_set_cursor;

        mouseCorner = Utility::LeftEdge;
    } else if (event->x() < m_contentGeometry.right()) {
        if (isFixedHeight)
            goto skip_set_cursor;

        if (event->y() <= m_contentGeometry.y()) {
            mouseCorner = Utility::TopEdge;
        } else if (!isFixedWidth || event->y() >= m_contentGeometry.bottom()) {
            mouseCorner = Utility::BottomEdge;
        } else {
            goto skip_set_cursor;
        }
    } else if (!isFixedWidth && (!isFixedHeight || event->x() >= m_contentGeometry.right())) {
        mouseCorner = Utility::RightEdge;
    } else {
        goto skip_set_cursor;
    }
set_cursor:
    Utility::setWindowCursor(winId(), mouseCorner);

    if (qApp->mouseButtons() == Qt::LeftButton) {
        Utility::startWindowSystemResize(Utility::getNativeTopLevelWindow(winId()), mouseCorner);
        m_isSystemMoveResizeState = true;

        cancelAdsorbCursor();
    } else {
        adsorbCursor(mouseCorner);
    }

    return;
skip_set_cursor:
    setCursor(Qt::ArrowCursor);

    cancelAdsorbCursor();
    m_canAdsorbCursor = canResize();
}

void DFrameWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isSystemMoveResizeState) {
        Utility::cancelWindowMoveResize(Utility::getNativeTopLevelWindow(winId()));
        m_isSystemMoveResizeState = false;
    }

    return QPaintDeviceWindow::mouseReleaseEvent(event);
}

void DFrameWindow::resizeEvent(QResizeEvent *event)
{
    updateFrameMask();

    return QPaintDeviceWindow::resizeEvent(event);
}

bool DFrameWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        m_canAdsorbCursor = canResize();
        break;
    case QEvent::Leave:
        m_canAdsorbCursor = false;
        cancelAdsorbCursor();
        break;
    default:
        break;
    }

    return QPaintDeviceWindow::event(event);
}

void DFrameWindow::timerEvent(QTimerEvent *event)
{
    Q_D(DFrameWindow);

    if (event->timerId() == d->flushTimer) {
        killTimer(d->flushTimer);
        d->flushTimer = 0;

        if (!d->flushArea.isEmpty()) {
            platformBackingStore->flush(this, d->flushArea, QPoint(0, 0));
            d->flushArea = QRegion();
        }
    } else if (event->timerId() == m_paintShadowOnContentTimerId) {
        killTimer(m_paintShadowOnContentTimerId);
        m_paintShadowOnContentTimerId = -1;

        if (!m_contentWindow || !m_contentWindow->handle())
            return QPaintDeviceWindow::timerEvent(event);

        QRect rect = m_contentWindow->handle()->geometry();

        rect.setTopLeft(QPoint(0, 0));
        m_contentBackingStore->flush(m_contentWindow, rect, QPoint(0, 0));
    } else {
        return QPaintDeviceWindow::timerEvent(event);
    }
}

#ifdef Q_OS_LINUX
static cairo_format_t cairo_format_from_qimage_format (QImage::Format fmt)
{
    switch (fmt) {
    case QImage::Format_ARGB32_Premultiplied:
        return CAIRO_FORMAT_ARGB32;
    case QImage::Format_RGB32:
        return CAIRO_FORMAT_RGB24;
    case QImage::Format_Indexed8: // XXX not quite
        return CAIRO_FORMAT_A8;
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        return CAIRO_FORMAT_A1;
    case QImage::Format_RGB16:
        return CAIRO_FORMAT_RGB16_565;
    case QImage::Format_RGB30:
    case QImage::Format_A2RGB30_Premultiplied: // 忽略2 bits的alpha通道
        return CAIRO_FORMAT_RGB30;
    case QImage::Format_Invalid:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_RGB666:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_RGB555:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_RGB888:
    case QImage::Format_RGB444:
    case QImage::Format_ARGB4444_Premultiplied:
    case QImage::NImageFormats:
    default:
        return (cairo_format_t) -1;
    }
}
#endif

void DFrameWindow::updateFromContents(void *ev)
{
#ifdef Q_OS_LINUX
    if (Q_UNLIKELY(nativeWindowXPixmap == XCB_PIXMAP_NONE && xsurfaceDirtySize.isEmpty())) {
        return;
    }

    xcb_connection_t *connect = DPlatformIntegration::xcbConnection()->xcb_connection();
    xcb_damage_notify_event_t *event = reinterpret_cast<xcb_damage_notify_event_t*>(ev);
    xcb_xfixes_region_t parts = xcb_generate_id(connect);

    xcb_xfixes_create_region(connect, parts, 0, 0);
    xcb_damage_subtract(connect, event->damage, XCB_NONE, parts);

    xcb_xfixes_fetch_region_cookie_t cookie = xcb_xfixes_fetch_region(connect, parts);
    xcb_xfixes_fetch_region_reply_t *reply = xcb_xfixes_fetch_region_reply(connect, cookie, 0);

    if (Q_UNLIKELY(!reply))
        return;

    xcb_rectangle_t *rectangles = xcb_xfixes_fetch_region_rectangles(reply);
    int length = xcb_xfixes_fetch_region_rectangles_length(reply);

    if (!xsurfaceDirtySize.isEmpty())
        updateNativeWindowXPixmap(xsurfaceDirtySize.width(), xsurfaceDirtySize.height());

    drawNativeWindowXPixmap(rectangles, length);

    free(reply);
#endif
}

void DFrameWindow::drawShadowTo(QPaintDevice *device)
{
    const QPoint &offset = m_contentGeometry.topLeft() - contentOffsetHint();
    qreal device_pixel_ratio = devicePixelRatio();
    const QSize &size = handle()->geometry().size();
    QPainter pa(device);

    if (m_redirectContent) {
        QPainterPath clip_path;

        clip_path.addRect(QRect(QPoint(0, 0), size));
        clip_path -= m_clipPath;

        pa.setRenderHint(QPainter::Antialiasing);
        pa.setClipPath(clip_path);
    }

    pa.setCompositionMode(QPainter::CompositionMode_Source);

    if (!disableFrame() && Q_LIKELY(DXcbWMSupport::instance()->hasWindowAlpha())
            && !m_shadowImage.isNull()) {
        pa.drawImage(offset * device_pixel_ratio, m_shadowImage);
    }

    if (m_borderWidth > 0 && m_borderColor != Qt::transparent) {
        if (Q_LIKELY(DXcbWMSupport::instance()->hasWindowAlpha())) {
            pa.setRenderHint(QPainter::Antialiasing);
            pa.fillPath(m_borderPath, m_borderColor);
        } else {
            pa.fillRect(QRect(QPoint(0, 0), size), m_borderColor);
        }
    }

    pa.end();
}

QPaintDevice *DFrameWindow::redirected(QPoint *) const
{
    return platformBackingStore->paintDevice();
}

void DFrameWindow::setContentPath(const QPainterPath &path, bool isRoundedRect, int radius)
{
    if (m_clipPathOfContent == path)
        return;

    if (!isRoundedRect)
        m_contentGeometry = path.boundingRect().toRect().translated(contentOffsetHint());

    qreal device_pixel_ratio = devicePixelRatio();

    m_clipPathOfContent = path;
    m_clipPath = path.translated(contentOffsetHint()) * device_pixel_ratio;

    if (isRoundedRect && m_pathIsRoundedRect == isRoundedRect && m_roundedRectRadius == radius && !m_shadowImage.isNull()) {
        const QMargins margins(qMax(m_shadowRadius + radius + qAbs(m_shadowOffset.x()), m_borderWidth),
                               qMax(m_shadowRadius + radius + qAbs(m_shadowOffset.y()), m_borderWidth),
                               qMax(m_shadowRadius + radius + qAbs(m_shadowOffset.x()), m_borderWidth),
                               qMax(m_shadowRadius + radius + qAbs(m_shadowOffset.y()), m_borderWidth));
        const QSize &margins_size = margins2Size(margins);
        const QSize &shadow_size = m_shadowImage.size() / device_pixel_ratio;


        if (margins_size.width() > m_contentGeometry.width()
                || margins_size.height() > m_contentGeometry.height()
                || margins_size.width() >= shadow_size.width()
                || margins_size.height() >= shadow_size.height()) {
            updateShadowAsync();
        } else if (m_canUpdateShadow && !m_contentGeometry.isEmpty() && isVisible() && !disableFrame()) {
            m_shadowImage = Utility::borderImage(QPixmap::fromImage(m_shadowImage), margins * device_pixel_ratio,
                                                 (m_contentGeometry + contentMarginsHint()).size() * device_pixel_ratio);
        }
    } else {
        m_pathIsRoundedRect = isRoundedRect;
        m_roundedRectRadius = radius;

        updateShadowAsync();
    }

    updateMask();
}

#ifdef Q_OS_LINUX
void DFrameWindow::drawNativeWindowXPixmap(xcb_rectangle_t *rects, int length)
{
    if (Q_UNLIKELY(!nativeWindowXSurface))
        return;

    const QPoint offset(m_contentMarginsHint.left() * devicePixelRatio(), m_contentMarginsHint.top() * devicePixelRatio());

    const QImage &source_image = platformBackingStore->toImage();
    QImage image(const_cast<uchar*>(source_image.bits()),
                 source_image.width(), source_image.height(),
                 source_image.bytesPerLine(), source_image.format());
    const cairo_format_t format = cairo_format_from_qimage_format(image.format());
    cairo_surface_t *surface = __cairo->cairo_image_surface_create_for_data(image.bits(), format, image.width(), image.height(), image.bytesPerLine());
    cairo_t *cr = __cairo->cairo_create(surface);

    __cairo->cairo_surface_mark_dirty(nativeWindowXSurface);
    __cairo->cairo_set_source_rgb(cr, 0, 255, 0);
    __cairo->cairo_set_source_surface(cr, nativeWindowXSurface, offset.x(), offset.y());
    __cairo->cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

    bool clip = false;

    for (int i = 0; i < m_clipPath.elementCount(); ++i) {
        const QPainterPath::Element &e = m_clipPath.elementAt(i);

        switch (e.type) {
        case QPainterPath::MoveToElement:
            clip = true;
            __cairo->cairo_move_to(cr, e.x, e.y);
            break;
        case QPainterPath::LineToElement:
            clip = true;
            __cairo->cairo_line_to(cr, e.x, e.y);
            break;
        case QPainterPath::CurveToElement: {
            clip = true;

            const QPainterPath::Element &p2 = m_clipPath.elementAt(++i);
            const QPainterPath::Element &p3 = m_clipPath.elementAt(++i);

            __cairo->cairo_curve_to(cr, e.x, e.y, p2.x, p2.y, p3.x, p3.y);
            break;
        }
        default:
            break;
        }
    }

    if (clip) {
        __cairo->cairo_clip(cr);
    }

    if (rects) {
        for (int i = 0; i < length; ++i) {
            const xcb_rectangle_t &rect = rects[i];

            d_func()->flushArea += QRect(rect.x + offset.x(), rect.y + offset.y(), rect.width, rect.height);
            __cairo->cairo_rectangle(cr, rect.x + offset.x(), rect.y + offset.y(), rect.width, rect.height);
            __cairo->cairo_fill(cr);
        }
    } else {
        __cairo->cairo_paint(cr);

        // draw shadow
        drawShadowTo(&image);
        d_func()->flushArea = QRect(QPoint(0, 0), d_func()->size);
    }

    __cairo->cairo_destroy(cr);
    __cairo->cairo_surface_destroy(surface);

    d_func()->flush(QRegion());
}

bool DFrameWindow::updateNativeWindowXPixmap(int width, int height)
{
    if (Q_UNLIKELY(!m_contentWindow->handle()->isExposed()))
        return false;

    WId winId = static_cast<QXcbWindow*>(m_contentWindow->handle())->QXcbWindow::winId();

    qreal width_extra = (m_contentMarginsHint.left() + m_contentMarginsHint.right()) * devicePixelRatio();
    qreal height_extra = (m_contentMarginsHint.top() + m_contentMarginsHint.bottom()) * devicePixelRatio();
    d_func()->resize(QSize(width + qRound(width_extra), height + qRound(height_extra)));

    xcb_connection_t *connect = DPlatformIntegration::xcbConnection()->xcb_connection();

    if (Q_LIKELY(nativeWindowXPixmap != XCB_PIXMAP_NONE)) {
        xcb_free_pixmap(connect, nativeWindowXPixmap);
    } else {
        nativeWindowXPixmap = xcb_generate_id(connect);
    }

    xcb_void_cookie_t cookie = xcb_composite_name_window_pixmap_checked(connect, winId, nativeWindowXPixmap);
    QScopedPointer<xcb_generic_error_t, QScopedPointerPodDeleter> error(xcb_request_check(connect, cookie));

    if (error) {
        nativeWindowXPixmap = XCB_PIXMAP_NONE;
        return false;
    }

    if (Q_LIKELY(nativeWindowXSurface)) {
        __cairo->cairo_xlib_surface_set_drawable(nativeWindowXSurface, nativeWindowXPixmap, width, height);
    } else if (__cairo->isValid()) {
        Display *display = (Display*)DPlatformIntegration::xcbConnection()->xlib_display();

        XWindowAttributes attr;
        XGetWindowAttributes(display, winId, &attr);
        //###(zccrs): 只能使用cairo的xlib接口，使用xcb接口会导致从创建的surface中读取内容时不正确
        nativeWindowXSurface = __cairo->cairo_xlib_surface_create(display, nativeWindowXPixmap, attr.visual, attr.width, attr.height);
    }

    return true;
}

void DFrameWindow::markXPixmapToDirty(int width, int height)
{
    if (Q_UNLIKELY(width < 0 || height < 0)) {
        WId winId = static_cast<QXcbWindow*>(m_contentWindow->handle())->QXcbWindow::winId();

        const QRect rect = Utility::windowGeometry(winId);

        width = rect.width();
        height = rect.height();
    }

    xsurfaceDirtySize.setWidth(width);
    xsurfaceDirtySize.setHeight(height);
}
#endif

void DFrameWindow::updateShadow()
{
    if (!isVisible())
        return;

    if (!m_canUpdateShadow || m_contentGeometry.isEmpty() || disableFrame())
        return;

    qreal device_pixel_ratio = devicePixelRatio();
    QPixmap pixmap(m_contentGeometry.size() * device_pixel_ratio);

    if (pixmap.isNull())
        return;

    pixmap.fill(Qt::transparent);

    QPainter pa(&pixmap);

    pa.fillPath(m_clipPath.translated(-m_contentGeometry.topLeft() * device_pixel_ratio), m_shadowColor);
    pa.end();

    m_shadowImage = Utility::dropShadow(pixmap, m_shadowRadius * device_pixel_ratio, m_shadowColor);
    update();

    // 阴影更新后尝试刷新内部窗口
    if (m_contentBackingStore)
        m_paintShadowOnContentTimerId = startTimer(300, Qt::PreciseTimer);
}

void DFrameWindow::updateShadowAsync(int delaye)
{
    if (m_updateShadowTimer.isActive()) {
        return;
    }

    m_updateShadowTimer.setSingleShot(true);
    m_updateShadowTimer.start(delaye);
}

void DFrameWindow::updateContentMarginsHint(bool force)
{
    QMargins margins;

    margins = QMargins(qMax(m_shadowRadius - m_shadowOffset.x(), m_borderWidth),
                       qMax(m_shadowRadius - m_shadowOffset.y(), m_borderWidth),
                       qMax(m_shadowRadius + m_shadowOffset.x(), m_borderWidth),
                       qMax(m_shadowRadius + m_shadowOffset.y(), m_borderWidth));

    if (!force && margins == m_contentMarginsHint)
        return;

    qreal device_pixel_ratio = devicePixelRatio();
    Utility::setFrameExtents(winId(), margins * device_pixel_ratio);

    const QMargins old_margins = m_contentMarginsHint;
    m_contentMarginsHint = margins;
    m_contentGeometry.translate(m_contentMarginsHint.left() - old_margins.left(),
                                m_contentMarginsHint.top() - old_margins.top());

    m_clipPath = m_clipPathOfContent.translated(contentOffsetHint()) * device_pixel_ratio;

    qreal width_extra = (m_contentMarginsHint.left() + m_contentMarginsHint.right()) * device_pixel_ratio;
    qreal height_extra = (m_contentMarginsHint.top() + m_contentMarginsHint.bottom()) * device_pixel_ratio;
#ifdef Q_OS_LINUX
    if (nativeWindowXSurface) {
        int width = __cairo->cairo_xlib_surface_get_width(nativeWindowXSurface);
        int height = __cairo->cairo_xlib_surface_get_height(nativeWindowXSurface);

        d_func()->resize(QSize(width + qRound(width_extra), height + qRound(height_extra)));
    }
#endif

    updateShadow();
    updateMask();

    emit contentMarginsHintChanged(old_margins);
}

void DFrameWindow::updateMask()
{
    if (windowState() == Qt::WindowMinimized)
        return;

    if (disableFrame()) {
        QRegion region(m_contentGeometry * devicePixelRatio());
        Utility::setShapeRectangles(winId(), region, DWMSupport::instance()->hasWindowAlpha(), flags().testFlag(Qt::WindowTransparentForInput));

        return;
    }

    // Set window clip mask
    int mouse_margins;

    if (DWMSupport::instance()->hasWindowAlpha())
        mouse_margins = canResize() ? MOUSE_MARGINS : 0;
    else
        mouse_margins = qRound(m_borderWidth * devicePixelRatio());

    const QPainterPath &path = m_clipPath;

    if (m_enableAutoInputMaskByContentPath && (!m_pathIsRoundedRect || m_roundedRectRadius > 0)) {
        QPainterPath p;

        if (Q_LIKELY(mouse_margins > 0)) {
            QPainterPathStroker stroker;
            stroker.setJoinStyle(Qt::MiterJoin);
            stroker.setWidth(mouse_margins * 2);
            p = stroker.createStroke(path);
            p = p.united(path);
        } else {
            p = path;
        }

        Utility::setShapePath(winId(), p, DWMSupport::instance()->hasWindowAlpha(), flags().testFlag(Qt::WindowTransparentForInput));
    } else {
        QRegion region((m_contentGeometry * devicePixelRatio()).adjusted(-mouse_margins, -mouse_margins, mouse_margins, mouse_margins));
        Utility::setShapeRectangles(winId(), region, DWMSupport::instance()->hasWindowAlpha(), flags().testFlag(Qt::WindowTransparentForInput));
    }

    QPainterPathStroker stroker;

    stroker.setJoinStyle(Qt::MiterJoin);
    stroker.setWidth(m_borderWidth);
    m_borderPath = stroker.createStroke(path);

    updateFrameMask();
    update();
}

void DFrameWindow::updateFrameMask()
{
#ifdef Q_OS_LINUX
//    QXcbWindow *xw = static_cast<QXcbWindow*>(handle());

//    if (!xw || !xw->wmWindowTypes().testFlag(QXcbWindowFunctions::Dock))
//        return;

//    if (!m_enableAutoFrameMask || !DWMSupport::instance()->hasWindowAlpha())
//        return;

//    const QRect rect(QRect(QPoint(0, 0), size()));

//    QRegion region(rect.united((m_contentGeometry + contentMarginsHint()))  * devicePixelRatio());

    // ###(zccrs): xfwm4 window manager会自动给dock类型的窗口加上阴影， 所以在此裁掉窗口之外的内容
//    setMask(region);
#endif
}

bool DFrameWindow::canResize() const
{
    bool ok = m_enableSystemResize
            && !flags().testFlag(Qt::Popup)
            && !flags().testFlag(Qt::BypassWindowManagerHint)
            && minimumSize() != maximumSize()
            && !disableFrame();

#ifdef Q_OS_LINUX
    if (!ok)
        return false;

    quint32 hints = DXcbWMSupport::getMWMFunctions(Utility::getNativeTopLevelWindow(winId()));

    return (hints == DXcbWMSupport::MWM_FUNC_ALL || hints & DXcbWMSupport::MWM_FUNC_RESIZE);
#endif

    return ok;
}

void DFrameWindow::cancelAdsorbCursor()
{
    QSignalBlocker blocker(&m_startAnimationTimer);
    Q_UNUSED(blocker)
    m_startAnimationTimer.stop();
    m_cursorAnimation.stop();
}

void DFrameWindow::adsorbCursor(Utility::CornerEdge cornerEdge)
{
    m_lastCornerEdge = cornerEdge;

    if (!m_canAdsorbCursor)
        return;

    if (m_cursorAnimation.state() == QVariantAnimation::Running)
        return;

    m_startAnimationTimer.start();
}

void DFrameWindow::startCursorAnimation()
{
    const QPoint &cursorPos = qApp->primaryScreen()->handle()->cursor()->pos();
    QPoint toPos = cursorPos - handle()->geometry().topLeft();
    const QRect geometry = (m_contentGeometry * devicePixelRatioF()).adjusted(-2, -2, 2, 2);

    switch (m_lastCornerEdge) {
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

    toPos += handle()->geometry().topLeft();
    const QPoint &tmp = toPos - cursorPos;

    if (qAbs(tmp.x()) < 3 && qAbs(tmp.y()) < 3)
        return;

    m_canAdsorbCursor = false;

    m_cursorAnimation.setStartValue(cursorPos);
    m_cursorAnimation.setEndValue(toPos);
    m_cursorAnimation.start();
}

bool DFrameWindow::disableFrame() const
{
    return windowState() == Qt::WindowFullScreen
            || windowState() == Qt::WindowMaximized
            || windowState() == Qt::WindowMinimized;
}

void DFrameWindow::onDevicePixelRatioChanged()
{
    updateContentMarginsHint(true);
}

DPP_END_NAMESPACE

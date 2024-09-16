// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "utility.h"
#include "qxcbconnection.h"
#include "qxcbscreen.h"

#include "dplatformintegration.h"
#include "dxcbwmsupport.h"

#include <QPixmap>
#include <QPainter>
#include <QCursor>
#include <QDebug>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QtX11Extras/QX11Info>
#endif

#include <QGuiApplication>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformcursor.h>

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
#include <QtWidgets/qtwidgetsglobal.h>
#endif

#include <xcb/shape.h>
#include <xcb/xcb_icccm.h>

#include <X11/cursorfont.h>
#include <X11/Xlib.h>

#define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

#define XATOM_MOVE_RESIZE "_NET_WM_MOVERESIZE"
#define XDEEPIN_BLUR_REGION "_NET_WM_DEEPIN_BLUR_REGION"
#define XDEEPIN_BLUR_REGION_ROUNDED "_NET_WM_DEEPIN_BLUR_REGION_ROUNDED"
#define _GTK_SHOW_WINDOW_MENU "_GTK_SHOW_WINDOW_MENU"

QT_BEGIN_NAMESPACE
//extern Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

QImage Utility::dropShadow(const QPixmap &px, qreal radius, const QColor &color)
{
    if (px.isNull())
        return QImage();

    QImage tmp(px.size() + QSize(radius * 2, radius * 2), QImage::Format_ARGB32_Premultiplied);
    tmp.fill(0);
    QPainter tmpPainter(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tmpPainter.drawPixmap(QPoint(radius, radius), px);
    tmpPainter.end();

    // blur the alpha channel
    QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(0);
    QPainter blurPainter(&blurred);
    qt_blurImage(&blurPainter, tmp, radius, false, true);
    blurPainter.end();

    if (color == QColor(Qt::black))
        return blurred;

    tmp = blurred;

    // blacken the image...
    tmpPainter.begin(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tmpPainter.fillRect(tmp.rect(), color);
    tmpPainter.end();

    return tmp;
}

QList<QRect> Utility::sudokuByRect(const QRect &rect, QMargins borders)
{
    QList<QRect> list;

//    qreal border_width = borders.left() + borders.right();

//    if ( border_width > rect.width()) {
//        borders.setLeft(borders.left() / border_width * rect.width());
//        borders.setRight(rect.width() - borders.left());
//    }

//    qreal border_height = borders.top() + borders.bottom();

//    if (border_height > rect.height()) {
//        borders.setTop(borders.top()/ border_height * rect.height());
//        borders.setBottom(rect.height() - borders.top());
//    }

    const QRect &contentsRect = rect - borders;

    list << QRect(0, 0, borders.left(), borders.top());
    list << QRect(list.at(0).topRight(), QSize(contentsRect.width(), borders.top())).translated(1, 0);
    list << QRect(list.at(1).topRight(), QSize(borders.right(), borders.top())).translated(1, 0);
    list << QRect(list.at(0).bottomLeft(), QSize(borders.left(), contentsRect.height())).translated(0, 1);
    list << contentsRect;
    list << QRect(contentsRect.topRight(), QSize(borders.right(), contentsRect.height())).translated(1, 0);
    list << QRect(list.at(3).bottomLeft(), QSize(borders.left(), borders.bottom())).translated(0, 1);
    list << QRect(contentsRect.bottomLeft(), QSize(contentsRect.width(), borders.bottom())).translated(0, 1);
    list << QRect(contentsRect.bottomRight(), QSize(borders.left(), borders.bottom())).translated(1, 1);

    return list;
}

QImage Utility::borderImage(const QPixmap &px, const QMargins &borders,
                            const QSize &size, QImage::Format format)
{
    QImage image(size, format);
    QPainter pa(&image);

    const QList<QRect> sudoku_src = sudokuByRect(px.rect(), borders);
    const QList<QRect> sudoku_tar = sudokuByRect(QRect(QPoint(0, 0), size), borders);

    pa.setCompositionMode(QPainter::CompositionMode_Source);

    for (int i = 0; i < 9; ++i) {
        pa.drawPixmap(sudoku_tar[i], px, sudoku_src[i]);
    }

    pa.end();

    return image;
}

xcb_atom_t Utility::internAtom(xcb_connection_t *connection, const char *name, bool only_if_exists)
{
    if (!name || *name == 0)
        return XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, only_if_exists, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, 0);

    if (!reply)
        return XCB_NONE;

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

xcb_atom_t Utility::internAtom(const char *name, bool only_if_exists)
{
    return internAtom(QX11Info::connection(), name, only_if_exists);
}

void Utility::startWindowSystemMove(quint32 WId)
{
    sendMoveResizeMessage(WId, _NET_WM_MOVERESIZE_MOVE);
}

void Utility::cancelWindowMoveResize(quint32 WId)
{
    sendMoveResizeMessage(WId, _NET_WM_MOVERESIZE_CANCEL);
}

void Utility::updateMousePointForWindowMove(quint32 WId, bool finished/* = false*/)
{
    xcb_client_message_event_t xev;
    const QPoint &globalPos = qApp->primaryScreen()->handle()->cursor()->pos();

    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = internAtom("_DEEPIN_MOVE_UPDATE");
    xev.window = WId;
    xev.format = 32;
    xev.data.data32[0] = globalPos.x();
    xev.data.data32[1] = globalPos.y();
    // fix touch screen moving dtk window : 0 update position, 1 moving finished and clear moving state
    xev.data.data32[2] = finished ? 1 : 0;
    xev.data.data32[3] = 0;
    xev.data.data32[4] = 0;

    xcb_send_event(DPlatformIntegration::xcbConnection()->xcb_connection(),
                   false, DPlatformIntegration::xcbConnection()->rootWindow(),
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                   (const char *)&xev);

    xcb_flush(DPlatformIntegration::xcbConnection()->xcb_connection());
}

void Utility::showWindowSystemMenu(quint32 WId, QPoint globalPos)
{
    if (globalPos.isNull()) {
        globalPos = qApp->primaryScreen()->handle()->cursor()->pos();
    }

    xcb_client_message_event_t xev;

    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = internAtom(_GTK_SHOW_WINDOW_MENU);
    xev.window = WId;
    xev.format = 32;
    xev.data.data32[1] = globalPos.x();
    xev.data.data32[2] = globalPos.y();

    // ungrab鼠标，防止窗管的菜单窗口无法grab鼠标
    xcb_ungrab_pointer(QX11Info::connection(), XCB_CURRENT_TIME);
    xcb_send_event(QX11Info::connection(), false, QX11Info::appRootWindow(QX11Info::appScreen()),
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                   (const char *)&xev);

    xcb_flush(QX11Info::connection());
}

void Utility::setFrameExtents(WId wid, const QMargins &margins)
{
    xcb_atom_t frameExtents = internAtom("_GTK_FRAME_EXTENTS");

    if (frameExtents == XCB_NONE) {
        qWarning() << "Failed to create atom with name _GTK_FRAME_EXTENTS";
        return;
    }

    int32_t data[4];
    data[0] = int32_t(margins.left());
    data[1] = int32_t(margins.right());
    data[2] = int32_t(margins.top());
    data[3] = int32_t(margins.bottom());
    xcb_change_property_checked(QX11Info::connection(), XCB_PROP_MODE_REPLACE, xcb_window_t(wid), frameExtents, XCB_ATOM_CARDINAL, 32, 4, data);
}

static QVector<xcb_rectangle_t> qregion2XcbRectangles(const QRegion &region)
{
    QVector<xcb_rectangle_t> rectangles;

    rectangles.reserve(region.rectCount());
    for (auto rect = region.cbegin(); rect != region.cend(); ++rect) {
        xcb_rectangle_t r;

        r.x = rect->x();
        r.y = rect->y();
        r.width = rect->width();
        r.height = rect->height();

        rectangles << r;
    }

    return rectangles;
}

static void setShapeRectangles(quint32 WId, const QVector<xcb_rectangle_t> &rectangles, bool onlyInput, bool transparentInput = false)
{
    xcb_shape_mask(QX11Info::connection(), XCB_SHAPE_SO_SET,
                   XCB_SHAPE_SK_BOUNDING, WId, 0, 0, XCB_NONE);

    if (transparentInput) {
        xcb_shape_rectangles(QX11Info::connection(), XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT,
                             XCB_CLIP_ORDERING_YX_BANDED, WId, 0, 0, 0, 0);

        if (onlyInput)
            return;
    } else {
        xcb_shape_mask(QX11Info::connection(), XCB_SHAPE_SO_SET,
                       XCB_SHAPE_SK_INPUT, WId, 0, 0, XCB_NONE);
    }

    if (rectangles.isEmpty()) {
        return;
    }

    xcb_shape_rectangles(QX11Info::connection(), XCB_SHAPE_SO_SET, onlyInput ? XCB_SHAPE_SK_INPUT : XCB_SHAPE_SK_BOUNDING,
                         XCB_CLIP_ORDERING_YX_BANDED, WId, 0, 0, rectangles.size(), rectangles.constData());
}

void Utility::setShapeRectangles(quint32 WId, const QRegion &region, bool onlyInput, bool transparentInput)
{
    ::setShapeRectangles(WId, qregion2XcbRectangles(region), onlyInput, transparentInput);
}

void Utility::setShapePath(quint32 WId, const QPainterPath &path, bool onlyInput, bool transparentInput)
{
    if (path.isEmpty()) {
        return ::setShapeRectangles(WId, QVector<xcb_rectangle_t>(), onlyInput, transparentInput);
    }

    QVector<xcb_rectangle_t> rectangles;

    foreach(const QPolygonF &polygon, path.toFillPolygons()) {
        QRegion region(polygon.toPolygon());
        for (auto area = region.cbegin(); area != region.cend(); ++area) {
            xcb_rectangle_t rectangle;

            rectangle.x = area->x();
            rectangle.y = area->y();
            rectangle.width = area->width();
            rectangle.height = area->height();

            rectangles.append(std::move(rectangle));
        }
    }

    ::setShapeRectangles(WId, rectangles, onlyInput, transparentInput);
}

void Utility::sendMoveResizeMessage(quint32 WId, uint32_t action, QPoint globalPos, Qt::MouseButton qbutton)
{
    int xbtn = qbutton == Qt::LeftButton ? XCB_BUTTON_INDEX_1 :
               qbutton == Qt::RightButton ? XCB_BUTTON_INDEX_3 :
               XCB_BUTTON_INDEX_ANY;

    if (globalPos.isNull()) {
        globalPos = qApp->primaryScreen()->handle()->cursor()->pos();
    }

    xcb_client_message_event_t xev;

    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = internAtom(XATOM_MOVE_RESIZE);
    xev.window = WId;
    xev.format = 32;
    xev.data.data32[0] = globalPos.x();
    xev.data.data32[1] = globalPos.y();
    xev.data.data32[2] = action;
    xev.data.data32[3] = xbtn;
    xev.data.data32[4] = 0;

    if (action != _NET_WM_MOVERESIZE_CANCEL)
        xcb_ungrab_pointer(QX11Info::connection(), QX11Info::appTime());

    xcb_send_event(QX11Info::connection(), false, QX11Info::appRootWindow(QX11Info::appScreen()),
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                   (const char *)&xev);

    xcb_flush(QX11Info::connection());
}

QWindow *Utility::getWindowById(quint32 WId)
{
    for (QWindow *w : qApp->allWindows()) {
        if (w->handle() && w->handle()->winId() == WId) {
            return w;
        }
    }

    return Q_NULLPTR;
}

qreal Utility::getWindowDevicePixelRatio(quint32 WId)
{
    if (const QWindow *w = getWindowById(WId))
        return w->devicePixelRatio();

    return qApp->devicePixelRatio();
}

void Utility::startWindowSystemResize(quint32 WId, CornerEdge cornerEdge, const QPoint &globalPos)
{
    sendMoveResizeMessage(WId, cornerEdge, globalPos);
}

static xcb_cursor_t CornerEdge2Xcb_cursor_t(Utility::CornerEdge ce)
{
    switch (ce) {
    case Utility::TopEdge:
        return XC_top_side;
    case Utility::TopRightCorner:
        return XC_top_right_corner;
    case Utility::RightEdge:
        return XC_right_side;
    case Utility::BottomRightCorner:
        return XC_bottom_right_corner;
    case Utility::BottomEdge:
        return XC_bottom_side;
    case Utility::BottomLeftCorner:
        return XC_bottom_left_corner;
    case Utility::LeftEdge:
        return XC_left_side;
    case Utility::TopLeftCorner:
        return XC_top_left_corner;
    default:
        return XCB_CURSOR_NONE;
    }
}

bool Utility::setWindowCursor(quint32 WId, Utility::CornerEdge ce)
{
    const auto display = QX11Info::display();

    Cursor cursor = XCreateFontCursor(display, CornerEdge2Xcb_cursor_t(ce));

    if (!cursor) {
        qWarning() << "[ui]::setWindowCursor() call XCreateFontCursor() failed";
        return false;
    }

    const int result = XDefineCursor(display, WId, cursor);

    XFlush(display);

    return result == Success;
}

QRegion Utility::regionAddMargins(const QRegion &region, const QMargins &margins, const QPoint &offset)
{
    QRegion tmp;
    for(auto rect = region.cbegin(); rect != region.cend(); ++rect) {
        tmp += rect->translated(offset) + margins;
    }
    return tmp;
}

QByteArray Utility::windowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, quint32 len)
{
    QByteArray data;
    xcb_connection_t* conn = QX11Info::connection();
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, WId, propAtom, typeAtom, 0, len);
    xcb_generic_error_t* err = nullptr;
    xcb_get_property_reply_t* reply = xcb_get_property_reply(conn, cookie, &err);

    if (reply != nullptr) {
        len = xcb_get_property_value_length(reply);
        const char* buf = static_cast<const char*>(xcb_get_property_value(reply));
        data.append(buf, len);
        free(reply);
    }

    if (err != nullptr) {
        free(err);
    }

    return data;
}

void Utility::setWindowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, const void *data, quint32 len, uint8_t format)
{
    xcb_connection_t* conn = QX11Info::connection();
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, WId, propAtom, typeAtom, format, len, data);
    xcb_flush(conn);
}

void Utility::clearWindowProperty(quint32 WId, xcb_atom_t propAtom)
{
    xcb_delete_property_checked(QX11Info::connection(), WId, propAtom);
}

void Utility::setNoTitlebar(quint32 WId, bool on)
{
    quint8 value = on;
    setWindowProperty(WId, DXcbWMSupport::instance()->_deepin_no_titlebar, XCB_ATOM_CARDINAL, &value, 1, 8);

    // 默认为使用noborder属性的窗口强制打开窗口标题栏
    xcb_atom_t _deepin_force_decorate = internAtom("_DEEPIN_FORCE_DECORATE", false);
    if (on) {
        quint8 value = on;
        setWindowProperty(WId, _deepin_force_decorate, XCB_ATOM_CARDINAL, &value, 1, 8);
    } else {
        clearWindowProperty(WId, _deepin_force_decorate);
    }
}

void Utility::splitWindowOnScreen(quint32 WId, quint32 type)
{
    splitWindowOnScreenByType(WId, type, 1);
}

void Utility::splitWindowOnScreenByType(quint32 WId, quint32 position, quint32 type)
{
    xcb_client_message_event_t xev;

    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = internAtom("_DEEPIN_SPLIT_WINDOW", false);
    xev.window = WId;
    xev.format = 32;
    xev.data.data32[0] = position; /* enum class SplitType in kwin-dev, Left=0x1, Right=0x10, Top=0x100, Bottom=0x1000*/
    xev.data.data32[1] = position == 15 ? 0 : 1; /* 0: not preview 1: preview*/
    xev.data.data32[2] = type; /* 1:two splitting 2:three splitting 4:four splitting*/

    xcb_send_event(QX11Info::connection(), false, QX11Info::appRootWindow(QX11Info::appScreen()),
                   SubstructureNotifyMask, (const char *)&xev);
    xcb_flush(QX11Info::connection());
}

bool Utility::supportForSplittingWindow(quint32 WId)
{
    return Utility::supportForSplittingWindowByType(WId, 1);
}

// screenSplittingType: 0: can't splitting, 1:two splitting, 2: four splitting(includ three splitting)
bool Utility::supportForSplittingWindowByType(quint32 WId, quint32 screenSplittingType)
{
    auto propAtom = internAtom("_DEEPIN_NET_SUPPORTED");
    QByteArray data = windowProperty(WId, propAtom, XCB_ATOM_CARDINAL, 4);

    if (const char *cdata = data.constData())
        return *(reinterpret_cast<const quint8 *>(cdata)) >= screenSplittingType;

    return false;
}

bool Utility::setEnableBlurWindow(const quint32 WId, bool enable)
{
    if (!DXcbWMSupport::instance()->hasBlurWindow() || !DXcbWMSupport::instance()->isKwin())
        return false;

    xcb_atom_t atom = DXcbWMSupport::instance()->_kde_net_wm_blur_rehind_region_atom;

    if (atom == XCB_NONE)
        return false;

    clearWindowProperty(WId, DXcbWMSupport::instance()->_net_wm_deepin_blur_region_mask);

    if (enable) {
        quint32 value = enable;
        setWindowProperty(WId, atom, XCB_ATOM_CARDINAL, &value, 1, sizeof(quint32) * 8);
    } else {
        clearWindowProperty(WId, atom);
    }

    return true;
}

bool Utility::blurWindowBackground(const quint32 WId, const QVector<BlurArea> &areas)
{
    if (!DXcbWMSupport::instance()->hasBlurWindow())
        return false;

    if (DXcbWMSupport::instance()->isDeepinWM()) {
        xcb_atom_t atom = DXcbWMSupport::instance()->_net_wm_deepin_blur_region_rounded_atom;

        if (atom == XCB_NONE)
            return false;

        if (areas.isEmpty()) {
            QVector<BlurArea> areas;

            areas << BlurArea();
        }

        clearWindowProperty(WId, DXcbWMSupport::instance()->_net_wm_deepin_blur_region_mask);
        setWindowProperty(WId, atom, XCB_ATOM_CARDINAL, areas.constData(), areas.size() * sizeof(BlurArea) / sizeof(quint32), sizeof(quint32) * 8);
    } else {
        xcb_atom_t atom = DXcbWMSupport::instance()->_kde_net_wm_blur_rehind_region_atom;

        if (atom == XCB_NONE)
            return false;

        QVector<quint32> rects;

        foreach (const BlurArea &area, areas) {
            if (area.xRadius <= 0 || area.yRaduis <= 0) {
                rects << area.x << area.y << area.width << area.height;
            } else {
                QPainterPath path;

                path.addRoundedRect(area.x, area.y, area.width, area.height, area.xRadius, area.yRaduis);

                foreach(const QPolygonF &polygon, path.toFillPolygons()) {
                    QRegion region(polygon.toPolygon());
                    for(auto area = region.cbegin(); area != region.cend(); ++area) {
                        rects << area->x() << area->y() << area->width() << area->height();
                    }
                }
            }
        }

        clearWindowProperty(WId, DXcbWMSupport::instance()->_net_wm_deepin_blur_region_mask);

        if (!areas.isEmpty())
            setWindowProperty(WId, atom, XCB_ATOM_CARDINAL, rects.constData(), rects.size(), sizeof(quint32) * 8);
    }

    return true;
}

bool Utility::blurWindowBackgroundByPaths(const quint32 WId, const QList<QPainterPath> &paths)
{
    if (DXcbWMSupport::instance()->isDeepinWM()) {
        QRect boundingRect;

        for (const QPainterPath &p : paths) {
            boundingRect |= p.boundingRect().toRect();
        }

        QImage image(boundingRect.size(), QImage::Format_Alpha8);

        image.fill(Qt::transparent);

        QPainter painter(&image);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(-boundingRect.topLeft());

        for (const QPainterPath &p : paths) {
            painter.fillPath(p, Qt::black);
        }

        return blurWindowBackgroundByImage(WId, boundingRect, image);
    } else if (DXcbWMSupport::instance()->isKwin()) {
        xcb_atom_t atom = DXcbWMSupport::instance()->_kde_net_wm_blur_rehind_region_atom;

        if (atom == XCB_NONE)
            return false;

        if (paths.isEmpty()) {
            clearWindowProperty(WId, DXcbWMSupport::instance()->_net_wm_deepin_blur_region_mask);
            return true;
        }

        QVector<quint32> rects;

        foreach (const QPainterPath &path, paths) {
            foreach(const QPolygonF &polygon, path.toFillPolygons()) {
                QRegion region(polygon.toPolygon());
                for(auto area = region.cbegin(); area != region.cend(); ++area) {
                    rects << area->x() << area->y() << area->width() << area->height();
                }
            }
        }

        setWindowProperty(WId, atom, XCB_ATOM_CARDINAL, rects.constData(), rects.size(), sizeof(quint32) * 8);
    }

    return true;
}

bool Utility::blurWindowBackgroundByImage(const quint32 WId, const QRect &blurRect, const QImage &maskImage)
{
    if (!DXcbWMSupport::instance()->isDeepinWM() || maskImage.format() != QImage::Format_Alpha8)
        return false;

    QByteArray array;
    QVector<qint32> area;

    area.reserve(5);
    area << blurRect.x() << blurRect.y() << blurRect.width() << blurRect.height() << maskImage.bytesPerLine();
    array.reserve(area.size() * sizeof(qint32) / sizeof(char) * area.size() + maskImage.sizeInBytes());
    array.append((const char*)area.constData(), sizeof(qint32) / sizeof(char) * area.size());
    array.append((const char*)maskImage.constBits(), maskImage.sizeInBytes());

    clearWindowProperty(WId, DXcbWMSupport::instance()->_net_wm_deepin_blur_region_rounded_atom);
    setWindowProperty(WId, DXcbWMSupport::instance()->_net_wm_deepin_blur_region_mask,
                      DXcbWMSupport::instance()->_net_wm_deepin_blur_region_mask,
                      array.constData(), array.length(), 8);

    return true;
}

bool Utility::updateBackgroundWallpaper(const quint32 WId, const QRect &area, const quint32 bMode)
{
    xcb_atom_t atom = DXcbWMSupport::instance()->_deepin_wallpaper;

    if (atom == XCB_NONE)
        return false;

    QVector<quint32> rects;
    quint32 window_mode = (bMode & 0xffff0000) >> 16; //High 16 bits
    quint32 wallpaper_mode = bMode & 0x0000ffff;      //low 16 bits
    rects << area.x() << area.y() << area.width() << area.height() << window_mode << wallpaper_mode;

    setWindowProperty(WId, atom, XCB_ATOM_CARDINAL, rects.constData(), rects.size(), sizeof(quint32) * 8);

    return true;
}

void Utility::clearWindowBlur(const quint32 WId)
{
    clearWindowProperty(WId, DXcbWMSupport::instance()->_net_wm_deepin_blur_region_rounded_atom);
    clearWindowProperty(WId, DXcbWMSupport::instance()->_net_wm_deepin_blur_region_mask);
    clearWindowProperty(WId, DXcbWMSupport::instance()->_kde_net_wm_blur_rehind_region_atom);
}

void Utility::clearWindowBackground(const quint32 WId)
{
    clearWindowProperty(WId, DXcbWMSupport::instance()->_deepin_wallpaper);
}

qint32 Utility::getWorkspaceForWindow(quint32 WId)
{
    xcb_get_property_cookie_t cookie = xcb_get_property(DPlatformIntegration::xcbConnection()->xcb_connection(), false, WId,
                                                        Utility::internAtom("_NET_WM_DESKTOP"), XCB_ATOM_CARDINAL, 0, 1);
    QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter> reply(
        xcb_get_property_reply(DPlatformIntegration::xcbConnection()->xcb_connection(), cookie, NULL));
    if (reply && reply->type == XCB_ATOM_CARDINAL && reply->format == 32 && reply->value_len == 1) {
        return *(qint32*)xcb_get_property_value(reply.data());
    }

    return 0;
}

Utility::QtMotifWmHints Utility::getMotifWmHints(quint32 WId)
{
    xcb_connection_t *xcb_connect = DPlatformIntegration::xcbConnection()->xcb_connection();
    QtMotifWmHints hints;

    xcb_get_property_cookie_t get_cookie =
        xcb_get_property_unchecked(xcb_connect, 0, WId, DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS)),
                                   DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS)), 0, 20);

    xcb_get_property_reply_t *reply =
        xcb_get_property_reply(xcb_connect, get_cookie, NULL);

    if (reply && reply->format == 32 && reply->type == DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS))) {
        hints = *((QtMotifWmHints *)xcb_get_property_value(reply));
    } else {
        hints.flags = 0L;
        hints.functions = DXcbWMSupport::MWM_FUNC_ALL;
        hints.decorations = DXcbWMSupport::MWM_DECOR_ALL;
        hints.input_mode = 0L;
        hints.status = 0L;
    }

    free(reply);

    return hints;
}

void Utility::setMotifWmHints(quint32 WId, Utility::QtMotifWmHints hints)
{
    if (hints.flags != 0l) {
        // 如果标志为设置了xxx_all标志，则其它标志位的设置无任何意义
        // 反而会导致窗管忽略xxx_all标志，因此此处重设此标志位
        if (hints.functions & DXcbWMSupport::MWM_FUNC_ALL) {
            hints.functions = DXcbWMSupport::MWM_FUNC_ALL;
        }

        if (hints.decorations & DXcbWMSupport::MWM_DECOR_ALL) {
            hints.decorations = DXcbWMSupport::MWM_DECOR_ALL;
        }

#ifdef Q_XCB_CALL2
        Q_XCB_CALL2(xcb_change_property(DPlatformIntegration::xcbConnection()->xcb_connection(),
                                        XCB_PROP_MODE_REPLACE,
                                        WId,
                                        DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS)),
                                        DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS)),
                                        32,
                                        5,
                                        &hints), c);
#else
        xcb_change_property(DPlatformIntegration::xcbConnection()->xcb_connection(),
                            XCB_PROP_MODE_REPLACE,
                            WId,
                            DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS)),
                            DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS)),
                            32,
                            5,
                            &hints);
#endif
    } else {
#ifdef Q_XCB_CALL2
        Q_XCB_CALL2(xcb_delete_property(DPlatformIntegration::xcbConnection()->xcb_connection(), WId,
                                        DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS))),
                                        DPlatformIntegration::xcbConnection()->xcb_connection());
#else
        xcb_delete_property(DPlatformIntegration::xcbConnection()->xcb_connection(), WId,
                            DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS)));
#endif
    }
}

quint32 Utility::getNativeTopLevelWindow(quint32 WId)
{
    xcb_connection_t *xcb_connection = DPlatformIntegration::xcbConnection()->xcb_connection();

    do {
        xcb_query_tree_cookie_t cookie = xcb_query_tree_unchecked(xcb_connection, WId);
        QScopedPointer<xcb_query_tree_reply_t, QScopedPointerPodDeleter> reply(xcb_query_tree_reply(xcb_connection, cookie, NULL));

        if (reply) {
            if (reply->parent == reply->root)
                break;

            QtMotifWmHints hints = getMotifWmHints(reply->parent);

            if (hints.flags == 0)
                break;

            hints = getMotifWmHints(WId);

            if ((hints.decorations & DXcbWMSupport::MWM_DECOR_BORDER) == DXcbWMSupport::MWM_DECOR_BORDER)
                break;

            WId = reply->parent;
        } else {
            break;
        }
    } while (true);

    return WId;
}

QPoint Utility::translateCoordinates(const QPoint &pos, quint32 src, quint32 dst)
{
    QPoint ret;
    xcb_translate_coordinates_cookie_t cookie =
        xcb_translate_coordinates(DPlatformIntegration::xcbConnection()->xcb_connection(), src, dst,
                                  pos.x(), pos.y());
    xcb_translate_coordinates_reply_t *reply =
        xcb_translate_coordinates_reply(DPlatformIntegration::xcbConnection()->xcb_connection(), cookie, NULL);
    if (reply) {
        ret.setX(reply->dst_x);
        ret.setY(reply->dst_y);
        free(reply);
    }

    return ret;
}

QRect Utility::windowGeometry(quint32 WId)
{
    xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(
            DPlatformIntegration::xcbConnection()->xcb_connection(),
            xcb_get_geometry(DPlatformIntegration::xcbConnection()->xcb_connection(), WId),
            NULL);

    QRect rect;

    if (geom) {
        // --
        // add the border_width for the window managers frame... some window managers
        // do not use a border_width of zero for their frames, and if we the left and
        // top strut, we ensure that pos() is absolutely correct.  frameGeometry()
        // will still be incorrect though... perhaps i should have foffset as well, to
        // indicate the frame offset (equal to the border_width on X).
        // - Brad
        // -- copied from qwidget_x11.cpp

        rect = QRect(geom->x, geom->y, geom->width, geom->height);

        free(geom);
    }

    return rect;
}

quint32 Utility::clientLeader()
{
    return DPlatformIntegration::xcbConnection()->clientLeader();
}

quint32 Utility::createGroupWindow()
{
    QXcbConnection *connection = DPlatformIntegration::xcbConnection();
    uint32_t group_leader = xcb_generate_id(connection->xcb_connection());
    QXcbScreen *screen = connection->primaryScreen();
    xcb_create_window(connection->xcb_connection(),
                      XCB_COPY_FROM_PARENT,
                      group_leader,
                      screen->root(),
                      0, 0, 1, 1,
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      screen->screen()->root_visual,
                      0, 0);
//#ifndef QT_NO_DEBUG
    QByteArray ba("Qt(dxcb) group leader window");
    xcb_change_property(connection->xcb_connection(),
                        XCB_PROP_MODE_REPLACE,
                        group_leader,
                        connection->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_NAME)),
                        connection->atom(QXcbAtom::D_QXCBATOM_WRAPPER(UTF8_STRING)),
                        8,
                        ba.length(),
                        ba.constData());
//#endif
    xcb_change_property(connection->xcb_connection(),
                        XCB_PROP_MODE_REPLACE,
                        group_leader,
                        connection->atom(QXcbAtom::D_QXCBATOM_WRAPPER(WM_CLIENT_LEADER)),
                        XCB_ATOM_WINDOW,
                        32,
                        1,
                        &group_leader);

#if !defined(QT_NO_SESSIONMANAGER) && defined(XCB_USE_SM)
    // If we are session managed, inform the window manager about it
    QByteArray session = qGuiApp->sessionId().toLatin1();
    if (!session.isEmpty()) {
        xcb_change_property(connection->xcb_connection(),
                            XCB_PROP_MODE_REPLACE,
                            group_leader,
                            connection->atom(QXcbAtom::D_QXCBATOM_WRAPPER(SM_CLIENT_ID)),
                            XCB_ATOM_STRING,
                            8,
                            session.length(),
                            session.constData());
    }
#endif

    // 将group leader的group leader设置为client leader
    setWindowGroup(group_leader, connection->clientLeader());

    return group_leader;
}

void Utility::destoryGroupWindow(quint32 groupLeader)
{
    xcb_destroy_window(DPlatformIntegration::xcbConnection()->xcb_connection(), groupLeader);
}

void Utility::setWindowGroup(quint32 window, quint32 groupLeader)
{
    window = getNativeTopLevelWindow(window);

    QXcbConnection *connection = DPlatformIntegration::xcbConnection();

    xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_hints_unchecked(connection->xcb_connection(), window);
    xcb_icccm_wm_hints_t hints;
    xcb_icccm_get_wm_hints_reply(connection->xcb_connection(), cookie, &hints, NULL);

    if (groupLeader > 0) {
        xcb_icccm_wm_hints_set_window_group(&hints, groupLeader);
    } else {
        hints.flags &= (~XCB_ICCCM_WM_HINT_WINDOW_GROUP);
    }

    xcb_icccm_set_wm_hints(connection->xcb_connection(), window, &hints);
}

int Utility::XIconifyWindow(void *display, quint32 w, int screen_number)
{
    return ::XIconifyWindow(reinterpret_cast<Display*>(display), w, screen_number);
}

QVector<uint> Utility::getWindows()
{
    return DXcbWMSupport::instance()->allWindow();
}

quint32 Utility::windowFromPoint(const QPoint &p)
{
    return DXcbWMSupport::instance()->windowFromPoint(p);
}

QVector<uint> Utility::getCurrentWorkspaceWindows()
{
    qint32 current_workspace = 0;

    xcb_get_property_cookie_t cookie = xcb_get_property(DPlatformIntegration::xcbConnection()->xcb_connection(), false,
                                                        DPlatformIntegration::xcbConnection()->rootWindow(),
                                                        Utility::internAtom("_NET_CURRENT_DESKTOP"), XCB_ATOM_CARDINAL, 0, 1);
    QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter> reply(
        xcb_get_property_reply(DPlatformIntegration::xcbConnection()->xcb_connection(), cookie, NULL));
    if (reply && reply->type == XCB_ATOM_CARDINAL && reply->format == 32 && reply->value_len == 1) {
        current_workspace = *(qint32*)xcb_get_property_value(reply.data());
    }

    QVector<uint> windows;

    foreach (quint32 WId, getWindows()) {
        qint32 ws = getWorkspaceForWindow(WId);

        if (ws < 0 || ws == current_workspace) {
            windows << WId;
        }
    }

    return windows;
}

DPP_END_NAMESPACE

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const Utility::BlurArea &area)
{
    QDebugStateSaver saver(deg);
    Q_UNUSED(saver)

    deg.setAutoInsertSpaces(true);
    deg << "x:" << area.x
        << "y:" << area.y
        << "width:" << area.width
        << "height:" << area.height
        << "xRadius:" << area.xRadius
        << "yRadius:" << area.yRaduis;

    return deg;
}
QT_END_NAMESPACE

// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include <QGuiApplication>
#undef private

#include "dplatformintegration.h"
#include "global.h"
#include "dforeignplatformwindow.h"
#include "vtablehook.h"
#include "dwmsupport.h"
#include "dnotitlebarwindowhelper.h"
#include "dnativesettings.h"
#include "dbackingstoreproxy.h"
#include "ddesktopinputselectioncontrol.h"
#include "dapplicationeventmonitor.h"

#ifdef USE_NEW_IMPLEMENTING
#include "dplatformwindowhelper.h"
#include "dplatformbackingstorehelper.h"
#include "dplatformopenglcontexthelper.h"
#include "dframewindow.h"
#else
#include "dplatformbackingstore.h"
#include "dplatformwindowhook.h"
#endif

#ifdef Q_OS_LINUX
#define private public
#include "qxcbcursor.h"
#include "qxcbdrag.h"
#undef private

#include "windoweventhook.h"
#include "xcbnativeeventfilter.h"
#include "dplatformnativeinterfacehook.h"
#include "dplatforminputcontexthook.h"
#include "dxcbxsettings.h"
#include "dhighdpi.h"

#include "qxcbscreen.h"

#include <X11/cursorfont.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_renderutil.h>
#endif

#include <QWidget>
#include <QGuiApplication>
#include <QLibrary>
#include <QDrag>
#include <QStyleHints>

#include <private/qguiapplication_p.h>
#define protected public
#include <private/qsimpledrag_p.h>
#undef protected
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatformservices.h>
#include <private/qpaintengine_raster_p.h>

#include "im_interface.h"
#include "dbus_interface.h"

// https://www.freedesktop.org/wiki/Specifications/XSettingsRegistry/
#define XSETTINGS_CURSOR_BLINK QByteArrayLiteral("Net/CursorBlink")
#define XSETTINGS_CURSOR_BLINK_TIME QByteArrayLiteral("Net/CursorBlinkTime")
#define XSETTINGS_DOUBLE_CLICK_TIME QByteArrayLiteral("Net/DoubleClickTime")

Q_LOGGING_CATEGORY(lcDxcb, "dtk.qpa.dxcb", QtInfoMsg)

class DQPaintEngine : public QPaintEngine
{
public:
    inline void clearFeatures(const QPaintEngine::PaintEngineFeatures &f)
    {
        gccaps &= ~f;
    }
};

DPP_BEGIN_NAMESPACE

#ifdef Q_OS_LINUX
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
DPlatformIntegration *DPlatformIntegration::m_instance = Q_NULLPTR;
#endif
#endif

DXcbXSettings *DPlatformIntegration::m_xsettings = nullptr;
DPlatformIntegration::DPlatformIntegration(const QStringList &parameters, int &argc, char **argv)
    : DPlatformIntegrationParent(parameters, argc, argv)
#ifdef USE_NEW_IMPLEMENTING
    , m_storeHelper(new DPlatformBackingStoreHelper)
    , m_contextHelper(new DPlatformOpenGLContextHelper)
    , m_pApplicationEventMonitor(nullptr)
#endif
{
#ifdef Q_OS_LINUX
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    m_instance = this;
#endif
#endif

    VtableHook::overrideVfptrFun(nativeInterface(),
                                 &QPlatformNativeInterface::platformFunction,
                                 &DPlatformNativeInterfaceHook::platformFunction);

    // 不仅仅需要在插件被加载时初始化, 也有可能DPlatformIntegration会被创建多次, 也应当在
    // DPlatformIntegration每次被创建时都重新初始化DHighDpi.
    DHighDpi::init();
}

DPlatformIntegration::~DPlatformIntegration()
{
    sendEndStartupNotifition();

#ifdef Q_OS_LINUX
    if (m_eventFilter) {
        qApp->removeNativeEventFilter(m_eventFilter);
        delete m_eventFilter;
    }
#endif

#ifdef USE_NEW_IMPLEMENTING
    delete m_storeHelper;
    delete m_contextHelper;

    if (m_xsettings) {
        delete m_xsettings;
        m_xsettings = nullptr;
    }
#endif
}

void DPlatformIntegration::setWindowProperty(QWindow *window, const char *name, const QVariant &value)
{
    if (isEnableNoTitlebar(window)) {
        DNoTitlebarWindowHelper::setWindowProperty(window, name, value);
    } else if (isEnableDxcb(window)) {
        DPlatformWindowHelper::setWindowProperty(window, name, value);
    }
}

bool DPlatformIntegration::enableDxcb(QWindow *window)
{
    qCDebug(lcDxcb) << "window:" << window << "window type:" << window->type() << "parent:" << window->parent();

    if (window->type() == Qt::Desktop)
        return false;

    QNativeWindow *xw = static_cast<QNativeWindow*>(window->handle());

    if (!xw) {
        window->setProperty(useDxcb, true);

        return true;
    }

#ifndef USE_NEW_IMPLEMENTING
    return false;
#endif

    if (DPlatformWindowHelper::mapped.value(xw))
        return true;

    if (xw->isExposed())
        return false;

    if (!DPlatformWindowHelper::windowRedirectContent(window)) {
        QPlatformBackingStore *store = reinterpret_cast<QPlatformBackingStore*>(qvariant_cast<quintptr>(window->property("_d_dxcb_BackingStore")));

        if (!store)
            return false;

        QSurfaceFormat format = window->format();

        const int oldAlpha = format.alphaBufferSize();
        const int newAlpha = 8;

        if (oldAlpha != newAlpha) {
            format.setAlphaBufferSize(newAlpha);
            window->setFormat(format);

            // 由于窗口alpha值的改变，需要重新创建x widnow
            xw->create();
        }

        DPlatformWindowHelper *helper = new DPlatformWindowHelper(xw);
        instance()->m_storeHelper->addBackingStore(store);
        helper->m_frameWindow->m_contentBackingStore = store;
    } else {
        Q_UNUSED(new DPlatformWindowHelper(xw))
    }

    window->setProperty(useDxcb, true);
    window->setProperty("_d_dxcb_TransparentBackground", window->format().hasAlpha());

    return true;
}

bool DPlatformIntegration::isEnableDxcb(const QWindow *window)
{
    return window->property(useDxcb).toBool();
}

bool DPlatformIntegration::setEnableNoTitlebar(QWindow *window, bool enable)
{
    if (enable && DNoTitlebarWindowHelper::mapped.value(window))
        return true;

    qCDebug(lcDxcb) << "enable titlebar:" << enable << "window:" << window
    << "window type:" << window->type() << "parent:" << window->parent();

    if (enable) {
        if (window->type() == Qt::Desktop)
            return false;

        if (!DWMSupport::instance()->hasNoTitlebar())
            return false;

        QNativeWindow *xw = static_cast<QNativeWindow*>(window->handle());
        window->setProperty(noTitlebar, true);

        if (!xw) {
            return true;
        }

        Utility::setNoTitlebar(xw->winId(), true);
        // 跟随窗口被销毁
        Q_UNUSED(new DNoTitlebarWindowHelper(window, xw->winId()))
    } else {
        if (auto helper = DNoTitlebarWindowHelper::mapped.value(window)) {
            Utility::setNoTitlebar(window->winId(), false);
            helper->deleteLater();
        }

        window->setProperty(noTitlebar, QVariant());
    }

    return true;
}

bool DPlatformIntegration::isEnableNoTitlebar(const QWindow *window)
{
    return window->property(noTitlebar).toBool();
}

bool DPlatformIntegration::buildNativeSettings(QObject *object, quint32 settingWindow)
{
    QByteArray settings_property = DNativeSettings::getSettingsProperty(object);
    DXcbXSettings *settings = nullptr;
    bool global_settings = false;
    if (settingWindow || !settings_property.isEmpty()) {
        settings = new DXcbXSettings(DPlatformIntegration::xcbConnection()->xcb_connection(), settingWindow, settings_property);
    } else {
        global_settings = true;
        settings = DPlatformIntegration::instance()->xSettings();
    }

    // 跟随object销毁
    auto native_settings = new DNativeSettings(object, settings, global_settings);

    if (!native_settings->isValid()) {
        delete native_settings;
        return false;
    }

    return true;
}

void DPlatformIntegration::clearNativeSettings(quint32 settingWindow)
{
#ifdef Q_OS_LINUX
    DXcbXSettings::clearSettings(settingWindow);
#endif
}

void DPlatformIntegration::setWMClassName(const QByteArray &name)
{
    if (auto self = instance())
        self->m_wmClass = name;
}

QPlatformWindow *DPlatformIntegration::createPlatformWindow(QWindow *window) const
{
    qCDebug(lcDxcb) << "window:" << window << "window type:" << window->type() << "parent:" << window->parent();

    if (qEnvironmentVariableIsSet("DXCB_PRINT_WINDOW_CREATE")) {
        printf("New Window: %s(0x%llx, name: \"%s\")\n", window->metaObject()->className(), (quintptr)window, qPrintable(window->objectName()));
    }

    // handle foreign native window
    if (window->type() == Qt::ForeignWindow) {
        WId win_id = qvariant_cast<WId>(window->property("_q_foreignWinId"));

        if (win_id > 0) {
            return new DForeignPlatformWindow(window, win_id);
        }
    }

    bool isNoTitlebar = window->type() != Qt::Desktop && window->property(noTitlebar).toBool();

    if (isNoTitlebar && DWMSupport::instance()->hasNoTitlebar()) {
        // 销毁旧的helper对象, 此处不用将mapped的值移除，后面会被覆盖
        if (DNoTitlebarWindowHelper *helper = DNoTitlebarWindowHelper::mapped.value(window)) {
            delete helper;
        }

        QPlatformWindow *w = DPlatformIntegrationParent::createPlatformWindow(window);
        Utility::setNoTitlebar(w->winId(), true);
        // 跟随窗口被销毁
        Q_UNUSED(new DNoTitlebarWindowHelper(window, w->winId()))
#ifdef Q_OS_LINUX
        WindowEventHook::init(static_cast<QNativeWindow*>(w), false);
#endif
        // for hi dpi
        if (DHighDpi::overrideBackingStore()) {
            bool ok = VtableHook::overrideVfptrFun(w, &QPlatformWindow::devicePixelRatio, &DHighDpi::devicePixelRatio);

            if (ok) {
                window->setProperty("_d_dxcb_overrideBackingStore", true);
            }
        }

        return w;
    }

    bool isUseDxcb = window->type() != Qt::Desktop && window->property(useDxcb).toBool();

    if (isUseDxcb) {
        QSurfaceFormat format = window->format();

        const int oldAlpha = format.alphaBufferSize();
        const int newAlpha = 8;

        window->setProperty("_d_dxcb_TransparentBackground", format.hasAlpha());

        if (!DPlatformWindowHelper::windowRedirectContent(window)) {
            if (oldAlpha != newAlpha) {
                format.setAlphaBufferSize(newAlpha);
                window->setFormat(format);
            }
        }
    }

    QPlatformWindow *w = DPlatformIntegrationParent::createPlatformWindow(window);
    QNativeWindow *xw = static_cast<QNativeWindow*>(w);

    if (isUseDxcb) {
#ifdef USE_NEW_IMPLEMENTING
        Q_UNUSED(new DPlatformWindowHelper(xw))
#else
        Q_UNUSED(new DPlatformWindowHook(xw))
#endif
    }

#ifdef Q_OS_LINUX
    const DFrameWindow *frame_window = qobject_cast<DFrameWindow*>(window);
    bool rc = false;

    if (isUseDxcb) {
        rc = frame_window ? DPlatformWindowHelper::windowRedirectContent(frame_window->m_contentWindow)
                          : DPlatformWindowHelper::windowRedirectContent(window);
    }

    WindowEventHook::init(xw, rc);
#endif

//    QWindow *tp_for_window = window->transientParent();

//    if (tp_for_window) {
//        // reset transient parent
//        if (DPlatformWindowHelper *helper = DPlatformWindowHelper::mapped.value(tp_for_window->handle())) {
//            window->setTransientParent(helper->m_frameWindow);
//        }
//    }

    if (window->type() != Qt::Desktop && !frame_window) {
        if (window->property(groupLeader).isValid()) {
            Utility::setWindowGroup(w->winId(), qvariant_cast<quint32>(window->property(groupLeader)));
        }
#ifdef Q_OS_LINUX
        else {
            Utility::setWindowGroup(w->winId(), xcbConnection()->clientLeader());
        }
#endif

        // for hi dpi
        if (!isUseDxcb && DHighDpi::overrideBackingStore()
                && (window->surfaceType() == QWindow::RasterSurface
                    || dynamic_cast<QPaintDevice*>(window)
                    || window->inherits("QWidgetWindow"))
                && !window->property("_d_dxcb_BackingStore").isValid()) {
            bool ok = VtableHook::overrideVfptrFun(w, &QPlatformWindow::devicePixelRatio, &DHighDpi::devicePixelRatio);

            if (ok) {
                window->setProperty("_d_dxcb_overrideBackingStore", true);
            }
        }
    }

    if (DBackingStoreProxy::useWallpaperPaint(window)) {
        QPair<QRect, int> wallpaper_pair = window->property("_d_dxcb_wallpaper").value<QPair<QRect, int>>();
        Utility::updateBackgroundWallpaper(w->winId(), wallpaper_pair.first, wallpaper_pair.second);
    }

    return xw;
}

QPlatformBackingStore *DPlatformIntegration::createPlatformBackingStore(QWindow *window) const
{
    qCDebug(lcDxcb) << "window:" << window << "window type:" << window->type() << "parent:" << window->parent();

    QPlatformBackingStore *store = DPlatformIntegrationParent::createPlatformBackingStore(window);
    bool useGLPaint = DBackingStoreProxy::useGLPaint(window);
    bool useWallpaper = DBackingStoreProxy::useWallpaperPaint(window);

    if (useGLPaint || useWallpaper || window->property("_d_dxcb_overrideBackingStore").toBool()) {
        // delegate of BackingStore for hidpi
        store = new DBackingStoreProxy(store, useGLPaint, useWallpaper);
        qInfo() << __FUNCTION__ << "enabled override backing store for:" << window;
    }

    if (window->type() == Qt::Desktop)
        return store;

    window->setProperty("_d_dxcb_BackingStore", reinterpret_cast<quintptr>(store));

    if (window->property(useDxcb).toBool())
#ifdef USE_NEW_IMPLEMENTING
        if (!DPlatformWindowHelper::windowRedirectContent(window)) {
            m_storeHelper->addBackingStore(store);

            if (DPlatformWindowHelper *helper = DPlatformWindowHelper::mapped.value(window->handle())) {
                helper->m_frameWindow->m_contentBackingStore = store;
            }
        }
#else
        return new DPlatformBackingStore(window, static_cast<QXcbBackingStore*>(store));
#endif

    return store;
}

QPlatformOpenGLContext *DPlatformIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    QPlatformOpenGLContext *p_context = DPlatformIntegrationParent::createPlatformOpenGLContext(context);

//    m_contextHelper->addOpenGLContext(context, p_context);

    return p_context;
}

QPaintEngine *DPlatformIntegration::createImagePaintEngine(QPaintDevice *paintDevice) const
{
    static QPaintEngine::PaintEngineFeatures disable_features = QPaintEngine::PaintEngineFeatures(-1);

    if (int(disable_features) < 0) {
        disable_features = QPaintEngine::PaintEngineFeatures();

        QByteArray data = qgetenv("DXCB_PAINTENGINE_DISABLE_FEATURES");

        do {
            if (!data.isEmpty()) {
                bool ok = false;
                disable_features = QPaintEngine::PaintEngineFeatures(data.toInt(&ok, 16));

                if (ok)
                    break;

                disable_features = QPaintEngine::PaintEngineFeatures();
            }

            QSettings settings(QSettings::IniFormat, QSettings::UserScope, "deepin", "qt-theme");

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            settings.setIniCodec("utf-8");
#endif
            settings.beginGroup("Platform");

            bool ok = false;

            disable_features = QPaintEngine::PaintEngineFeatures(settings.value("PaintEngineDisableFeatures").toByteArray().toInt(&ok, 16));

            if (!ok)
                disable_features = QPaintEngine::PaintEngineFeatures();
        } while (false);
    }

    QPaintEngine *base_engine = DPlatformIntegrationParent::createImagePaintEngine(paintDevice);

    if (disable_features == 0)
        return base_engine;

    if (!base_engine)
        base_engine = new QRasterPaintEngine(paintDevice);

    DQPaintEngine *engine = static_cast<DQPaintEngine*>(base_engine);

    engine->clearFeatures(disable_features);

    return engine;
}

QStringList DPlatformIntegration::themeNames() const
{
    QStringList list = DPlatformIntegrationParent::themeNames();
    const QByteArray desktop_session = qgetenv("DESKTOP_SESSION");

    // 在lightdm环境中，无此环境变量
    if (desktop_session.isEmpty() || desktop_session.startsWith("deepin"))
        list.prepend("deepin");

    return list;
}

#define GET_VALID_XSETTINGS(key) { \
    auto value = xSettings()->setting(key); \
    if (value.isValid()) return value; \
}

QVariant DPlatformIntegration::styleHint(QPlatformIntegration::StyleHint hint) const
{
#ifdef Q_OS_LINUX
    switch ((int)hint) {
    case CursorFlashTime:
        if (enableCursorBlink()) {
            GET_VALID_XSETTINGS(XSETTINGS_CURSOR_BLINK_TIME);
            break;
        } else {
            return 0;
        }
    case MouseDoubleClickInterval:
        GET_VALID_XSETTINGS(XSETTINGS_DOUBLE_CLICK_TIME);
        break;
    case ShowShortcutsInContextMenus:
        return false;
    default:
        break;
    }
#endif

    return DPlatformIntegrationParent::styleHint(hint);
}

#ifdef Q_OS_LINUX
typedef int (*PtrXcursorLibraryLoadCursor)(void *, const char *);
typedef char *(*PtrXcursorLibraryGetTheme)(void *);
typedef int (*PtrXcursorLibrarySetTheme)(void *, const char *);
typedef int (*PtrXcursorLibraryGetDefaultSize)(void *);

#if defined(XCB_USE_XLIB) && !defined(QT_NO_LIBRARY)
#include <X11/Xlib.h>
enum {
    XCursorShape = CursorShape
};
#undef CursorShape

static PtrXcursorLibraryLoadCursor ptrXcursorLibraryLoadCursor = 0;
static PtrXcursorLibraryGetTheme ptrXcursorLibraryGetTheme = 0;
static PtrXcursorLibrarySetTheme ptrXcursorLibrarySetTheme = 0;
static PtrXcursorLibraryGetDefaultSize ptrXcursorLibraryGetDefaultSize = 0;

static xcb_font_t cursorFont = 0;
#endif

static int cursorIdForShape(int cshape)
{
    int cursorId = 0;
    switch (cshape) {
    case Qt::ArrowCursor:
        cursorId = XC_left_ptr;
        break;
    case Qt::UpArrowCursor:
        cursorId = XC_center_ptr;
        break;
    case Qt::CrossCursor:
        cursorId = XC_crosshair;
        break;
    case Qt::WaitCursor:
        cursorId = XC_watch;
        break;
    case Qt::IBeamCursor:
        cursorId = XC_xterm;
        break;
    case Qt::SizeAllCursor:
        cursorId = XC_fleur;
        break;
    case Qt::PointingHandCursor:
        cursorId = XC_hand2;
        break;
    case Qt::SizeBDiagCursor:
        cursorId = XC_top_right_corner;
        break;
    case Qt::SizeFDiagCursor:
        cursorId = XC_bottom_right_corner;
        break;
    case Qt::SizeVerCursor:
    case Qt::SplitVCursor:
        cursorId = XC_sb_v_double_arrow;
        break;
    case Qt::SizeHorCursor:
    case Qt::SplitHCursor:
        cursorId = XC_sb_h_double_arrow;
        break;
    case Qt::WhatsThisCursor:
        cursorId = XC_question_arrow;
        break;
    case Qt::ForbiddenCursor:
        cursorId = XC_circle;
        break;
    case Qt::BusyCursor:
        cursorId = XC_watch;
        break;
    default:
        break;
    }
    return cursorId;
}

static const char * const cursorNames[] = {
    "left_ptr",
    "up_arrow",
    "cross",
    "wait",
    "ibeam",
    "size_ver",
    "size_hor",
    "size_bdiag",
    "size_fdiag",
    "size_all",
    "blank",
    "split_v",
    "split_h",
    "pointing_hand",
    "forbidden",
    "whats_this",
    "left_ptr_watch",
    "openhand",
    "closedhand",
    "copy",
    "move",
    "link"
};

static xcb_cursor_t loadCursor(void *dpy, int cshape)
{
    xcb_cursor_t cursor = XCB_NONE;
    if (!ptrXcursorLibraryLoadCursor || !dpy)
        return cursor;
    switch (cshape) {
    case Qt::DragCopyCursor:
        cursor = ptrXcursorLibraryLoadCursor(dpy, "dnd-copy");
        break;
    case Qt::DragMoveCursor:
        cursor = ptrXcursorLibraryLoadCursor(dpy, "dnd-move");
        break;
    case Qt::DragLinkCursor:
        cursor = ptrXcursorLibraryLoadCursor(dpy, "dnd-link");
        break;
    default:
        break;
    }
    if (!cursor) {
        cursor = ptrXcursorLibraryLoadCursor(dpy, cursorNames[cshape]);
    }
    return cursor;
}

static xcb_cursor_t qt_xcb_createCursorXRender(QXcbScreen *screen, const QImage &image,
                                        const QPoint &spot)
{
#ifdef XCB_USE_RENDER
    xcb_connection_t *conn = screen->xcb_connection();
    const int w = image.width();
    const int h = image.height();
    xcb_generic_error_t *error = 0;
    xcb_render_query_pict_formats_cookie_t formatsCookie = xcb_render_query_pict_formats(conn);
    xcb_render_query_pict_formats_reply_t *formatsReply = xcb_render_query_pict_formats_reply(conn,
                                                                                              formatsCookie,
                                                                                              &error);
    if (!formatsReply || error) {
        qWarning("qt_xcb_createCursorXRender: query_pict_formats failed");
        free(formatsReply);
        free(error);
        return XCB_NONE;
    }
    xcb_render_pictforminfo_t *fmt = xcb_render_util_find_standard_format(formatsReply,
                                                                          XCB_PICT_STANDARD_ARGB_32);
    if (!fmt) {
        qWarning("qt_xcb_createCursorXRender: Failed to find format PICT_STANDARD_ARGB_32");
        free(formatsReply);
        return XCB_NONE;
    }

    QImage img = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    xcb_image_t *xi = xcb_image_create(w, h, XCB_IMAGE_FORMAT_Z_PIXMAP,
                                       32, 32, 32, 32,
                                       QSysInfo::ByteOrder == QSysInfo::BigEndian ? XCB_IMAGE_ORDER_MSB_FIRST : XCB_IMAGE_ORDER_LSB_FIRST,
                                       XCB_IMAGE_ORDER_MSB_FIRST,
                                       0, 0, 0);
    if (!xi) {
        qWarning("qt_xcb_createCursorXRender: xcb_image_create failed");
        free(formatsReply);
        return XCB_NONE;
    }
    xi->data = (uint8_t *) malloc(xi->stride * h);
    if (!xi->data) {
        qWarning("qt_xcb_createCursorXRender: Failed to malloc() image data");
        xcb_image_destroy(xi);
        free(formatsReply);
        return XCB_NONE;
    }

    memcpy(xi->data, img.constBits(), img.sizeInBytes());

    xcb_pixmap_t pix = xcb_generate_id(conn);
    xcb_create_pixmap(conn, 32, pix, screen->root(), w, h);

    xcb_render_picture_t pic = xcb_generate_id(conn);
    xcb_render_create_picture(conn, pic, pix, fmt->id, 0, 0);

    xcb_gcontext_t gc = xcb_generate_id(conn);
    xcb_create_gc(conn, gc, pix, 0, 0);
    xcb_image_put(conn, pix, gc, xi, 0, 0, 0);
    xcb_free_gc(conn, gc);

    xcb_cursor_t cursor = xcb_generate_id(conn);
    xcb_render_create_cursor(conn, cursor, pic, spot.x(), spot.y());

    free(xi->data);
    xcb_image_destroy(xi);
    xcb_render_free_picture(conn, pic);
    xcb_free_pixmap(conn, pix);
    free(formatsReply);
    return cursor;

#else
    Q_UNUSED(screen);
    Q_UNUSED(image);
    Q_UNUSED(spot);
    return XCB_NONE;
#endif
}

static uint8_t cur_blank_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static xcb_cursor_t overrideCreateNonStandardCursor(QXcbCursor *xcb_cursor, Qt::CursorShape cshape, QWindow *window)
{
    xcb_cursor_t cursor = 0;
    xcb_connection_t *conn = xcb_cursor->xcb_connection();
    QImage image;

    switch (cshape) {
    case Qt::BlankCursor: {
        xcb_pixmap_t cp = xcb_create_pixmap_from_bitmap_data(conn, xcb_cursor->m_screen->root(), cur_blank_bits, 16, 16,
                                                             1, 0, 0, 0);
        xcb_pixmap_t mp = xcb_create_pixmap_from_bitmap_data(conn, xcb_cursor->m_screen->root(), cur_blank_bits, 16, 16,
                                                             1, 0, 0, 0);
        cursor = xcb_generate_id(conn);
        xcb_create_cursor(conn, cursor, cp, mp, 0, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF, 8, 8);

        return cursor;
    }
    case Qt::SizeVerCursor:
        image.load(":/bottom_side.png");
        break;
    case Qt::SizeAllCursor:
        image.load(":/all-scroll.png");
        break;
    case Qt::SplitVCursor:
        image.load(":/sb_v_double_arrow.png");
        break;
    case Qt::SplitHCursor:
        image.load(":/sb_h_double_arrow.png");
        break;
    case Qt::WhatsThisCursor:
        image.load(":/question_arrow.png");
        break;
    case Qt::BusyCursor:
        image.load(":/left_ptr_watch_0001.png");
        break;
    case Qt::ForbiddenCursor:
        image.load(":/crossed_circle.png");
        break;
    case Qt::OpenHandCursor:
        image.load(":/hand1.png");
        break;
    case Qt::ClosedHandCursor:
        image.load(":/grabbing.png");
        break;
    case Qt::DragCopyCursor:
        image.load(":/dnd-copy.png");
        break;
    case Qt::DragMoveCursor:
        image.load(":/dnd-move.png");
        break;
    case Qt::DragLinkCursor:
        image.load(":/dnd-link.png");
        break;
    default:
        break;
    }


    if (!image.isNull()) {
        image = image.scaledToWidth(24 * window->devicePixelRatio());
        cursor = qt_xcb_createCursorXRender(xcb_cursor->m_screen, image, QPoint(8, 8) * window->devicePixelRatio());
    }

    return cursor;
}

static bool updateCursorTheme(void *dpy)
{
    if (!ptrXcursorLibraryGetTheme
            || !ptrXcursorLibrarySetTheme)
        return false;
    QByteArray theme = ptrXcursorLibraryGetTheme(dpy);

    int setTheme = ptrXcursorLibrarySetTheme(dpy,theme.constData());
    return setTheme;
}

static xcb_cursor_t overrideCreateFontCursor(QXcbCursor *xcb_cursor, QCursor *c, QWindow *window)
{
    const Qt::CursorShape cshape = c->shape();
    xcb_connection_t *conn = xcb_cursor->xcb_connection();
    int cursorId = cursorIdForShape(cshape);
    xcb_cursor_t cursor = XCB_NONE;

    // Try Xcursor first
#if defined(XCB_USE_XLIB) && !defined(QT_NO_LIBRARY)
    if (cshape >= 0 && cshape <= Qt::LastCursor) {
        void *dpy = xcb_cursor->connection()->xlib_display();
        // special case for non-standard dnd-* cursors
        cursor = loadCursor(dpy, cshape);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        if (!cursor && !xcb_cursor->m_callbackForPropertyRegistered) {
#else
        if (!cursor && !xcb_cursor->m_gtkCursorThemeInitialized) {
#endif
            VtableHook::callOriginalFun(xcb_cursor, &QPlatformCursor::changeCursor, c, window);

            if (updateCursorTheme(dpy)) {
                cursor = loadCursor(dpy, cshape);
            }
        }
    }
    if (cursor)
        return cursor;
    if (!cursor && cursorId) {
#ifdef DISPLAY_FROM_XCB
        cursor = XCreateFontCursor(DISPLAY_FROM_XCB(xcb_cursor), cursorId);
#else
        cursor = XCreateFontCursor(static_cast<Display *>(xcb_cursor->connection()->xlib_display()), cursorId);
#endif
        if (cursor)
            return cursor;
    }

#endif

    // Non-standard X11 cursors are created from bitmaps
    cursor = overrideCreateNonStandardCursor(xcb_cursor, cshape, window);

    // Create a glyph cursor if everything else failed
    if (!cursor && cursorId) {
        cursor = xcb_generate_id(conn);
        xcb_create_glyph_cursor(conn, cursor, cursorFont, cursorFont,
                                cursorId, cursorId + 1,
                                0xFFFF, 0xFFFF, 0xFFFF, 0, 0, 0);
    }

    if (cursor && cshape >= 0 && cshape < Qt::LastCursor && xcb_cursor->connection()->hasXFixes()) {
        const char *name = cursorNames[cshape];
        xcb_xfixes_set_cursor_name(conn, cursor, strlen(name), name);
    }

    return cursor;
}

static void overrideChangeCursor(QPlatformCursor *cursorHandle, QCursor * cursor, QWindow * widget)
{
    QXcbWindow *w = nullptr;
    if (widget && widget->handle())
        w = static_cast<QXcbWindow *>(widget->handle());
    else
        // No X11 cursor control when there is no widget under the cursor
        return;

    if (widget->property(disableOverrideCursor).toBool())
        return;

#ifdef D_ENABLE_CURSOR_HOOK
    // set cursor size scale
    static bool xcursrSizeIsSet = qEnvironmentVariableIsSet("XCURSOR_SIZE");

    if (!xcursrSizeIsSet)
        qputenv("XCURSOR_SIZE", QByteArray::number(24 * qApp->devicePixelRatio()));

    QXcbCursor *xcb_cursor = static_cast<QXcbCursor*>(cursorHandle);

    xcb_cursor_t c = XCB_CURSOR_NONE;
    if (cursor && cursor->shape() != Qt::BitmapCursor) {
        const QXcbCursorCacheKey key(cursor->shape());
        QXcbCursor::CursorHash::iterator it = xcb_cursor->m_cursorHash.find(key);
        if (it == xcb_cursor->m_cursorHash.end()) {
            it = xcb_cursor->m_cursorHash.insert(key, overrideCreateFontCursor(xcb_cursor, cursor, widget));
        }
        c = it.value();
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 1)
        w->setCursor(c);
#elif QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
        w->setCursor(c, false);
#else
        xcb_change_window_attributes(DPlatformIntegration::xcbConnection()->xcb_connection(), w->xcb_window(), XCB_CW_CURSOR, &c);
        xcb_flush(DPlatformIntegration::xcbConnection()->xcb_connection());
#endif
    }
#endif // D_ENABLE_CURSOR_HOOK

    VtableHook::callOriginalFun(cursorHandle, &QPlatformCursor::changeCursor, cursor, widget);
}

static void hookXcbCursor(QScreen *screen)
{
    // 解决切换光标主题，光标没有适配当前主题，显示错乱的问题
    if (screen && screen->handle())
         VtableHook::overrideVfptrFun(screen->handle()->cursor(), &QPlatformCursor::changeCursor, &overrideChangeCursor);
}
#endif // Q_OS_LINUX

static bool hookDragObjectEventFilter(QBasicDrag *drag, QObject *o, QEvent *e)
{
    if (e->type() == QEvent::MouseMove) {
        return drag->QBasicDrag::eventFilter(o, e);
    }

    return VtableHook::callOriginalFun(drag, &QObject::eventFilter, o, e);
}

#ifdef Q_OS_LINUX
typedef QXcbScreen QNativeScreen;
#elif defined(Q_OS_WIN)
typedef QWindowsScreen QNativeScreen;
#endif

QWindow *overrideTopLevelAt(QPlatformScreen *s, const QPoint &point)
{
    QWindow *window = static_cast<QNativeScreen*>(s)->QNativeScreen::topLevelAt(point);

    if (DFrameWindow *fw = qobject_cast<DFrameWindow*>(window)) {
        return fw->contentWindow();
    }

    return window;
}

static void hookScreenGetWindow(QScreen *screen)
{
    if (screen && screen->handle())
        VtableHook::overrideVfptrFun(screen->handle(), &QPlatformScreen::topLevelAt, &overrideTopLevelAt);
}

static void watchScreenDPIChange(QScreen *screen)
{
    if (screen && screen->handle()) {
        DPlatformIntegration::instance()->xSettings()->registerCallbackForProperty("Qt/DPI/" + screen->name().toLocal8Bit(), &DHighDpi::onDPIChanged, screen);
    } else {
        qWarning("screen or handle is nullptr!");
    }
}

static void startDrag(QXcbDrag *drag)
{
    VtableHook::callOriginalFun(drag, &QXcbDrag::startDrag);

    QVector<xcb_atom_t> support_actions;
    const Qt::DropActions actions = drag->currentDrag()->supportedActions();

    if (actions.testFlag(Qt::CopyAction))
        support_actions << drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionCopy));

    if (actions.testFlag(Qt::MoveAction))
        support_actions << drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionMove));

    if (actions.testFlag(Qt::LinkAction))
        support_actions << drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionLink));

    // 此处不能直接 return ,因为一个拖拽包含多种 actions 后再次拖拽单一 action 时需要将 property 更新, 否则单一 actoin 的拖拽会被强行改成
    // 上一次的 actions, 导致某些小问题 (比如文管的拖拽行为不一致的问题)
    //    if (support_actions.size() < 2)
    //        return;
#if QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
    xcb_change_property(drag->xcb_connection(), XCB_PROP_MODE_REPLACE, drag->connection()->clipboard()->m_owner,
                        drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionList)), XCB_ATOM_ATOM, sizeof(xcb_atom_t) * 8,
                        support_actions.size(), support_actions.constData());
#else
    xcb_change_property(drag->xcb_connection(), XCB_PROP_MODE_REPLACE, drag->connection()->clipboard()->m_requestor, //TODO: m_ower deleted, replaced by m_requestor ?
                        drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionList)), XCB_ATOM_ATOM, sizeof(xcb_atom_t) * 8,
                        support_actions.size(), support_actions.constData());
#endif
    xcb_flush(drag->xcb_connection());
}

void DPlatformIntegration::initialize()
{
    // 由于Qt很多代码中写死了是xcb，所以只能伪装成是xcb
    // FIXME: set platform_name to xcb to avoid webengine crash
    // It need dtk update tag, make it default when dtk udate
    if (qgetenv("DXCB_FAKE_PLATFORM_NAME_XCB") != "0") {
        *QGuiApplicationPrivate::platform_name = "xcb";
    }
    qApp->setProperty("_d_isDxcb", true);

    QXcbIntegration::initialize();

    // 配置opengl渲染模块类型
    QByteArray opengl_module_type = qgetenv("D_OPENGL_MODULE_TYPE");
    if(!opengl_module_type.isEmpty()) {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();
        QSurfaceFormat::RenderableType module = opengl_module_type.contains("gles") ? QSurfaceFormat::OpenGLES :
                                                                                      QSurfaceFormat::OpenGL;
        format.setRenderableType(module);
        QSurfaceFormat::setDefaultFormat(format);
    }

#ifdef Q_OS_LINUX
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    m_eventFilter = new XcbNativeEventFilter(connection());
#else
    m_eventFilter = new XcbNativeEventFilter(defaultConnection());
#endif
    qApp->installNativeEventFilter(m_eventFilter);

    if (!qEnvironmentVariableIsSet("DXCB_DISABLE_HOOK_CURSOR")) {
#if defined(XCB_USE_XLIB) && !defined(QT_NO_LIBRARY)
        static bool function_ptrs_not_initialized = true;
        if (function_ptrs_not_initialized) {
            QLibrary xcursorLib(QLatin1String("Xcursor"), 1);
            bool xcursorFound = xcursorLib.load();
            if (!xcursorFound) { // try without the version number
                xcursorLib.setFileName(QLatin1String("Xcursor"));
                xcursorFound = xcursorLib.load();
            }
            if (xcursorFound) {
                ptrXcursorLibraryLoadCursor =
                        (PtrXcursorLibraryLoadCursor) xcursorLib.resolve("XcursorLibraryLoadCursor");
                ptrXcursorLibraryGetTheme =
                        (PtrXcursorLibraryGetTheme) xcursorLib.resolve("XcursorGetTheme");
                ptrXcursorLibrarySetTheme =
                        (PtrXcursorLibrarySetTheme) xcursorLib.resolve("XcursorSetTheme");
                ptrXcursorLibraryGetDefaultSize =
                        (PtrXcursorLibraryGetDefaultSize) xcursorLib.resolve("XcursorGetDefaultSize");
            }
            function_ptrs_not_initialized = false;
        }
#endif

//#ifdef D_ENABLE_CURSOR_HOOK
        for (QScreen *s : qApp->screens()) {
            hookXcbCursor(s);
        }

        QObject::connect(qApp, &QGuiApplication::screenAdded, qApp, &hookXcbCursor);
//#endif // D_ENABLE_CURSOR_HOOK
    }

    VtableHook::overrideVfptrFun(xcbConnection()->drag(), &QXcbDrag::startDrag, &startDrag);
#endif

    VtableHook::overrideVfptrFun(qApp->d_func(), &QGuiApplicationPrivate::isWindowBlocked,
                                 this, &DPlatformIntegration::isWindowBlockedHandle);

    // FIXME(zccrs): 修复启动drag后鼠标从一块屏幕移动到另一块后图标窗口位置不对
    VtableHook::overrideVfptrFun(static_cast<QBasicDrag *>(drag()), &QBasicDrag::eventFilter, &hookDragObjectEventFilter);

    for (QScreen *s : qApp->screens()) {
        hookScreenGetWindow(s);

        if (DHighDpi::isActive()) {
            // 监听屏幕dpi变化
            watchScreenDPIChange(s);
        }
    }

    QObject::connect(qApp, &QGuiApplication::screenAdded, qApp, &hookScreenGetWindow);

    if (DHighDpi::isActive()) {
        // 监听屏幕dpi变化
        QObject::connect(qApp, &QGuiApplication::screenAdded, qApp, &watchScreenDPIChange);
    }

    if (QGuiApplicationPrivate::instance()->platformIntegration()->services()->desktopEnvironment().toLower().endsWith("tablet")) {
        m_pApplicationEventMonitor.reset(new DApplicationEventMonitor);

        QObject::connect(m_pApplicationEventMonitor.data(), &DApplicationEventMonitor::lastInputDeviceTypeChanged, qApp, [this] {
            // 这里为了不重复对g_desktopInputSelectionControl 做初始化设定, 做一个exists判定
            if (!m_pDesktopInputSelectionControl && m_pApplicationEventMonitor->lastInputDeviceType() == DApplicationEventMonitor::TouchScreen) {
                m_pDesktopInputSelectionControl.reset(new DDesktopInputSelectionControl(nullptr, qApp->inputMethod()));
                m_pDesktopInputSelectionControl->createHandles();
                m_pDesktopInputSelectionControl->setApplicationEventMonitor(m_pApplicationEventMonitor.data());
            }
        });

        // 适配虚拟键盘
        if (DPlatformInputContextHook::instance()->isValid()) {
            inputContextHookFunc();
        } else {
            // 虚拟键盘服务未启动时,开始监听DBUS
            // 待虚拟键盘启动后 Hook QPlatformInputContext函数
            // 该连接将一直存在防止使用中虚拟键盘服务重启
            OrgFreedesktopDBusInterface *interface = new OrgFreedesktopDBusInterface("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus(), qApp);
            QObject::connect(interface, &OrgFreedesktopDBusInterface::NameOwnerChanged, qApp, [ = ](const QString &in0, const QString &in1, const QString &in2){
                Q_UNUSED(in1)
                Q_UNUSED(in2)

                if (in0 == "com.deepin.im") {
                    inputContextHookFunc();
                    QObject::disconnect(interface, &OrgFreedesktopDBusInterface::NameOwnerChanged, nullptr, nullptr);
                    interface->deleteLater();
                }
            });
        }
    }
}

#ifdef Q_OS_LINUX
static void onXSettingsChanged(xcb_connection_t *connection, const QByteArray &name, const QVariant &property, void *handle)
{
    Q_UNUSED(connection)
    Q_UNUSED(property)
    Q_UNUSED(name)

    if (handle == reinterpret_cast<void*>(QPlatformIntegration::CursorFlashTime)) {
        // 由于QStyleHints中的属性值可能已经被自定义，因此不应该使用property中的值
        // 此处只表示 QStyleHints 的 cursorFlashTime 属性可能变了
        Q_EMIT qGuiApp->styleHints()->cursorFlashTimeChanged(qGuiApp->styleHints()->cursorFlashTime());
    }
}

bool DPlatformIntegration::enableCursorBlink() const
{
    auto value = xSettings()->setting(XSETTINGS_CURSOR_BLINK);
    bool ok = false;
    int enable = value.toInt(&ok);

    return !ok || enable;
}

DXcbXSettings *DPlatformIntegration::xSettings(bool onlyExists) const
{
    if (onlyExists)
        return m_xsettings;

    return xSettings(xcbConnection());
}

DXcbXSettings *DPlatformIntegration::xSettings(QXcbConnection *connection)
{
    if (!m_xsettings) {
        auto xsettings = new DXcbXSettings(connection->xcb_connection());
        m_xsettings = xsettings;

        // 注册回调，用于通知 QStyleHints 属性改变
        xsettings->registerCallbackForProperty(XSETTINGS_CURSOR_BLINK_TIME,
                                               onXSettingsChanged,
                                               reinterpret_cast<void*>(CursorFlashTime));
        xsettings->registerCallbackForProperty(XSETTINGS_CURSOR_BLINK,
                                               onXSettingsChanged,
                                               reinterpret_cast<void*>(CursorFlashTime));

        if (DHighDpi::isActive()) {
            // 监听XSettings的dpi设置变化
            xsettings->registerCallbackForProperty("Xft/DPI", &DHighDpi::onDPIChanged, nullptr);
        }
    }

    return m_xsettings;
}
#endif

bool DPlatformIntegration::isWindowBlockedHandle(QWindow *window, QWindow **blockingWindow)
{
    if (DFrameWindow *frame = qobject_cast<DFrameWindow*>(window)) {
        bool blocked = VtableHook::callOriginalFun(qApp->d_func(), &QGuiApplicationPrivate::isWindowBlocked, frame->m_contentWindow, blockingWindow);

        // NOTE(zccrs): 将内容窗口的blocked状态转移到frame窗口，否则会被QXcbWindow::relayFocusToModalWindow重复调用requestActivate而引起死循环
        if (blockingWindow && frame->m_contentWindow.data() == *blockingWindow) {
            *blockingWindow = window;
        }

        return blocked;
    }

    return VtableHook::callOriginalFun(qApp->d_func(), &QGuiApplicationPrivate::isWindowBlocked, window, blockingWindow);
}

void DPlatformIntegration::inputContextHookFunc()
{
//    if (!VtableHook::hasVtable(inputContext())) {
        VtableHook::overrideVfptrFun(inputContext(),
                                    &QPlatformInputContext::showInputPanel,
                                    &DPlatformInputContextHook::showInputPanel);
        VtableHook::overrideVfptrFun(inputContext(),
                                    &QPlatformInputContext::hideInputPanel,
                                    &DPlatformInputContextHook::hideInputPanel);
        VtableHook::overrideVfptrFun(inputContext(),
                                    &QPlatformInputContext::isInputPanelVisible,
                                    &DPlatformInputContextHook::isInputPanelVisible);
        VtableHook::overrideVfptrFun(inputContext(),
                                    &QPlatformInputContext::keyboardRect,
                                    &DPlatformInputContextHook::keyboardRect);

        QObject::connect(DPlatformInputContextHook::instance(), &ComDeepinImInterface::geometryChanged,
                             inputContext(), &QPlatformInputContext::emitKeyboardRectChanged);
        QObject::connect(DPlatformInputContextHook::instance(), &ComDeepinImInterface::imActiveChanged,
                             inputContext(), &QPlatformInputContext::emitInputPanelVisibleChanged);
//    }
}

void DPlatformIntegration::sendEndStartupNotifition()
{
    QByteArray message, startupid;
    if (QPlatformNativeInterface *ni = QGuiApplication::platformNativeInterface())
        startupid = (const char *)ni->nativeResourceForIntegration(QByteArrayLiteral("startupid"));

    if (startupid.isEmpty())
        return ;

    message =  QByteArrayLiteral("remove: ID=") + startupid;
    xcb_client_message_event_t ev;
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.format = 8;
    ev.type = xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_STARTUP_INFO_BEGIN));
    ev.sequence = 0;
    ev.window = xcbConnection()->rootWindow();
    int sent = 0;
    int length = message.length() + 1; // include NUL byte
    const char *data = message.constData();
    do {
        if (sent == 20)
            ev.type = xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_STARTUP_INFO));

        const int start = sent;
        const int numBytes = qMin(length - start, 20);
        memcpy(ev.data.data8, data + start, numBytes);
        xcb_send_event(xcbConnection()->xcb_connection(), false, xcbConnection()->rootWindow(), XCB_EVENT_MASK_PROPERTY_CHANGE, (const char *) &ev);

        sent += numBytes;
    } while (sent < length);
}

DPP_END_NAMESPACE

// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dxcbwmsupport.h"
#include "dplatformintegration.h"
#include "utility.h"
#include "dframewindow.h"

#include "qxcbconnection.h"
#define private public
#include "qxcbscreen.h"
#undef private
#include "qxcbwindow.h"
#include "3rdparty/clientwin.h"

DPP_BEGIN_NAMESPACE

class _DXcbWMSupport : public DXcbWMSupport {};

Q_GLOBAL_STATIC(_DXcbWMSupport, globalXWMS)

DXcbWMSupport::DXcbWMSupport()
{
    updateWMName(false);

    connect(this, &DXcbWMSupport::windowMotifWMHintsChanged, this, [this] (quint32 winId) {
        for (const DFrameWindow *frame : DFrameWindow::frameWindowList) {
            if (frame->m_contentWindow && frame->m_contentWindow->handle()
                    && static_cast<QXcbWindow*>(frame->m_contentWindow->handle())->QXcbWindow::winId() == winId) {
                if (frame->handle())
                    emit windowMotifWMHintsChanged(frame->handle()->winId());
                break;
            }
        }
    });
}

void DXcbWMSupport::updateWMName(bool emitSignal)
{
    _net_wm_deepin_blur_region_rounded_atom = Utility::internAtom(QT_STRINGIFY(_NET_WM_DEEPIN_BLUR_REGION_ROUNDED), false);
    _net_wm_deepin_blur_region_mask = Utility::internAtom(QT_STRINGIFY(_NET_WM_DEEPIN_BLUR_REGION_MASK), false);
    _kde_net_wm_blur_rehind_region_atom = Utility::internAtom(QT_STRINGIFY(_KDE_NET_WM_BLUR_BEHIND_REGION), false);
    _deepin_wallpaper = Utility::internAtom(QT_STRINGIFY(_DEEPIN_WALLPAPER), false);
    _deepin_wallpaper_shared_key = Utility::internAtom(QT_STRINGIFY(_DEEPIN_WALLPAPER_SHARED_MEMORY), false);
    _deepin_no_titlebar = Utility::internAtom(QT_STRINGIFY(_DEEPIN_NO_TITLEBAR), false);
    _deepin_scissor_window = Utility::internAtom(QT_STRINGIFY(_DEEPIN_SCISSOR_WINDOW), false);

    m_wmName.clear();

    xcb_connection_t *xcb_connection = DPlatformIntegration::xcbConnection()->xcb_connection();
    xcb_window_t root = DPlatformIntegration::xcbConnection()->primaryScreen()->root();

    xcb_get_property_reply_t *reply =
        xcb_get_property_reply(xcb_connection,
            xcb_get_property_unchecked(xcb_connection, false, root,
                             DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_SUPPORTING_WM_CHECK)),
                             XCB_ATOM_WINDOW, 0, 1024), NULL);

    if (reply && reply->format == 32 && reply->type == XCB_ATOM_WINDOW) {
        xcb_window_t windowManager = *((xcb_window_t *)xcb_get_property_value(reply));

        if (windowManager != XCB_WINDOW_NONE) {
            xcb_get_property_reply_t *windowManagerReply =
                xcb_get_property_reply(xcb_connection,
                    xcb_get_property_unchecked(xcb_connection, false, windowManager,
                                     DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_NAME)),
                                     DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(UTF8_STRING)), 0, 1024), NULL);
            if (windowManagerReply && windowManagerReply->format == 8
                    && windowManagerReply->type == DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(UTF8_STRING))) {
                m_wmName = QString::fromUtf8((const char *)xcb_get_property_value(windowManagerReply), xcb_get_property_value_length(windowManagerReply));
            }

            free(windowManagerReply);
        }
    }
    free(reply);

    m_isDeepinWM = (m_wmName == QStringLiteral("Mutter(DeepinGala)"));
    m_isKwin = !m_isDeepinWM && (m_wmName == QStringLiteral("KWin"));

    updateHasComposite();
    updateNetWMAtoms();
    updateRootWindowProperties();

    if (emitSignal)
        emit windowManagerChanged();
}

void DXcbWMSupport::updateNetWMAtoms()
{
    net_wm_atoms.clear();

    xcb_window_t root = DPlatformIntegration::xcbConnection()->primaryScreen()->root();
    int offset = 0;
    int remaining = 0;
    xcb_connection_t *xcb_connection = DPlatformIntegration::xcbConnection()->xcb_connection();

    do {
        xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection, false, root,
                                                            DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_SUPPORTED)),
                                                            XCB_ATOM_ATOM, offset, 1024);
        xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection, cookie, NULL);
        if (!reply)
            break;

        remaining = 0;

        if (reply->type == XCB_ATOM_ATOM && reply->format == 32) {
            int len = xcb_get_property_value_length(reply)/sizeof(xcb_atom_t);
            xcb_atom_t *atoms = (xcb_atom_t *)xcb_get_property_value(reply);
            int s = net_wm_atoms.size();
            net_wm_atoms.resize(s + len);
            memcpy(net_wm_atoms.data() + s, atoms, len*sizeof(xcb_atom_t));

            remaining = reply->bytes_after;
            offset += len;
        }

        free(reply);
    } while (remaining > 0);

    updateHasBlurWindow();
    updateHasNoTitlebar();
    updateHasScissorWindow();
    updateWallpaperEffect();
}

void DXcbWMSupport::updateRootWindowProperties()
{
    root_window_properties.clear();

    xcb_window_t root = DPlatformIntegration::xcbConnection()->primaryScreen()->root();
    xcb_connection_t *xcb_connection = DPlatformIntegration::xcbConnection()->xcb_connection();

    xcb_list_properties_cookie_t cookie = xcb_list_properties(xcb_connection, root);
    xcb_list_properties_reply_t *reply = xcb_list_properties_reply(xcb_connection, cookie, NULL);

    if (!reply)
        return;

    int len = xcb_list_properties_atoms_length(reply);
    xcb_atom_t *atoms = (xcb_atom_t *)xcb_list_properties_atoms(reply);
    root_window_properties.resize(len);
    memcpy(root_window_properties.data(), atoms, len * sizeof(xcb_atom_t));

    free(reply);

    updateHasBlurWindow();
}

void DXcbWMSupport::updateHasBlurWindow()
{
    bool hasBlurWindow((m_isDeepinWM && isSupportedByWM(_net_wm_deepin_blur_region_rounded_atom))
                       || (m_isKwin && isContainsForRootWindow(_kde_net_wm_blur_rehind_region_atom)));
    // 当窗口visual不支持alpha通道时，也等价于不支持窗口背景模糊
    hasBlurWindow = hasBlurWindow && getHasWindowAlpha();

    if (m_hasBlurWindow == hasBlurWindow)
        return;

    m_hasBlurWindow = hasBlurWindow;

    emit hasBlurWindowChanged(hasBlurWindow);
}

void DXcbWMSupport::updateHasComposite()
{
    bool hasComposite;

    xcb_connection_t *xcb_connection = DPlatformIntegration::xcbConnection()->xcb_connection();

    auto atom = Utility::internAtom("_NET_KDE_COMPOSITE_TOGGLING");
    xcb_window_t root = DPlatformIntegration::xcbConnection()->primaryScreen()->root();

    //stage1: check if _NET_KDE_COMPOSITE_TOGGLING is supported
    xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection,
            xcb_get_property_unchecked(xcb_connection, false, root, atom, atom, 0, 1), NULL);
    if (reply && reply->type != XCB_NONE) {
        int value = 0;
        if (reply->type == atom && reply->format == 8) {
            value = *(int*)xcb_get_property_value(reply);
        }

        hasComposite = value == 1;
        free(reply);

        // 及时更新Qt中记录的值，KWin在关闭窗口合成的一段时间内（2S）并未释放相关的selection owner
        // 因此会导致Qt中的值在某个阶段与真实状态不匹配，用到QX11Info::isCompositingManagerRunning()
        // 的地方会出现问题，如drag窗口
        DPlatformIntegration::xcbConnection()->primaryVirtualDesktop()->m_compositingActive = hasComposite;
    } else {
        //stage2: fallback to check selection owner
        xcb_get_selection_owner_cookie_t cookit = xcb_get_selection_owner(xcb_connection, DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_CM_S0)));
        xcb_get_selection_owner_reply_t *reply = xcb_get_selection_owner_reply(xcb_connection, cookit, NULL);
        if (!reply)
            return;

        hasComposite = reply->owner != XCB_NONE;

        free(reply);
    }

    if (m_hasComposite == hasComposite)
        return;

    m_hasComposite = hasComposite;

    emit hasCompositeChanged(hasComposite);
}

void DXcbWMSupport::updateHasNoTitlebar()
{
    bool hasNoTitlebar(net_wm_atoms.contains(_deepin_no_titlebar));

    if (m_hasNoTitlebar == hasNoTitlebar)
        return;

    m_hasNoTitlebar = hasNoTitlebar;

    emit hasNoTitlebarChanged(m_hasNoTitlebar);
}

void DXcbWMSupport::updateHasScissorWindow()
{
    bool hasScissorWindow(net_wm_atoms.contains(_deepin_scissor_window) && hasComposite());

    if (m_hasScissorWindow == hasScissorWindow)
        return;

    m_hasScissorWindow = hasScissorWindow;

    emit hasScissorWindowChanged(m_hasScissorWindow);
}

void DXcbWMSupport::updateWallpaperEffect()
{
    bool hasWallpaperEffect(net_wm_atoms.contains(_deepin_wallpaper));

    if (m_hasWallpaperEffect == hasWallpaperEffect)
        return;

    m_hasWallpaperEffect = hasWallpaperEffect;

    emit hasWallpaperEffectChanged(hasWallpaperEffect);
}

qint8 DXcbWMSupport::getHasWindowAlpha() const
{
    if (m_windowHasAlpha < 0) {
        // 测试窗口visual是否支持alpha通道
        QWindow test_window;
        QSurfaceFormat sf = test_window.format();
        sf.setDepthBufferSize(32);
        sf.setAlphaBufferSize(8);
        test_window.setFormat(sf);
        test_window.create();
        // 当窗口位深不等于32时即认为它不支持alpha通道
        const_cast<DXcbWMSupport*>(this)->m_windowHasAlpha = static_cast<QXcbWindow*>(test_window.handle())->depth() == 32;
    }

    return m_windowHasAlpha;
}

DXcbWMSupport *DXcbWMSupport::instance()
{
    return globalXWMS;
}

bool DXcbWMSupport::connectWindowManagerChangedSignal(QObject *object, std::function<void ()> slot)
{
    if (!object)
        return QObject::connect(globalXWMS, &DXcbWMSupport::windowManagerChanged, slot);

    return QObject::connect(globalXWMS, &DXcbWMSupport::windowManagerChanged, object, slot);
}

bool DXcbWMSupport::connectHasBlurWindowChanged(QObject *object, std::function<void ()> slot)
{
    if (!object)
        return QObject::connect(globalXWMS, &DXcbWMSupport::hasBlurWindowChanged, slot);

    return QObject::connect(globalXWMS, &DXcbWMSupport::hasBlurWindowChanged, object, slot);
}

bool DXcbWMSupport::connectHasCompositeChanged(QObject *object, std::function<void ()> slot)
{
    if (!object)
        return QObject::connect(globalXWMS, &DXcbWMSupport::hasCompositeChanged, slot);

    return QObject::connect(globalXWMS, &DXcbWMSupport::hasCompositeChanged, object, slot);
}

bool DXcbWMSupport::connectHasNoTitlebarChanged(QObject *object, std::function<void ()> slot)
{
    if (!object)
        return QObject::connect(globalXWMS, &DXcbWMSupport::hasNoTitlebarChanged, slot);

    return QObject::connect(globalXWMS, &DXcbWMSupport::hasNoTitlebarChanged, object, slot);
}

bool DXcbWMSupport::connectHasWallpaperEffectChanged(QObject *object, std::function<void ()> slot)
{
    if (!object)
        return QObject::connect(globalXWMS, &DXcbWMSupport::hasWallpaperEffectChanged, slot);

    return QObject::connect(globalXWMS, &DXcbWMSupport::hasWallpaperEffectChanged, object, slot);
}

bool DXcbWMSupport::connectWindowListChanged(QObject *object, std::function<void ()> slot)
{
    if (!object)
        return QObject::connect(globalXWMS, &DXcbWMSupport::windowListChanged, slot);

    return QObject::connect(globalXWMS, &DXcbWMSupport::windowListChanged, object, slot);
}

bool DXcbWMSupport::connectWindowMotifWMHintsChanged(QObject *object, std::function<void (quint32)> slot)
{
    if (!object)
        return QObject::connect(globalXWMS, &DXcbWMSupport::windowMotifWMHintsChanged, slot);

    return QObject::connect(globalXWMS, &DXcbWMSupport::windowMotifWMHintsChanged, object, slot);
}

void DXcbWMSupport::setMWMFunctions(quint32 winId, quint32 func)
{
    // FIXME(zccrs): The Openbox window manager does not support the Motif Hints
    if (instance()->windowManagerName() == "Openbox")
        return;

    Utility::QtMotifWmHints hints = Utility::getMotifWmHints(winId);

    hints.flags |= MWM_HINTS_FUNCTIONS;
    hints.functions = func;

    Utility::setMotifWmHints(winId, hints);
}

quint32 DXcbWMSupport::getMWMFunctions(quint32 winId)
{
    Utility::QtMotifWmHints hints = Utility::getMotifWmHints(winId);

    if (hints.flags & MWM_HINTS_FUNCTIONS)
        return hints.functions;

    return MWM_FUNC_ALL;
}

quint32 DXcbWMSupport::getRealWinId(quint32 winId)
{
    for (const DFrameWindow *frame : DFrameWindow::frameWindowList) {
        if (frame->handle() && frame->handle()->winId() == winId
                && frame->m_contentWindow && frame->m_contentWindow->handle()) {
            return static_cast<QXcbWindow*>(frame->m_contentWindow->handle())->QXcbWindow::winId();
        }
    }

    return winId;
}

void DXcbWMSupport::setMWMDecorations(quint32 winId, quint32 decor)
{
    winId = getRealWinId(winId);

    Utility::QtMotifWmHints hints = Utility::getMotifWmHints(winId);

    hints.flags |= MWM_HINTS_DECORATIONS;
    hints.decorations = decor;

    Utility::setMotifWmHints(winId, hints);
}

quint32 DXcbWMSupport::getMWMDecorations(quint32 winId)
{
    winId = getRealWinId(winId);

    Utility::QtMotifWmHints hints = Utility::getMotifWmHints(winId);

    if (hints.flags & MWM_HINTS_DECORATIONS)
        return hints.decorations;

    return MWM_DECOR_ALL;
}

void DXcbWMSupport::popupSystemWindowMenu(quint32 winId)
{
    Utility::showWindowSystemMenu(winId);
}

QString DXcbWMSupport::windowManagerName() const
{
    return m_wmName;
}

QVector<xcb_window_t> DXcbWMSupport::allWindow() const
{
    QVector<xcb_window_t> window_list_stacking;

    xcb_window_t root = DPlatformIntegration::xcbConnection()->primaryScreen()->root();
    int offset = 0;
    int remaining = 0;
    xcb_connection_t *xcb_connection = DPlatformIntegration::xcbConnection()->xcb_connection();

    do {
        xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection, false, root,
                                                            Utility::internAtom("_NET_CLIENT_LIST_STACKING"),
                                                            XCB_ATOM_WINDOW, offset, 1024);
        xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection, cookie, NULL);
        if (!reply)
            break;

        remaining = 0;

        if (reply->type == XCB_ATOM_WINDOW && reply->format == 32) {
            int len = xcb_get_property_value_length(reply)/sizeof(xcb_window_t);
            xcb_window_t *windows = (xcb_window_t *)xcb_get_property_value(reply);
            int s = window_list_stacking.size();
            window_list_stacking.resize(s + len);
            memcpy(window_list_stacking.data() + s, windows, len*sizeof(xcb_window_t));

            remaining = reply->bytes_after;
            offset += len;
        }

        free(reply);
    } while (remaining > 0);

    return window_list_stacking;
}

static QXcbScreen *screenFromPoint(const QPoint &p)
{
    for (QXcbScreen *screen : DPlatformIntegration::xcbConnection()->screens()) {
        if (screen->geometry().contains(p)) {
            return screen;
        }
    }

    return DPlatformIntegration::xcbConnection()->primaryScreen();
}

xcb_window_t DXcbWMSupport::windowFromPoint(const QPoint &p) const
{
    xcb_window_t wid = XCB_NONE;
    xcb_connection_t *xcb_connection = DPlatformIntegration::xcbConnection()->xcb_connection();
    xcb_window_t root = screenFromPoint(p)->root();

    xcb_window_t parent = root;
    xcb_window_t child = root;
    int16_t x = static_cast<int16_t>(p.x());
    int16_t y = static_cast<int16_t>(p.y());

    auto translate_reply = Q_XCB_REPLY_UNCHECKED(xcb_translate_coordinates, xcb_connection, parent, child, x, y);
    if (!translate_reply) {
        return wid;
    }

    child = translate_reply->child;
    if (!child || child == root)
        return wid;

    child = Find_Client(xcb_connection, root, child);
    wid = child;
    return wid;
}

bool DXcbWMSupport::isDeepinWM() const
{
    return m_isDeepinWM;
}

bool DXcbWMSupport::isKwin() const
{
    return m_isKwin;
}

bool DXcbWMSupport::isSupportedByWM(xcb_atom_t atom) const
{
    return net_wm_atoms.contains(atom);
}

bool DXcbWMSupport::isContainsForRootWindow(xcb_atom_t atom) const
{
    return root_window_properties.contains(atom);
}

bool DXcbWMSupport::hasBlurWindow() const
{
    return m_hasBlurWindow && getHasWindowAlpha();
}

bool DXcbWMSupport::hasComposite() const
{
    return m_hasComposite;
}

bool DXcbWMSupport::hasNoTitlebar() const
{
    /*
    *  NOTE(lxz): Some dtk windows may be started before the window manager,
    *  and the rounded corners of the window cannot be set correctly,
    *  and need to use environment variables to force the setting.
    */
    if (qEnvironmentVariableIsSet("D_DXCB_FORCE_NO_TITLEBAR")) {
        return qEnvironmentVariableIntValue("D_DXCB_FORCE_NO_TITLEBAR") != 0;
    }

    static bool disable = qEnvironmentVariableIsSet("D_DXCB_DISABLE_NO_TITLEBAR");

    return !disable && m_hasNoTitlebar;
}

bool DXcbWMSupport::hasScissorWindow() const
{
    static bool disable = qEnvironmentVariableIsSet("D_DXCB_DISABLE_SCISSOR_WINDOW");

    return !disable && m_hasScissorWindow;
}

bool DXcbWMSupport::hasWindowAlpha() const
{
    // 窗管不支持混成时也等价于窗口visual不支持alpha通道
    return m_hasComposite && getHasWindowAlpha();
}

bool DXcbWMSupport::hasWallpaperEffect() const
{
    return m_hasWallpaperEffect;
}

bool DXcbWMSupport::Global::hasBlurWindow()
{
    return  DXcbWMSupport::instance()->hasBlurWindow();
}

bool DXcbWMSupport::Global::hasComposite()
{
    // 为了兼容现有的dtk应用中的逻辑，此处默认认为窗管是否支持混成等价于窗口是否支持alpha通道
    static bool composite_with_alpha = qgetenv("D_DXCB_COMPOSITE_WITH_WINDOW_ALPHA") != "0";

    return composite_with_alpha ? hasWindowAlpha() : DXcbWMSupport::instance()->hasComposite();
}

bool DXcbWMSupport::Global::hasNoTitlebar()
{
    return DXcbWMSupport::instance()->hasNoTitlebar();
}

bool DXcbWMSupport::Global::hasWindowAlpha()
{
    return DXcbWMSupport::instance()->hasWindowAlpha();
}

bool DXcbWMSupport::Global::hasWallpaperEffect()
{
    return  DXcbWMSupport::instance()->hasWallpaperEffect();
}

QString DXcbWMSupport::Global::windowManagerName()
{
    return DXcbWMSupport::instance()->windowManagerName();
}

DPP_END_NAMESPACE

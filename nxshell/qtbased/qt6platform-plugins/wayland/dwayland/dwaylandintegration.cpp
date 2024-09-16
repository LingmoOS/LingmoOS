// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwaylandintegration.h"
#include "dwaylandinterfacehook.h"
#include "vtablehook.h"
#include "dxsettings.h"

#include <wayland-cursor.h>

#define private public
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandcursor_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#undef private
#include <QtWaylandClientVersion>

#include <QDebug>
#include <QTimer>

#include <qpa/qplatformnativeinterface.h>
#include <private/qguiapplication_p.h>

DPP_BEGIN_NAMESPACE

#define XSETTINGS_DOUBLE_CLICK_TIME QByteArrayLiteral("Net/DoubleClickTime")
#define XSETTINGS_CURSOR_THEME_NAME QByteArrayLiteral("Gtk/CursorThemeName")
#define XSETTINGS_PRIMARY_MONITOR_RECT QByteArrayLiteral("DDE/PrimaryMonitorRect")

enum XSettingType:quint64 {
    Gtk_CursorThemeName,
    Dde_PrimaryMonitorRect
};

static void overrideChangeCursor(QPlatformCursor *cursorHandle, QCursor * cursor, QWindow * widget)
{
    if (!(widget && widget->handle()))
        return;

    if (widget->property(disableOverrideCursor).toBool())
        return;

    // qtwayland里面判断了，如果没有设置环境变量，就用默认大小32, 这里通过设在环境变量将大小设置我们需要的24
    static bool xcursorSizeIsSet = qEnvironmentVariableIsSet("XCURSOR_SIZE");
    if (!xcursorSizeIsSet) {
        qputenv("XCURSOR_SIZE", QByteArray::number(24 * qApp->devicePixelRatio()));
    }

    HookCall(cursorHandle, &QPlatformCursor::changeCursor, cursor, widget);

    // 调用changeCursor后，控制中心窗口的光标没有及时更新，这里强制刷新一下
    for (auto *inputDevice : DWaylandIntegration::instance()->display()->inputDevices()) {
        if (inputDevice->pointer()) {
            inputDevice->pointer()->updateCursor();
        }
    }
}

static void onXSettingsChanged(xcb_connection_t *connection, const QByteArray &name, const QVariant &property, void *handle)
{
    Q_UNUSED(connection)
    Q_UNUSED(property)
    Q_UNUSED(name)

    quint64 type = reinterpret_cast<quint64>(handle);

    switch (type) {
    case Gtk_CursorThemeName:
        const QByteArray &cursor_name = dXSettings->globalSettings()->setting(name).toByteArray();

#if QTWAYLANDCLIENT_VERSION < QT_VERSION_CHECK(5, 13, 0)
        const auto &cursor_map = DWaylandIntegration::instance()->display()->mCursorThemesBySize;
#elif QTWAYLANDCLIENT_VERSION < QT_VERSION_CHECK(5, 16, 0)
        const auto &cursor_map = DWaylandIntegration::instance()->display()->mCursorThemes;
#endif
        // 处理光标主题变化
        for (auto cursor = cursor_map.constBegin(); cursor != cursor_map.constEnd(); ++cursor) {
            QtWaylandClient::QWaylandCursorTheme *ct = cursor.value();
            // 根据大小记载新的主题

#if QTWAYLANDCLIENT_VERSION < QT_VERSION_CHECK(5, 13, 0)
            auto theme = wl_cursor_theme_load(cursor_name.constData(), cursor.key(), DWaylandIntegration::instance()->display()->shm()->object());
#elif QTWAYLANDCLIENT_VERSION < QT_VERSION_CHECK(5, 16, 0)
            auto theme = wl_cursor_theme_load(cursor_name.constData(), cursor.key().second, DWaylandIntegration::instance()->display()->shm()->object());
#endif
            // 如果新主题创建失败则不更新数据
            if (!theme)
                continue;

            // 先尝试销毁旧主题
            if (ct->m_theme) {
                wl_cursor_theme_destroy(ct->m_theme);
            }

            // 清理缓存数据
            ct->m_cursors.clear();
            ct->m_theme = theme;
        }

        // 更新窗口光标
        for (auto s : DWaylandIntegration::instance()->display()->screens()) {
            for (QWindow *w : s->windows()) {
                QCursor cursor = w->cursor();
                // 为窗口重新设置光标
                s->cursor()->changeCursor(&cursor, w);
            }
        }

        break;

    }
}

static void onPrimaryRectChanged(xcb_connection_t *connection, const QByteArray &name, const QVariant &property, void *handle)
{
    Q_UNUSED(connection)
    Q_UNUSED(property)

    quint64 type = reinterpret_cast<quint64>(handle);
    switch (type) {
    case Dde_PrimaryMonitorRect:
    {
        auto screens = DWaylandIntegration::instance()->display()->screens();
        const QString &primaryScreenRect = dXSettings->globalSettings()->setting(name).toString();
        auto list = primaryScreenRect.split('-');
        if (list.size() != 4)
            return;

        QRect xsettingsRect(list.at(0).toInt(), list.at(1).toInt(), list.at(2).toInt(), list.at(3).toInt());

        qDebug() << "primary rect from xsettings:" << primaryScreenRect << ", key:" << name;
        for (auto s : screens) {
            qDebug() << "screen info:"
                     << s->screen()->name() << s->screen()->model() << s->screen()->geometry() << s->geometry()
                     << s->name() << s->model() << s->screen()->geometry() << s->geometry();
        }

        // 设置新的主屏
        QtWaylandClient::QWaylandScreen *primaryScreen = nullptr;

        // 坐标一致，认定为主屏
        for (auto screen : screens) {
            if (screen->geometry().topLeft() == xsettingsRect.topLeft() && screen->screen() != qApp->primaryScreen()) {
                primaryScreen = screen;
                qDebug() << "got primary:" << primaryScreen->name() << primaryScreen->geometry();
                break;
            }
        }

        if (primaryScreen) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
            QWindowSystemInterface::handlePrimaryScreenChanged(primaryScreen);
#else
            DWaylandIntegration::instance()->setPrimaryScreen(primaryScreen);
#endif
        }

        break;
    }
    default:
        break;
    }
    qDebug() << "primary screen info:" << QGuiApplication::primaryScreen()->name() << QGuiApplication::primaryScreen()->model() << QGuiApplication::primaryScreen()->geometry();
}

DWaylandIntegration::DWaylandIntegration()
{
    dXSettings->initXcbConnection();
}

void DWaylandIntegration::initialize()
{
    // 由于Qt代码中可能写死了判断是不是wayland平台，所以只能伪装成是wayland
    if (qgetenv("DXCB_FAKE_PLATFORM_NAME_XCB") != "0") {
        *QGuiApplicationPrivate::platform_name = "wayland";
    }
    qApp->setProperty("_d_isDwayland", true);

    QWaylandIntegration::initialize();
    // 覆盖wayland的平台函数接口，用于向上层返回一些必要的与平台相关的函数供其使用
    HookOverride(nativeInterface(), &QPlatformNativeInterface::platformFunction, &DWaylandInterfaceHook::platformFunction);
    // hook qtwayland的改变光标的函数，用于设置环境变量并及时更新
    for (QScreen *screen : qApp->screens()) {
        //在没有屏幕的时候，qtwayland会创建一个虚拟屏幕，此时不能调用screen->handle()->cursor()
        if (screen && screen->handle() && screen->handle()->cursor()) {
             HookOverride(screen->handle()->cursor(), &QPlatformCursor::changeCursor, &overrideChangeCursor);
        }
    }
    // 监听xsettings的信号，用于更新程序状态（如更新光标主题）
    dXSettings->globalSettings()->registerCallbackForProperty(XSETTINGS_CURSOR_THEME_NAME, onXSettingsChanged, reinterpret_cast<void*>(XSettingType::Gtk_CursorThemeName));

    // 增加rect的属性，保存主屏的具体坐标，不依靠其name判断(根据name查找对应的屏幕时概率性出错，根据主屏的rect确定哪一个QScreen才是主屏)
    dXSettings->globalSettings()->registerCallbackForProperty(XSETTINGS_PRIMARY_MONITOR_RECT, onPrimaryRectChanged, reinterpret_cast<void*>(XSettingType::Dde_PrimaryMonitorRect));

    //初始化时应该设一次主屏，防止应用启动时主屏闪变
    onPrimaryRectChanged(nullptr, XSETTINGS_PRIMARY_MONITOR_RECT, QVariant(), reinterpret_cast<void*>(XSettingType::Dde_PrimaryMonitorRect));

    QTimer *m_delayTimer = new QTimer;
    m_delayTimer->setInterval(10);
    m_delayTimer->setSingleShot(true);
    QObject::connect(qApp, &QGuiApplication::aboutToQuit, m_delayTimer, &QObject::deleteLater);

    QObject::connect(m_delayTimer, &QTimer::timeout, []{
        onPrimaryRectChanged(nullptr, XSETTINGS_PRIMARY_MONITOR_RECT, QVariant(), reinterpret_cast<void*>(XSettingType::Dde_PrimaryMonitorRect));
    });

    // 显示器信息发生变化时，刷新主屏信息
    auto handleScreenInfoChanged = [ = ](QScreen *s) {
#define HANDLE_SCREEN_SIGNAL(signal) \
    QObject::connect(s, signal, m_delayTimer, static_cast<void (QTimer::*)()>(&QTimer::start));

        HANDLE_SCREEN_SIGNAL(&QScreen::geometryChanged);
        HANDLE_SCREEN_SIGNAL(&QScreen::availableGeometryChanged);
        HANDLE_SCREEN_SIGNAL(&QScreen::physicalSizeChanged);
        HANDLE_SCREEN_SIGNAL(&QScreen::physicalDotsPerInchChanged);
        HANDLE_SCREEN_SIGNAL(&QScreen::logicalDotsPerInchChanged);
        HANDLE_SCREEN_SIGNAL(&QScreen::virtualGeometryChanged);
        HANDLE_SCREEN_SIGNAL(&QScreen::primaryOrientationChanged);
        HANDLE_SCREEN_SIGNAL(&QScreen::orientationChanged);
        HANDLE_SCREEN_SIGNAL(&QScreen::refreshRateChanged);

        m_delayTimer->start();
    };

    for (auto s : qApp->screens()) {
        handleScreenInfoChanged(s);
    }

    QObject::connect(qApp, &QGuiApplication::screenAdded, handleScreenInfoChanged);
    QObject::connect(qApp, &QGuiApplication::screenAdded, m_delayTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
}

QStringList DWaylandIntegration::themeNames() const
{
    auto list = QWaylandIntegration::themeNames();
    const QByteArray desktop_session = qgetenv("DESKTOP_SESSION");

    // 在lightdm环境中，无此环境变量。默认使用deepin平台主题
    if (desktop_session.isEmpty() || desktop_session == "deepin")
        list.prepend("deepin");

    return list;
}

#define GET_VALID_XSETTINGS(key) { \
    auto value = dXSettings->globalSettings()->setting(key); \
    if (value.isValid()) return value; \
}

QVariant DWaylandIntegration::styleHint(QPlatformIntegration::StyleHint hint) const
{
#ifdef Q_OS_LINUX
    switch ((int)hint) {
    case MouseDoubleClickInterval:
        GET_VALID_XSETTINGS(XSETTINGS_DOUBLE_CLICK_TIME);
        break;
    case ShowShortcutsInContextMenus:
        return false;
    default:
        break;
    }
#endif

    return QtWaylandClient::QWaylandIntegration::styleHint(hint);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
void DWaylandIntegration::setPrimaryScreen(QPlatformScreen *newPrimary)
{
    return QPlatformIntegration::setPrimaryScreen(newPrimary);
}
#endif

DPP_END_NAMESPACE

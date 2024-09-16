/*
 * SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "qdeepintheme.h"
#include "qdeepinfiledialoghelper.h"
#include "filedialogmanager_interface.h"
#include "dthemesettings.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "../3rdparty/qdbustrayicon_p.h"
#endif

#include <DGuiApplicationHelper>
#include <DPlatformTheme>

#include <QGuiApplication>
#include <QIconEnginePlugin>

#include <private/qicon_p.h>
#include <private/qiconloader_p.h>
#define private public
#include <private/qhighdpiscaling_p.h>
#undef private
#include <private/qwindow_p.h>
#include <private/qguiapplication_p.h>
#include <qpa/qwindowsysteminterface_p.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformcursor.h>

#undef signals
#include <X11/Xlib.h>

DGUI_USE_NAMESPACE

#ifdef XDG_ICON_VERSION_MAR
#include <XdgIcon>
extern void updateXdgIconSystemTheme();
#if XDG_ICON_VERSION_MAR >= 3
namespace DEEPIN_QT_THEME {
QThreadStorage<QString> colorScheme;
void(*setFollowColorScheme)(bool);
bool(*followColorScheme)();
}
#endif
#endif

#define DISABLE_UPDATE_WINDOW_GEOMETRY "D_DISABLE_UPDATE_WINDOW_GEOMETRY_FOR_SCALE"
#define DNOT_UPDATE_WINDOW_GEOMETRY "_d_disable_update_geometry_for_scale"
#define HOOK_UPDATE_WINDOW_GEOMETRY_OBJECT "_d_hookUpdateGeometryForScaleObject"
#define UPDATE_WINDOW_GEOMETRY_ENTRY "_d_updateGeometryForScaleEntry"
#define UPDATE_WINDOW_GEOMETRY_GEOMETRY "_d_updateGeometryForScaleGeometry"
#define UPDATE_WINDOW_GEOMETRY_EXIT "_d_updateGeometryForScaleExit"

QT_BEGIN_NAMESPACE

const char *QDeepinTheme::name = "deepin";
bool QDeepinTheme::m_usePlatformNativeDialog = true;
QMimeDatabase QDeepinTheme::m_mimeDatabase;
DThemeSettings *QDeepinTheme::m_settings = 0;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
static bool isDBusTrayAvailable() {
    static bool dbusTrayAvailable = false;
    static bool dbusTrayAvailableKnown = false;
    if (!dbusTrayAvailableKnown) {
        thirdparty::QDBusMenuConnection conn;
        if (conn.isStatusNotifierHostRegistered())
            dbusTrayAvailable = true;
        dbusTrayAvailableKnown = true;
        qCDebug(thirdparty::dLcTray) << "D-Bus tray available:" << dbusTrayAvailable;
    }
    return dbusTrayAvailable;
}
#endif

static void onIconThemeSetCallback()
{
    QIconLoader::instance()->updateSystemTheme();
#ifdef XDG_ICON_VERSION_MAR
    updateXdgIconSystemTheme();
#endif
    if (qApp->inherits("Dtk::Widget::DApplication")) {
        // emit the signal: DApplication::iconThemeChanged
        qApp->metaObject()->invokeMethod(qApp, QT_STRINGIFY(iconThemeChanged));
    }

    // 通知所有窗口重绘制
    QEvent update(QEvent::UpdateRequest);
    for (QWindow *window : qGuiApp->allWindows()) {
        if (window->type() == Qt::Desktop)
            continue;

        qApp->sendEvent(window, &update);
    }
}

static inline uint resolveMask(const QFont &f)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return f.resolve();
#else
    return f.resolveMask();
#endif
}

static inline void setResolveMask(QFont &f, uint mask)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return f.resolve(mask);
#else
    return f.setResolveMask(mask);
#endif
}

static void onFontChanged()
{
    // 插件中初始化 appFont 时会 resolve(0) 即清空 resolve_mask
    // 如果用户再主动设置字体会改变 resolve_mask 即 app_font->resolve() > 0
    // qApp->setFont(resolvedFont) 将不再响应 xsettings 字体变化
    if (QGuiApplicationPrivate::app_font &&
        resolveMask(*QGuiApplicationPrivate::app_font))
        return;

    // 先清理旧的font对象
    if (QGuiApplicationPrivate::app_font)
        delete QGuiApplicationPrivate::app_font;
    QGuiApplicationPrivate::app_font = nullptr;

    QFont appFont = qGuiApp->font(); // Refresh the font

    QEvent event(QEvent::ApplicationFontChange);
    qApp->sendEvent(qApp, &event);

    // 通知所有窗口字体改变了
    for (QWindow *window : qGuiApp->allWindows()) {
        if (window->type() == Qt::Desktop)
            continue;

        qApp->sendEvent(window, &event);
    }

    qApp->sendEvent(DGuiApplicationHelper::instance(), &event);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Q_EMIT qGuiApp->fontChanged(appFont);
#else
    Q_UNUSED(appFont)
#endif
}

static void updateWindowGeometry(QWindow *w)
{
    if (w->type() == Qt::ForeignWindow || w->type() == Qt::Desktop) {
        return;
    }

    if (!w->handle() || !w->isTopLevel())
        return;

    if (w->property(DNOT_UPDATE_WINDOW_GEOMETRY).toBool()) {
        QWindowSystemInterfacePrivate::GeometryChangeEvent gce(w, QHighDpi::fromNativePixels(w->handle()->geometry(), w)
                                                   #if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
                                                               , QRect()
                                                   #endif
                                                               );
        QGuiApplicationPrivate::processGeometryChangeEvent(&gce);
    } else {// 通知窗口大小发送改变
        const QRect currentGeo = QWindowPrivate::get(w)->geometry;

        if (!currentGeo.isValid())
            return;

        // 对外提供能hook自动更新窗口大小的方法
        QObject *hook_obj = qvariant_cast<QObject*>(w->property(HOOK_UPDATE_WINDOW_GEOMETRY_OBJECT));

        if (!hook_obj) {
            hook_obj = w;
        }

        bool accept = true;
        // 通知窗口即将开始更新其geometry
        if (hook_obj->metaObject()->indexOfMethod(UPDATE_WINDOW_GEOMETRY_ENTRY "()") >= 0) {
            QMetaObject::invokeMethod(hook_obj, UPDATE_WINDOW_GEOMETRY_ENTRY,
                                      Qt::DirectConnection, Q_RETURN_ARG(bool, accept));
        }

        if (!accept) {
            // 中断操作
            return;
        }

        QRect nativeGeo;
        // 尝试从窗口对象获取新的geometry
        if (hook_obj->metaObject()->indexOfMethod(UPDATE_WINDOW_GEOMETRY_GEOMETRY "()") >= 0) {
            QMetaObject::invokeMethod(hook_obj, UPDATE_WINDOW_GEOMETRY_GEOMETRY,
                                      Qt::DirectConnection, Q_RETURN_ARG(QRect, nativeGeo));
        }

        if (!nativeGeo.isValid()) {
            nativeGeo = w->handle()->geometry();

            qreal scale = QHighDpiScaling::factor(w);
            const QPoint &cursor_pos = w->screen()->handle()->cursor()->pos();
            // 如果窗口是active的，且鼠标处于窗口内，应当移动窗口位置以保持鼠标相对窗口的位置
            if (w->isActive()) {
                const QMargins &frame_margins = w->handle()->frameMargins();
                const QRect &frame_rect = nativeGeo.marginsAdded(frame_margins);

                if (frame_rect.contains(cursor_pos)) {
                    nativeGeo.moveTopLeft(cursor_pos + (nativeGeo.topLeft() - cursor_pos) * currentGeo.width() * scale / nativeGeo.width());
                }
            }

            nativeGeo.setSize(currentGeo.size() * scale);
        }

        bool positionAutomaticSave = QWindowPrivate::get(w)->positionAutomatic;
        QWindowPrivate::get(w)->positionAutomatic = false;
        w->handle()->setGeometry(nativeGeo);
        QWindowPrivate::get(w)->positionAutomatic = positionAutomaticSave;
        // 请求重绘
        QGuiApplication::sendEvent(w, new QEvent(QEvent::UpdateRequest));
        // 通知窗口geometry更新结束
        if (hook_obj->metaObject()->indexOfMethod(UPDATE_WINDOW_GEOMETRY_EXIT "()") >= 0) {
            QMetaObject::invokeMethod(hook_obj, UPDATE_WINDOW_GEOMETRY_EXIT);
        }
    }
}

static void updateAllWindowGeometry()
{
    for (QWindow *w : qGuiApp->allWindows()) {
        updateWindowGeometry(w);
    }
}

// 延迟一段时间更新窗口geometry，防止屏幕缩放比在短时间内连续变化时导致窗口闪动
static void updateAllWindowGeometryDelay(int interval = 500)
{
    static QTimer *t = new QTimer();

    t->setSingleShot(true);
    t->setInterval(interval);
    t->connect(t, &QTimer::timeout, t, updateAllWindowGeometry, Qt::UniqueConnection);
    t->start();
}

static void notifyScreenScaleUpdated()
{
    for (QScreen *s : qGuiApp->screens()) {
        Q_EMIT s->geometryChanged(s->geometry());

        // 发射信号通知屏幕缩放比发生变化，DApplication中会定义此信号
        if (qGuiApp->metaObject()->indexOfSignal("screenDevicePixelRatioChanged(QScreen*)")) {
            qGuiApp->metaObject()->invokeMethod(qGuiApp, "screenDevicePixelRatioChanged", Q_ARG(QScreen*, s));
        }
    }
}

static bool updateScaleFactor(qreal value)
{
    if (qIsNull(value)) {
        value = 1.0;
    }

    if (qFuzzyCompare(QHighDpiScaling::m_factor, value)) {
        return false;
    }

    QHighDpiScaling::setGlobalFactor(value);

    return true;
}

static void onScaleFactorChanged(qreal value)
{
    if (updateScaleFactor(value)) {
        notifyScreenScaleUpdated();
        updateAllWindowGeometryDelay();
    }
}

// 用于窗口屏幕改变时更新窗口大小
class AutoScaleWindowObject : public QObject
{
    Q_OBJECT
public:
    explicit AutoScaleWindowObject(QObject *parent = nullptr)
        : QObject(parent) {
        qGuiApp->installEventFilter(this);
    }

    void onScreenChanged(QScreen *s)
    {
        Q_UNUSED(s);

        if (QWindow *w = qobject_cast<QWindow*>(sender())) {
            updateWindowGeometry(w);
        }
    }

private:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() != QEvent::PlatformSurface)
            return false;

        if (QWindow *w = qobject_cast<QWindow*>(watched)) {
            QPlatformSurfaceEvent *se = static_cast<QPlatformSurfaceEvent*>(event);

            if (se->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                // 监听窗口屏幕变化信号，自动根据新的屏幕缩放比更新窗口大小
                connect(w, &QWindow::screenChanged,
                        this, &AutoScaleWindowObject::onScreenChanged,
                        Qt::DirectConnection);
            } else {
                disconnect(w, &QWindow::screenChanged,
                           this, &AutoScaleWindowObject::onScreenChanged);
            }
        }

        return false;
    }
};

// 判断ScreenScaleFactor的值是否应该应用于所有屏幕
static bool isGenericScreenScaleFactors(const QByteArray &value)
{
    for (char ch : value) {
        if (ch == '=' || ch == ';')
            return false;
    }

    return true;
}

static void onAutoScaleWindowChanged()
{
    bool on = QDeepinTheme::getSettings()->autoScaleWindow();

    if (on) {
        const QByteArray &multi_scale = QDeepinTheme::getSettings()->screenScaleFactors();

        // 只在针对多个屏幕分别设置了缩放比时开启此功能
        if (isGenericScreenScaleFactors(multi_scale)) {
            on = false;
        }
    }

    static AutoScaleWindowObject *event_fileter = nullptr;

    if (on) {
        if (event_fileter)
            return;

        event_fileter = new AutoScaleWindowObject(qGuiApp);
    } else {
        if (!event_fileter)
            return;

        event_fileter->deleteLater();
        event_fileter = nullptr;
    }
}

// 使用 QT_SCREEN_SCALE_FACTORS 为每个屏幕设置不同的缩放比之后，Qt会自动将 dpi 除以主屏的
// 缩放倍数，以此来避免字体被放大。font dpi会影响未设置pixel size的QFont，默认情况下，
// QGuiApplication::font() 不会设置pixel size，因此，使用分屏幕设置不同缩放比后，字体却还
// 是缩放前的大小。
// 此处，如果设置了 ScreenScaleFactors，但未指定 ScaleLogcailDpi 时，默认将其重设回主屏
// 的 logicalDpi。
static bool updateScaleLogcailDpi(const QPair<qreal, qreal> &dpi)
{
    bool ok = dpi.first >= 0 && dpi.second >= 0;
#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
    if (dpi.first > 0) {
        QHighDpiScaling::m_logicalDpi.first = dpi.first;
    } else if (qIsNull(dpi.first)) {
        QHighDpiScaling::m_logicalDpi.first = qGuiApp->primaryScreen()->handle()->logicalDpi().first;
    }

    if (dpi.second > 0) {
        QHighDpiScaling::m_logicalDpi.second = dpi.second;
    } else if (qIsNull(dpi.second)) {
        QHighDpiScaling::m_logicalDpi.second = qGuiApp->primaryScreen()->handle()->logicalDpi().second;
    }
#elif QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QHighDpiScaling::m_usePixelDensity = false; // Do not use dpi from platform plugin
#else
    QHighDpiScaling::m_usePlatformPluginDpi = false; // Do not use dpi from platform plugin
#endif
    return ok;
}

static QByteArray getEnvValueByScreenScaleFactors(const QByteArray &data)
{
    QByteArray envValue;

    if (!isGenericScreenScaleFactors(data)) {
        envValue = data;
    } else if (!data.isEmpty()) {
        // 这种情况下，在Qt的实现中，只会将值应用给第一个屏幕，因此此处需要给所有屏幕都设置通用的值
        int screen_count = qApp->screens().count();

        // 为除了最后一个屏幕的其它屏幕添加缩放比
        while (--screen_count) {
            envValue.append(data).append(';');
        }

        // 为最后一个屏幕添加缩放比
        envValue.append(data);
    }

    return envValue;
}

static bool updateScreenScaleFactors(DThemeSettings *s, const QByteArray &value, bool unsetenv = false)
{
    const QByteArray &envValue = getEnvValueByScreenScaleFactors(value);

    if (qgetenv("QT_SCREEN_SCALE_FACTORS") == envValue)
        return false;

    if (envValue.isEmpty()) {
        if (!unsetenv)
            return false;

        qunsetenv("QT_SCREEN_SCALE_FACTORS");
    } else {
        qputenv("QT_SCREEN_SCALE_FACTORS", envValue);
    }

    QHighDpiScaling::updateHighDpiScaling();
#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
    qDebug() << QHighDpiScaling::logicalDpi();
#else
    qDebug() << QHighDpiScaling::logicalDpi(QGuiApplication::primaryScreen());
#endif
    updateScaleLogcailDpi(s->scaleLogicalDpi());

#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
    qDebug() << QHighDpiScaling::logicalDpi();
#else
    qDebug() << QHighDpiScaling::logicalDpi(QGuiApplication::primaryScreen());
#endif

    return true;
}
static void onScreenScaleFactorsChanged(const QByteArray &value)
{
    if (updateScreenScaleFactors(QDeepinTheme::getSettings(), value, true)) {
        notifyScreenScaleUpdated();
        updateAllWindowGeometryDelay();
    }
}

static bool enabledRTScreenScale()
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    // 应用中设置了和屏幕缩放相关的环境变量或启动相关属性后后不开启自动缩放功能
    static bool enable = !qEnvironmentVariableIsSet("D_DISABLE_RT_SCREEN_SCALE") &&
                            !qEnvironmentVariableIsSet("QT_DEVICE_PIXEL_RATIO") &&
                            !qEnvironmentVariableIsSet("QT_SCALE_FACTOR") &&
                            !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR") &&
                            !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS") &&
                            !QCoreApplication::testAttribute(Qt::AA_DisableHighDpiScaling) &&
                            !QCoreApplication::testAttribute(Qt::AA_EnableHighDpiScaling) &&
                            qGuiApp->platformName().endsWith("xcb");
#else
    static bool enable = false; // TODO What is the enable logic under Qt6 when Qt::AA_EnableHighDpiScaling is always true
#endif

    return enable;
}

QDeepinTheme::QDeepinTheme()
{
#if XDG_ICON_VERSION_MAR >= 3
    // 注意!!!, 此处还没有开始启动事件循环, 很多Qt类无法使用, 例如QTimer
    DEEPIN_QT_THEME::setFollowColorScheme = XdgIcon::setFollowColorScheme;
    DEEPIN_QT_THEME::followColorScheme = XdgIcon::followColorScheme;
#endif

    if (enabledRTScreenScale()) {
        QScopedPointer<DThemeSettings> setting(new DThemeSettings(false));
        // 程序启动时初始设置屏幕缩放比
        updateScaleFactor(setting->scaleFactor());

        if (!updateScreenScaleFactors(setting.data(), setting->screenScaleFactors())) {
            updateScaleLogcailDpi(setting->scaleLogicalDpi());
        }
    }
}

QDeepinTheme::~QDeepinTheme()
{
    if (QDeepinFileDialogHelper::manager) {
        QDeepinFileDialogHelper::manager->deleteLater();
        QDeepinFileDialogHelper::manager = Q_NULLPTR;
    }
}

bool QDeepinTheme::usePlatformNativeDialog(DialogType type) const
{
    if (type == FileDialog) {
        if (qgetenv("_d_disableDBusFileDialog") == "true")
            return false;

        static bool dbusDialogManagerInitialized = false;

        if (!dbusDialogManagerInitialized) {
            dbusDialogManagerInitialized = true;
            QDeepinFileDialogHelper::initDBusFileDialogManager();
        }

        return m_usePlatformNativeDialog
                && QDeepinFileDialogHelper::manager
                && QDeepinFileDialogHelper::manager->isUseFileChooserDialog();
    }

    return QGenericUnixTheme::usePlatformNativeDialog(type);
}

QPlatformDialogHelper *QDeepinTheme::createPlatformDialogHelper(DialogType type) const
{
    if (type == FileDialog && usePlatformNativeDialog(type))
        return new QDeepinFileDialogHelper();

    return QGenericUnixTheme::createPlatformDialogHelper(type);
}

static QIconEnginePlugin *getIconEngineFactory(const QString &key)
{
    static QFactoryLoader loader(QIconEngineFactoryInterface_iid, QLatin1String("/iconengines"), Qt::CaseSensitive);
    int index = loader.indexOf(key);

    if (index != -1)
        return qobject_cast<QIconEnginePlugin *>(loader.instance(index));

    return nullptr;
}

static QIconEngine *createIconEngineWithKey(const QString &iconName, const QString &key)
{
    QIconEnginePlugin *plugin = getIconEngineFactory(key);
    if (!plugin)
        return nullptr;

    QIconEngine *iconEngine = plugin->create(iconName);
    if (!iconEngine)
        return nullptr;

    if (iconEngine->isNull()) {
        delete iconEngine;
        return nullptr;
    }

    return iconEngine;
}

QIconEngine *QDeepinTheme::createIconEngine(const QString &iconName) const
{
    QIconEngine *engine = createIconEngineWithKey(iconName, "DIconProxyEngine");
    return engine ? engine : QGenericUnixTheme::createIconEngine(iconName);
}

QPixmap QDeepinTheme::standardPixmap(QPlatformTheme::StandardPixmap sp, const QSizeF &size) const
{
    return QGenericUnixTheme::standardPixmap(sp, size);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
QIcon QDeepinTheme::fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions) const
{
    Q_UNUSED(iconOptions);

    return QIcon::fromTheme(m_mimeDatabase.mimeTypeForFile(fileInfo).iconName());
}
#else
QPixmap QDeepinTheme::fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size, QPlatformTheme::IconOptions iconOptions) const
{
    Q_UNUSED(iconOptions);

    return QIcon::fromTheme(m_mimeDatabase.mimeTypeForFile(fileInfo).iconName()).pixmap(size.toSize());
}
#endif

#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QPlatformSystemTrayIcon *QDeepinTheme::createPlatformSystemTrayIcon() const
{
    if (isDBusTrayAvailable())
        return new thirdparty::QDBusTrayIcon();
    return nullptr;
}
#endif

static DPlatformTheme *appTheme()
{
    static QPointer<DPlatformTheme> theme;

    if (!theme) {
        theme = DGuiApplicationHelper::instance()->applicationTheme();
        QObject::connect(theme, &DPlatformTheme::iconThemeNameChanged, &onIconThemeSetCallback);
        QObject::connect(theme, &DPlatformTheme::fontNameChanged, &onFontChanged);
        QObject::connect(theme, &DPlatformTheme::fontPointSizeChanged, [] {
            if (theme->fontName().isEmpty())
                return;

            onFontChanged();
        });
        QObject::connect(theme, &DPlatformTheme::gtkFontNameChanged, [] {
            if (theme->fontName().isEmpty()) {
                onFontChanged();
            }
        });
    }

    return theme;
}

QVariant QDeepinTheme::themeHint(QPlatformTheme::ThemeHint hint) const
{
    switch (hint) {
    case QPlatformTheme::StyleNames: {
        return QStringList({"chameleon", "fusion"});
    }
    case QPlatformTheme::SystemIconThemeName:
        return appTheme()->iconThemeName();
    case QPlatformTheme::SystemIconFallbackThemeName:
        return "bloom";
    case QPlatformTheme::IconThemeSearchPaths:
        return QVariant(QGenericUnixTheme::xdgIconThemePaths() << QDir::homePath() + "/.local/share/icons");
    case UseFullScreenForPopupMenu:
        return true;
    case ShowShortcutsInContextMenus:
        return false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case ButtonPressKeys:
        return QVariant::fromValue(QList<Qt::Key>({ Qt::Key_Enter, Qt::Key_Return, Qt::Key_Space, Qt::Key_Select } ));
#endif
    default:
        break;
    }

    return QGenericUnixTheme::themeHint(hint);
}

const QPalette *QDeepinTheme::palette(QPlatformTheme::Palette type) const
{
    if (type != SystemPalette) {
        return QGenericUnixTheme::palette(type);
    }

    static QPalette palette;
    palette = DGuiApplicationHelper::instance()->applicationPalette();

    return &palette;
}

const QFont *QDeepinTheme::font(QPlatformTheme::Font type) const
{
    if (!qApp->desktopSettingsAware())
        return QGenericUnixTheme::font(type);

    switch (type) {
    case SystemFont:
        if (DPlatformTheme *theme = appTheme()) {
            QByteArray font_name = theme->fontName();
            qreal font_size = 0;

            if (font_name.isEmpty()) {
                font_name = theme->gtkFontName();
                int font_size_index = font_name.lastIndexOf(' ');

                if (font_size_index <= 0)
                    break;

                font_size = font_name.mid(font_size_index + 1).toDouble();
                font_name = font_name.left(font_size_index);
            } else {
                font_size = theme->fontPointSize();
            }

            if (font_size <= 0) {
                font_size = 10.5;
            }

            static QScopedPointer<QFont> sysFont;

            // We need to re-contruct a new QFont each time, inorder to refresh font dpi
            sysFont.reset(new QFont(QString()));
            sysFont->setFamily(font_name);
            sysFont->setPointSizeF(font_size);
            setResolveMask(*sysFont, QFont::NoPropertiesResolved);

            return sysFont.data();
        }

        break;
    case FixedFont: {
        if (DPlatformTheme *theme = appTheme()) {
            QByteArray font_name = theme->monoFontName();

            if (font_name.isEmpty()) {
                break;
            }

            qreal font_size = theme->fontPointSize();

            if (font_size <= 0) {
                font_size = 10.5;
            }

            static QScopedPointer<QFont> fixedFont;

            // We need to re-contruct a new QFont each time, inorder to refresh font dpi
            fixedFont.reset(new QFont(QString()));
            fixedFont->setFamily(font_name);
            fixedFont->setPointSizeF(font_size);
            setResolveMask(*fixedFont, QFont::NoPropertiesResolved);

            return fixedFont.data();
        }

        break;
    }
    default:
        break;
    }

    return QGenericUnixTheme::font(type);
}

static void compelledUpdateScaleLogcailDpi() {
    updateScaleLogcailDpi(QDeepinTheme::getSettings()->scaleLogicalDpi());
}

static void onScreenAdded(QScreen *s) {
    if (QHighDpiScaling::m_screenFactorSet) {
        auto setting = QDeepinTheme::getSettings();
        auto value = setting->screenScaleFactors();

        if (!value.isEmpty() && isGenericScreenScaleFactors(value)) {
            const QByteArray &envValue = getEnvValueByScreenScaleFactors(value);

            qputenv("QT_SCREEN_SCALE_FACTORS", envValue);
            bool ok = false;
            qreal scale = value.toDouble(&ok);

            // 为新的屏幕设置缩放比
            if (ok)
                QHighDpiScaling::setScreenFactor(s, scale);
        }
    }

    compelledUpdateScaleLogcailDpi();
}

DThemeSettings *QDeepinTheme::settings() const
{
    if (!m_settings) {
        m_settings = new DThemeSettings();

        qApp->setProperty("_d_theme_settings_object", (quintptr)m_settings);

        if (enabledRTScreenScale()) {
#ifdef QT_NO_DEBUG
            if (!qEnvironmentVariableIsSet("D_ENABLE_RT_SCALE"))
                return m_settings;
#endif

            QObject::connect(m_settings, &DThemeSettings::scaleFactorChanged,
                             m_settings, onScaleFactorChanged, Qt::UniqueConnection);
            QObject::connect(m_settings, &DThemeSettings::screenScaleFactorsChanged,
                             m_settings, onScreenScaleFactorsChanged, Qt::UniqueConnection);
            QObject::connect(m_settings, &DThemeSettings::scaleLogicalDpiChanged,
                             m_settings, updateScaleLogcailDpi, Qt::UniqueConnection);

            // 当屏幕connected时，Qt库中会更新缩放值，导致dpi的设置被覆盖，此处应该再重新设置dpi值。
            // TODO(zccrs): 当最后一个屏幕disconnectd时，Qt不会移除它，在这种状态下，插入一个新的
            //              屏幕时，Qt只会更新遗留的screen对象的信息，因此不会有screenAdded信号，
            //              而且同样会更新dpi的值，因为目前无法通过Qt api检测到此状态，所以这种情
            //              况下无法自动更新dpi值，将导致应用字体显示大小不受缩放比控制。
            //              !!!
            //              针对 dtk 应用，将在dxcb插件中通知应用更新dpi值。
            qApp->setProperty("_d_updateScaleLogcailDpi", (quintptr)&compelledUpdateScaleLogcailDpi);
            QObject::connect(qApp, &QGuiApplication::screenAdded,
                             m_settings, onScreenAdded,
                             Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));

            if (!qEnvironmentVariableIsSet(DISABLE_UPDATE_WINDOW_GEOMETRY)) {
                QObject::connect(m_settings, &DThemeSettings::autoScaleWindowChanged,
                                 m_settings, onAutoScaleWindowChanged, Qt::UniqueConnection);
                QObject::connect(m_settings, &DThemeSettings::screenScaleFactorsChanged,
                                 m_settings, onAutoScaleWindowChanged, Qt::UniqueConnection);

                onAutoScaleWindowChanged();
            }
        }
    }

    return m_settings;
}

DThemeSettings *QDeepinTheme::getSettings()
{
    return m_settings;
}

QT_END_NAMESPACE

#include "qdeepintheme.moc"

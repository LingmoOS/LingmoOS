// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dguiapplicationhelper.h"
#include "private/dguiapplicationhelper_p.h"
#include "dplatformhandle.h"
#include <DFontManager>
#include <DStandardPaths>

#include <QHash>
#include <QColor>
#include <QPalette>
#include <QWindow>
#include <QGuiApplication>
#include <QPointer>
#include <QPlatformSurfaceEvent>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLoggingCategory>
#include <QDir>
#include <QLockFile>
#include <QDirIterator>
#include <QDesktopServices>
#include <QLibraryInfo>

#ifdef Q_OS_UNIX
#include <QDBusError>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QProcess>
#include <DConfig>
#endif
#include <QDir>
#include <QLockFile>
#include <QDirIterator>
#include <QDesktopServices>

#ifdef Q_OS_UNIX
#include <QDBusError>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QProcess>
#include <DPathBuf>
#endif

#include <private/qguiapplication_p.h>
#include <qpa/qplatformservices.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformtheme.h>

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#ifdef Q_OS_UNIX
class EnvReplaceGuard
{
public:
    EnvReplaceGuard(const int uid);
    ~EnvReplaceGuard();

    char *m_backupLogName;
    char *m_backupHome;
    bool initialized = false;
};

EnvReplaceGuard::EnvReplaceGuard(const int uid)
{
    if (struct passwd *pwd = getpwuid(static_cast<__uid_t>(uid))) {
        m_backupLogName = getenv("LOGNAME");
        m_backupHome = getenv("HOME");

        setenv("LOGNAME", pwd->pw_name, 1);
        setenv("HOME", pwd->pw_dir, 1);
        initialized = true;
    }
}

EnvReplaceGuard::~EnvReplaceGuard()
{
    if (initialized) {
        setenv("LOGNAME", m_backupLogName, 1);
        setenv("HOME", m_backupHome, 1);
    }
}
#endif

DGUI_BEGIN_NAMESPACE

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
using HelperCreator = DGuiApplicationHelper::HelperCreator;
#else
using HelperCreator = DGuiApplicationHelper *(*)();
#endif

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(dgAppHelper, "dtk.dguihelper")
#else
Q_LOGGING_CATEGORY(dgAppHelper, "dtk.dguihelper", QtInfoMsg)
#endif

Q_GLOBAL_STATIC(QLocalServer, _d_singleServer)

static quint8 _d_singleServerVersion = 1;
Q_GLOBAL_STATIC(DFontManager, _globalFM)

#define WINDOW_THEME_KEY "_d_platform_theme"

#define APP_THEME_TYPE "themeType"
Q_GLOBAL_STATIC_WITH_ARGS(DTK_CORE_NAMESPACE::DConfig, _d_dconfig, ("org.deepin.dtk.preference"));

/*!
 @private
 */
class _DGuiApplicationHelper
{
public:
#define INVALID_HELPER reinterpret_cast<DGuiApplicationHelper*>(1)
    inline DGuiApplicationHelper *helper()
    {
        // 临时存储一个无效的指针值, 用于此处条件变量的竞争
        if (m_helper.testAndSetRelaxed(nullptr, INVALID_HELPER)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            m_helper.storeRelaxed(creator());
            m_helper.loadRelaxed()->initialize();
        }
        return m_helper.loadRelaxed();
#else
            m_helper.store(creator());
            m_helper.load()->initialize();
        }
        return m_helper.load();
#endif
    }

    inline void clear()
    {
        if (m_helper != INVALID_HELPER)
            delete m_helper.fetchAndStoreRelaxed(nullptr);
    }

    static DGuiApplicationHelper *defaultCreator()
    {
        return new DGuiApplicationHelper();
    }

    QAtomicPointer<DGuiApplicationHelper> m_helper;
    static HelperCreator creator;
};

class LoadManualServiceWorker : public QThread
{
public:
    explicit LoadManualServiceWorker(QObject *parent = nullptr);
    ~LoadManualServiceWorker() override;
    void checkManualServiceWakeUp();

protected:
    void run() override;
};

LoadManualServiceWorker::LoadManualServiceWorker(QObject *parent)
    : QThread(parent)
{
    if (!parent)
        connect(qApp, &QGuiApplication::aboutToQuit, this, std::bind(&LoadManualServiceWorker::exit, this, 0));
}

LoadManualServiceWorker::~LoadManualServiceWorker()
{
}

void LoadManualServiceWorker::run()
{
    QDBusInterface("com.deepin.Manual.Search",
                   "/com/deepin/Manual/Search",
                   "com.deepin.Manual.Search");
}

void LoadManualServiceWorker::checkManualServiceWakeUp()
{
    if (this->isRunning())
        return;

    start();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class Q_DECL_HIDDEN GuiApplicationEventFilter : public QObject
{
public:
    explicit GuiApplicationEventFilter(DGuiApplicationHelperPrivate *transmitter,
                                       QObject *parent = nullptr)
        : QObject(parent)
        , m_transmitter(transmitter)
    {
    }
    virtual bool eventFilter(QObject *watched, QEvent *event) override
    {
        switch(event->type()) {
        case QEvent::ApplicationFontChange: {
            const QFont font(qGuiApp->font());
            m_transmitter->q_func()->fontChanged(font);
        } break;
        default:
            break;
        }
        return QObject::eventFilter(watched, event);
    }
private:
    DGuiApplicationHelperPrivate *m_transmitter = nullptr;
};
#endif

HelperCreator _DGuiApplicationHelper::creator = _DGuiApplicationHelper::defaultCreator;
Q_GLOBAL_STATIC(_DGuiApplicationHelper, _globalHelper)

int DGuiApplicationHelperPrivate::waitTime = 3000;
DGuiApplicationHelper::Attributes DGuiApplicationHelperPrivate::attributes = DGuiApplicationHelper::UseInactiveColorGroup;
static const DGuiApplicationHelper::SizeMode InvalidSizeMode = static_cast<DGuiApplicationHelper::SizeMode>(-1);

DGuiApplicationHelperPrivate::DGuiApplicationHelperPrivate(DGuiApplicationHelper *qq)
    : DObjectPrivate(qq)
    , explicitSizeMode(InvalidSizeMode)
{

}

void DGuiApplicationHelperPrivate::init()
{
    if (qGuiApp) {
        initApplication(qGuiApp);
    } else {
        // 确保qAddPreRoutine只会被调用一次
        static auto _ = [] {qAddPreRoutine(staticInitApplication); return true;}();
        Q_UNUSED(_)
    }
}

void DGuiApplicationHelperPrivate::initApplication(QGuiApplication *app)
{
    D_Q(DGuiApplicationHelper);

    if (!systemTheme) {
        // 需要在QGuiApplication创建后再创建DPlatformTheme，否则DPlatformTheme无效.
        // qGuiApp->platformFunction()会报警告，并返回nullptr.
        systemTheme = new DPlatformTheme(0, q);
        // 直接对应到系统级别的主题, 不再对外提供为某个单独程序设置主题的接口.
        // 程序设置自身主题相关的东西皆可通过 setPaletteType 和 setApplicationPalette 实现.
        appTheme = systemTheme;
    }

    // 跟随application销毁
    qAddPostRoutine(staticCleanApplication);

    // 转发程序自己变化的信号
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    app->installEventFilter(new GuiApplicationEventFilter(this, app));
#else
    q->connect(app, &QGuiApplication::fontChanged, q, &DGuiApplicationHelper::fontChanged);
#endif
    // TODO handle event in qt6.
    q->connect(app, &QGuiApplication::paletteChanged, q, [this] {
        onApplicationPaletteChanged();
    });

    if (Q_UNLIKELY(!appTheme)) { // 此时说明appTheme可能已经被初始化为了systemtheme
        if (QGuiApplicationPrivate::is_app_running) {
            _q_initApplicationTheme();
        } else {
            // 延后初始化数据，因为在调用 clientLeader 前必须要保证QGuiApplication已经完全构造完成
            q->metaObject()->invokeMethod(q, "_q_initApplicationTheme", Qt::QueuedConnection, Q_ARG(bool, true));
        }
    } else if (appTheme == systemTheme) {
        // 此时appTheme等价于systemTheme, 可以直接信号链接
        _q_initApplicationTheme();
    }

    systemSizeMode = static_cast<DGuiApplicationHelper::SizeMode>(systemTheme->sizeMode());
    q->connect(systemTheme, SIGNAL(sizeModeChanged(int)), q, SLOT(_q_sizeModeChanged(int)));
}

void DGuiApplicationHelperPrivate::staticInitApplication()
{
    if (!_globalHelper.exists())
        return;

    if (DGuiApplicationHelper *helper = _globalHelper->helper())
        helper->d_func()->initApplication(qGuiApp);
}

void DGuiApplicationHelperPrivate::staticCleanApplication()
{
    if (_globalHelper.exists())
        _globalHelper->clear();
}

DPlatformTheme *DGuiApplicationHelperPrivate::initWindow(QWindow *window) const
{
    DPlatformTheme *theme = new DPlatformTheme(window->winId(), q_func()->applicationTheme());
    window->setProperty(WINDOW_THEME_KEY, QVariant::fromValue(theme));
    theme->setParent(window); // 跟随窗口销毁

    auto onWindowThemeChanged = [window, theme, this] {
        // 如果程序自定义了调色板, 则没有必要再关心窗口自身平台主题的变化
        // 需要注意的是, 这里的信号和事件可能会与 notifyAppThemeChanged 中的重复
        // 但是不能因此而移除这里的通知, 当窗口自身所对应的平台主题发生变化时, 这里
        // 的通知机制就有了用武之地.
        if (Q_LIKELY(!isCustomPalette())) {
            qGuiApp->postEvent(window, new QEvent(QEvent::ThemeChange));
        }
    };

    window->connect(theme, &DPlatformTheme::themeNameChanged, window, onWindowThemeChanged);
    window->connect(theme, &DPlatformTheme::activeColorChanged, window, onWindowThemeChanged);
    window->connect(theme, &DPlatformTheme::paletteChanged, window, onWindowThemeChanged);

    return theme;
}

void DGuiApplicationHelperPrivate::_q_initApplicationTheme(bool notifyChange)
{
    if (!appTheme) {
        // DPlatfromHandle::windowLeader依赖platformIntegration
        Q_ASSERT(QGuiApplicationPrivate::platformIntegration());
        appTheme = new DPlatformTheme(DPlatformHandle::windowLeader(), systemTheme);
    }

    QGuiApplication *app = qGuiApp;
    auto onAppThemeChanged = [this] {
        // 只有当程序未自定义调色板时才需要关心DPlatformTheme中themeName和palette的改变
        if (!isCustomPalette())
            notifyAppThemeChanged();
    };
    // 监听与程序主题相关的改变
    QObject::connect(appTheme, &DPlatformTheme::themeNameChanged, app, onAppThemeChanged);
    QObject::connect(appTheme, &DPlatformTheme::paletteChanged, app, onAppThemeChanged);
    QObject::connect(appTheme, &DPlatformTheme::activeColorChanged, app, [this] {
        if (!appPalette)
            notifyAppThemeChanged();
    });

    // appTheme在此之前可能由systemTheme所代替被使用，此时在创建appTheme
    // 并初始化之后，应当发送信号通知程序主题的改变
    if (notifyChange && appTheme->isValid()) {
        notifyAppThemeChanged();
    }
}

void DGuiApplicationHelperPrivate::notifyAppThemeChanged()
{
    D_Q(DGuiApplicationHelper);
    notifyAppThemeChangedByEvent();
    // 通知主题类型发生变化, 此处可能存在误报的行为, 不过不应该对此做额外的约束
    // 此信号的行为应当等价于 applicationPaletteChanged
    Q_EMIT q->themeTypeChanged(q->themeType());
    // 通知调色板对象的改变
    Q_EMIT q->applicationPaletteChanged();
}

void DGuiApplicationHelperPrivate::notifyAppThemeChangedByEvent()
{
    QWindowSystemInterfacePrivate::ThemeChangeEvent event(nullptr);
    // 此事件会促使QGuiApplication重新从QPlatformTheme中获取系统级别的QPalette.
    // 而在deepin平台下, 系统级别的QPalette来源自 \a applicationPalette()
    QGuiApplicationPrivate::processThemeChanged(&event);
}

void DGuiApplicationHelperPrivate::onApplicationPaletteChanged()
{
    D_Q(DGuiApplicationHelper);
    // 如果用户没有自定义颜色类型, 则应该通知程序的颜色类型发送变化
    if (Q_LIKELY(!isCustomPalette())) {
        Q_EMIT q->themeTypeChanged(q->toColorType(qGuiApp->palette()));
        Q_EMIT q->applicationPaletteChanged();
    } else {
        qWarning() << "DGuiApplicationHelper: Don't use QGuiApplication::setPalette on DTK application.";
    }
}

bool DGuiApplicationHelperPrivate::isCustomPalette() const
{
    return appPalette || paletteType != DGuiApplicationHelper::UnknownType;
}

void DGuiApplicationHelperPrivate::setPaletteType(DGuiApplicationHelper::ColorType ct, bool emitSignal)
{
    if (paletteType == ct)
        return;

    if (qGuiApp && qGuiApp->testAttribute(Qt::AA_SetPalette))
        qWarning() << "DGuiApplicationHelper: Plase check 'QGuiApplication::setPalette',"
                      " Don't use it on DTK application.";

    paletteType = ct;

    if (!emitSignal) {
        notifyAppThemeChangedByEvent();
        return;
    }

    // 如果未固定调色板, 则paletteType的变化可能会导致调色板改变, 应当通知程序更新数据
    if (!appPalette)
        notifyAppThemeChanged();

    D_Q(DGuiApplicationHelper);
    Q_EMIT q->paletteTypeChanged(paletteType);
}

void DGuiApplicationHelperPrivate::initPaletteType() const
{
    DCORE_USE_NAMESPACE
    if (DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::DontSaveApplicationTheme))
        return;

    if (_d_dconfig.exists())
        return;

    auto applyThemeType = [this](bool emitSignal){
        int ct = _d_dconfig->value(APP_THEME_TYPE, DGuiApplicationHelper::UnknownType).toInt();
        if (ct > DGuiApplicationHelper::DarkType || ct < DGuiApplicationHelper::UnknownType)
            ct = DGuiApplicationHelper::UnknownType;

        const_cast<DGuiApplicationHelperPrivate *>(this)->setPaletteType(DGuiApplicationHelper::ColorType(ct), emitSignal);
    };

    applyThemeType(false);

    QObject::connect(_d_dconfig, &DConfig::valueChanged, _d_dconfig, [applyThemeType](const QString &key){
        if (key != APP_THEME_TYPE)
            return;

        applyThemeType(true);
    });
}

void DGuiApplicationHelperPrivate::_q_sizeModeChanged(int mode)
{
    D_Q(DGuiApplicationHelper);
    qCInfo(dgAppHelper) << "Receiving that system size mode is set to [" << static_cast<DGuiApplicationHelper::SizeMode>(mode)
                        << "], and old system size mode is [" << systemSizeMode << "]";

    const auto oldSizeMode = fetchSizeMode();
    systemSizeMode = static_cast<DGuiApplicationHelper::SizeMode>(mode);
    const auto currentSizeMode = fetchSizeMode();
    if (oldSizeMode != currentSizeMode)
        Q_EMIT q->sizeModeChanged(currentSizeMode);
}

DGuiApplicationHelper::SizeMode DGuiApplicationHelperPrivate::fetchSizeMode(bool *isSystemSizeMode) const
{
    if (isSystemSizeMode)
        *isSystemSizeMode = false;
    // `setSizeMode` > `D_DTK_SIZEMODE` > `systemSizeMode`
    if (explicitSizeMode != InvalidSizeMode)
        return explicitSizeMode;

    static const QString envSizeMode(qEnvironmentVariable("D_DTK_SIZEMODE"));
    if (!envSizeMode.isEmpty()) {
        bool ok = false;
        const auto mode = envSizeMode.toInt(&ok);
        if (ok)
            return static_cast<DGuiApplicationHelper::SizeMode>(mode);
    }

    if (isSystemSizeMode)
        *isSystemSizeMode = true;
    return systemSizeMode;
}

/*!
  \class Dtk::Gui::DGuiApplicationHelper
  \inmodule dtkgui
  \brief DGuiApplicationHelper 应用程序的 GUI ，如主题、调色板等.
 */

/*!
  \enum DGuiApplicationHelper::ColorType
  DGuiApplicationHelper::ColorType 定义了主题类型.
  
  \var DGuiApplicationHelper::ColorType DGuiApplicationHelper::UnknownType
  未知主题(浅色主题或深色主题)
  
  \var DGuiApplicationHelper::ColorType DGuiApplicationHelper::LightType
  浅色主题
  
  \var DGuiApplicationHelper::ColorType DGuiApplicationHelper::DarkType
  深色主题
 */

/*!
  \enum DGuiApplicationHelper::Attribute
  DGuiApplicationHelper::Attribute 定义了功能属性
  
  \var DGuiApplicationHelper::Attribute DGuiApplicationHelper::UseInactiveColorGroup
  如果开启，当窗口处于Inactive状态时就会使用QPalette::Inactive的颜色，否则窗口将没有任何颜色变化。
  
  \var DGuiApplicationHelper::Attribute DGuiApplicationHelper::ColorCompositing
  是否采用半透明样式的调色板。
  
  \var DGuiApplicationHelper::Attribute DGuiApplicationHelper::ReadOnlyLimit
  区分只读枚举。
  
  \var DGuiApplicationHelper::Attribute DGuiApplicationHelper::IsDeepinPlatformTheme
  获取当前是否使用deepin的platformtheme插件，platformtheme插件可以为Qt程序提供特定的控件样式，默认使用chameleon主题。
  
  \var DGuiApplicationHelper::Attribute DGuiApplicationHelper::IsDXcbPlatform
  获取当前使用的是不是dtk的xcb窗口插件，dxcb插件提供了窗口圆角和阴影功能。
  
  \var DGuiApplicationHelper::Attribute DGuiApplicationHelper::IsXWindowPlatform
  获取当前是否运行在X11环境中。
  
  \var DGuiApplicationHelper::Attribute DGuiApplicationHelper::IsTableEnvironment
  获取当前是否运行在deepin平板环境中，检测XDG_CURRENT_DESKTOP环境变量是不是tablet结尾。
  
  \var DGuiApplicationHelper::Attribute DGuiApplicationHelper::IsDeepinEnvironment
  获取当前是否运行在deepin桌面环境中，检测XDG_CURRENT_DESKTOP环境变量是不是deepin。
 */

DGuiApplicationHelper::DGuiApplicationHelper()
    : QObject(nullptr)
    , DObject(*new DGuiApplicationHelperPrivate(this))
{

}

void DGuiApplicationHelper::initialize()
{
    D_D(DGuiApplicationHelper);

    d->init();
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
/*!
  \brief 创建 DGuiApplicationHelper 对象.

  \param creator 函数指针
  \note 一定要先调用此函数,再使用 DGuiApplicationHelper::instance()
 */
void DGuiApplicationHelper::registerInstanceCreator(DGuiApplicationHelper::HelperCreator creator)
{
    if (creator == _DGuiApplicationHelper::creator)
        return;

    _DGuiApplicationHelper::creator = creator;

    if (_globalHelper.exists()) {
        _globalHelper->clear();
    }
}
#endif

inline static int adjustColorValue(int base, qint8 increment, int max = 255)
{
    return increment > 0 ? (max - base) * increment / 100.0 + base
           : base * (1 + increment / 100.0);
}

/*!
  \brief DGuiApplicationHelper::instance返回 DGuiApplicationHelper 对象
  \return DGuiApplicationHelper对象
 */
DGuiApplicationHelper *DGuiApplicationHelper::instance()
{
    return _globalHelper->helper();
}

DGuiApplicationHelper::~DGuiApplicationHelper()
{
    _globalHelper->m_helper = nullptr;
}

static inline QColor adjustHSLColor(const QColor &base, qint8 hueFloat, qint8 saturationFloat,
                                    qint8 lightnessFloat, qint8 alphaFloat = 0)
{
    if (Q_LIKELY(hueFloat || saturationFloat || lightnessFloat || alphaFloat)) {
        // 按HSL格式调整
        int H, S, L, A;
        base.getHsl(&H, &S, &L, &A);

        H = H > 0 ? adjustColorValue(H, hueFloat, 359) : H;
        S = adjustColorValue(S, saturationFloat);
        L = adjustColorValue(L, lightnessFloat);

        return QColor::fromHsl(H, S, L, A);
    }

    return base;
}

static inline QColor adjustRGBColor(const QColor &base, qint8 redFloat, qint8 greenFloat,
                                    qint8 blueFloat, qint8 alphaFloat = 0)
{
    if (Q_LIKELY(redFloat || greenFloat || blueFloat || alphaFloat)) {
        // 按RGB格式调整
        int R, G, B, A;
        base.getRgb(&R, &G, &B, &A);

        R = adjustColorValue(R, redFloat);
        G = adjustColorValue(G, greenFloat);
        B = adjustColorValue(B, blueFloat);
        A = adjustColorValue(A, alphaFloat);

        return QColor(R, G, B, A);
    }

    return base;
}

/*!
  \brief 调整颜色.

  \note 取值范围均为 -100 ~ 100 ,当三原色参数为-100时，颜色为黑色，参数为100时，颜色为白色.
  以透明度( alphaFloat )为例,当参数为负数时基础色的 alphaFloat 值减少，现象偏向透明, 参数为正数alphaFloat 值增加，现象偏不透明
  \param base 基础色
  \param hueFloat 色调
  \param saturationFloat 饱和度
  \param lightnessFloat 亮度
  \param redFloat 红色
  \param greenFloat 绿色
  \param blueFloat 蓝色
  \param alphaFloat Alpha通道(透明度)
  \return 经过调整的颜色
 */
QColor DGuiApplicationHelper::adjustColor(const QColor &base,
                                          qint8 hueFloat, qint8 saturationFloat, qint8 lightnessFloat,
                                          qint8 redFloat, qint8 greenFloat, qint8 blueFloat, qint8 alphaFloat)
{
    if (Q_UNLIKELY(!base.isValid()))
        return base;

    // First do the adjust of spec of this color, Avoid precision loss caused by color spec conversion.
    if (Q_UNLIKELY(base.spec() == QColor::Hsl)) {
        QColor newColor = adjustHSLColor(base, hueFloat, saturationFloat, lightnessFloat, alphaFloat);
        return adjustRGBColor(newColor, redFloat, greenFloat, blueFloat);
    }

    // For other specs, first do RGB adjust
    QColor newColor = adjustRGBColor(base, redFloat, greenFloat, blueFloat, alphaFloat);
    return adjustHSLColor(newColor, hueFloat, saturationFloat, lightnessFloat);
}

/*!
  \brief 调整图片整体像素颜色.

  \note 取值范围均为 -100 ~ 100 ,当三原色参数为-100时，颜色为黑色，参数为100时，颜色为白色.
  以透明度( alphaFloat )为例,当参数为负数时基础色的 alphaFloat 值减少，现象偏向透明, 参数为正数alphaFloat 值增加，现象偏不透明
  \param base 基础色
  \param hueFloat 色调
  \param saturationFloat 饱和度
  \param lightnessFloat 亮度
  \param redFloat 红色
  \param greenFloat 绿色
  \param blueFloat 蓝色
  \param alphaFloat Alpha通道(透明度)
  \return 经过调整的图片
 */
QImage DGuiApplicationHelper::adjustColor(const QImage &base, qint8 hueFloat, qint8 saturationFloat, qint8 lightnessFloat, qint8 redFloat, qint8 greenFloat, qint8 blueFloat, qint8 alphaFloat)
{
    if (base.isNull())
        return base;

    if (!hueFloat && !saturationFloat && !lightnessFloat && !redFloat
            && !greenFloat && !blueFloat && !alphaFloat)
        return base;

    QImage dest = base;
    for (int y = 0; y < dest.height(); ++y) {
        const QRgb *rgb = reinterpret_cast<const QRgb *>(base.scanLine(y));
        for (int x = 0; x < dest.width(); ++x) {
            QColor base = QColor::fromRgba(rgb[x]);
            if (base.alpha() == 0)
                continue;
            QColor color = adjustColor(base, hueFloat, saturationFloat, lightnessFloat,
                                       redFloat, greenFloat, blueFloat, alphaFloat);
            dest.setPixel(x, y, color.rgba());
        }
    }
    return dest;
}

/*!
  \brief 将两种颜色混合，合成新的颜色.

  \param substrate 底层颜色
  \param superstratum 上层颜色
  \return 混合颜色
 */
QColor DGuiApplicationHelper::blendColor(const QColor &substrate, const QColor &superstratum)
{
    QColor c2 = superstratum.toRgb();

    if (c2.alpha() >= 255)
        return c2;

    QColor c1 = substrate.toRgb();
    qreal c1_weight = 1 - c2.alphaF();

    int r = c1_weight * c1.red() + c2.alphaF() * c2.red();
    int g = c1_weight * c1.green() + c2.alphaF() * c2.green();
    int b = c1_weight * c1.blue() + c2.alphaF() * c2.blue();

    return QColor(r, g, b, c1.alpha());
}

static QColor light_qpalette[QPalette::NColorRoles] {
    QColor(0, 0, 0, 0.7 * 255),         //WindowText
    QColor("#e5e5e5"),                  //Button
    QColor("#e6e6e6"),                  //Light
    QColor("#e5e5e5"),                  //Midlight
    QColor("#e3e3e3"),                  //Dark
    QColor("#e4e4e4"),                  //Mid
    QColor(0, 0, 0, 0.7 * 255),         //Text
    Qt::black,                          //BrightText
    QColor(0, 0, 0, 0.7 * 255),         //ButtonText
    Qt::white,                          //Base
    QColor("#f8f8f8"),                  //Window
    QColor(0, 0, 0, 0.05 * 255),        //Shadow
    QColor("#0081ff"),                  //Highlight
    Qt::white,                          //HighlightedText
    QColor("#0082fa"),                  //Link
    QColor("#ad4579"),                  //LinkVisited
    QColor(0, 0, 0, 0.03 * 255),        //AlternateBase
    Qt::white,                          //NoRole
    QColor(255, 255, 255, 0.8 * 255),   //ToolTipBase
    QColor(0, 0, 0, 0.85 * 255)         //ToolTipText
};

static QColor dark_qpalette[QPalette::NColorRoles] {
    QColor(255, 255, 255, 0.7 * 255),   //WindowText
    QColor("#444444"),                  //Button
    QColor("#484848"),                  //Light
    QColor("#474747"),                  //Midlight
    QColor("#414141"),                  //Dark
    QColor("#434343"),                  //Mid
    QColor(255, 255, 255, 0.7 * 255),   //Text
    Qt::white,                          //BrightText
    QColor(255, 255, 255, 0.7 * 255),   //ButtonText
    QColor("#282828"),                  //Base
    QColor("#252525"),                  //Window
    QColor(0, 0, 0, 0.05 * 255),        //Shadow
    QColor("#0081ff"),                  //Highlight
    QColor("#F1F6FF"),                  //HighlightedText
    QColor("#0082fa"),                  //Link
    QColor("#ad4579"),                  //LinkVisited
    QColor(0, 0, 0, 0.05 * 255),        //AlternateBase
    Qt::black,                          //NoRole
    QColor(45, 45, 45, 0.8 * 255),      //ToolTipBase
    QColor(255, 255, 255, 0.85 * 255)   //ToolTipText
};

static QColor light_dpalette[DPalette::NColorTypes] {
    QColor(),                       //NoType
    QColor(0, 0, 0, 255 * 0.03),    //ItemBackground
    QColor(0, 0, 0, 0.85 * 255),    //TextTitle
    QColor(0, 0, 0, 0.6 * 255),     //TextTips
    QColor("#FF5736"),              //TextWarning
    Qt::black,                      //TextLively
    QColor("#0081FF"),              //LightLively
    QColor("#0081FF"),              //DarkLively
    QColor(0, 0, 0, 0.05 * 255),    //FrameBorder
    QColor(85, 85, 85, 0.4 * 255),  //PlaceholderText
    QColor(0, 0, 0, 0.1 * 255),     //FrameShadowBorder
    QColor(0, 0, 0, 0.1 * 255)      //ObviousBackground
};

static QColor dark_dpalette[DPalette::NColorTypes] {
    QColor(),                           //NoType
    QColor(255, 255, 255, 255 * 0.05),  //ItemBackground
    QColor(255, 255, 255, 0.85 * 255),  //TextTitle
    QColor(255, 255, 255, 0.6 * 255),   //TextTips
    QColor("#E43F2E"),                  //TextWarning
    Qt::white,                          //TextLively
    QColor("#0059d2"),                  //LightLively
    QColor("#0059d2"),                  //DarkLively
    QColor(255, 255, 255, 0.1 * 255),   //FrameBorder
    QColor(192, 198, 212, 0.4 * 255),   //PlaceholderText
    QColor(0, 0, 0, 0.8 * 255),         //FrameShadowBorder
    QColor(255, 255, 255, 0.1 * 255)    //ObviousBackground
};

/*!
  \brief 根据主题获取标准调色板.

  \param type 主题枚举值
  \return 调色板
 */
DPalette DGuiApplicationHelper::standardPalette(DGuiApplicationHelper::ColorType type)
{
    static const DPalette *light_palette = nullptr, *dark_palette = nullptr;
    static const DPalette *alpha_light_palette = nullptr, *alpha_dark_palette = nullptr;
    const bool allowCompositingColor = DGuiApplicationHelper::testAttribute(ColorCompositing);

    if (type == LightType) {
        if (Q_UNLIKELY(allowCompositingColor)) {
            if (Q_LIKELY(alpha_light_palette)) {
                return *alpha_light_palette;
            }
        }

        if (Q_LIKELY(light_palette)) {
            return *light_palette;
        }
    } else if (type == DarkType) {
        if (Q_UNLIKELY(allowCompositingColor)) {
            if (Q_LIKELY(alpha_dark_palette)) {
                return *alpha_dark_palette;
            }
        }

        if (Q_LIKELY(dark_palette)) {
            return *dark_palette;
        }
    } else {
        return DPalette();
    }

    DPalette *pa;
    const QColor *qcolor_list, *dcolor_list;

    if (type == DarkType) {
        pa = new DPalette();

        if (allowCompositingColor)
            alpha_dark_palette = pa;
        else
            dark_palette = pa;

        qcolor_list = dark_qpalette;
        dcolor_list = dark_dpalette;
    } else {
        pa = new DPalette();

        if (allowCompositingColor)
            alpha_light_palette = pa;
        else
            light_palette = pa;

        qcolor_list = light_qpalette;
        dcolor_list = light_dpalette;
    }

    for (int i = 0; i < DPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);
        QColor color = qcolor_list[i];

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        if (role == QPalette::PlaceholderText) {
            color = dcolor_list[DPalette::PlaceholderText];
        }
#endif
        // 处理半透明色
        if (allowCompositingColor) {
            switch (role) {
            case QPalette::Window:
                color = type == LightType ? adjustColor(color, 0, 0, 0, 0, 0, 0, -20) : adjustColor(color, 0, 0, -10, 0, 0, 0, -20);
                break;
            case QPalette::Base:
                color = adjustColor(color, 0, 0, 0, 0, 0, 0, -20);
                break;
            case QPalette::WindowText:
            case QPalette::Text:
                color = adjustColor(color, 0, 0, type == LightType ? -20 : +20, 0, 0, 0, -20);
                break;
            case QPalette::ButtonText:
                color = type == LightType ? adjustColor(color, 0, 0, -20, 0, 0, 0, -20) : adjustColor(color, 0, 0, +20, 0, 0, 0, 0);
                break;
            case QPalette::Button:
            case QPalette::Light:
            case QPalette::Mid:
            case QPalette::Midlight:
            case QPalette::Dark:
                color = adjustColor(color, 0, 0, -20, 0, 0, 0, -40);
                break;
            default:
                break;
            }
        }

        pa->setColor(DPalette::Active, role, color);
        generatePaletteColor(*pa, role, type);
    }

    for (int i = 0; i < DPalette::NColorTypes; ++i) {
        DPalette::ColorType role = static_cast<DPalette::ColorType>(i);
        QColor color = dcolor_list[i];

        // 处理半透明色
        if (allowCompositingColor) {
            switch (role) {
            case DPalette::ItemBackground:
                color = adjustColor(color, 0, 0, 100, 0, 0, 0, type == LightType ? -80 : -90);
                break;
            case DPalette::TextTitle:
                color = adjustColor(color, 0, 0, -20, 0, 0, 0, -20);
                break;
            case DPalette::TextTips:
                color = type == LightType ? adjustColor(color, 0, 0, -40, 0, 0, 0, -40) : adjustColor(color, 0, 0, +40, 0, 0, 0, -50);
                break;
            default:
                break;
            }
        }

        pa->setColor(DPalette::Active, role, color);
        generatePaletteColor(*pa, role, type);
    }

    return *const_cast<const DPalette*>(pa);
}

template<typename M>
static void generatePaletteColor_helper(DPalette &base, M role, DGuiApplicationHelper::ColorType type)
{
    if (type == DGuiApplicationHelper::UnknownType) {
        type = DGuiApplicationHelper::toColorType(base);
    }

    QColor disable_mask_color, inactive_mask_color;

    if (type == DGuiApplicationHelper::DarkType) {
        disable_mask_color = dark_qpalette[QPalette::Window];
        inactive_mask_color = dark_qpalette[QPalette::Window];
        disable_mask_color.setAlphaF(0.7);
        inactive_mask_color.setAlphaF(0.6);
    } else {
        disable_mask_color = light_qpalette[QPalette::Window];
        inactive_mask_color = light_qpalette[QPalette::Window];
        disable_mask_color.setAlphaF(0.6);
        inactive_mask_color.setAlphaF(0.4);
    }

    const QColor &color = base.color(QPalette::Normal, role);
    base.setColor(QPalette::Disabled, role, DGuiApplicationHelper::blendColor(color, disable_mask_color));

    QPalette::ColorRole qr = static_cast<QPalette::ColorRole>(role);
    if (qr == QPalette::Text) {
        // disable text color olny -60% alpha
        base.setColor(QPalette::Disabled, role, DGuiApplicationHelper::adjustColor(color, 0, 0, 0, 0, 0, 0, -60));
    }

    if (DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::Attribute::UseInactiveColorGroup))
        base.setColor(QPalette::Inactive, role, DGuiApplicationHelper::blendColor(color, inactive_mask_color));
    else
        base.setColor(QPalette::Inactive, role, color);
}

/*!
  \brief 获取调色板颜色.

  \param base 调色板
  \param role 色码
  \param type 主题枚举值
  \sa QPalette::ColorRole
 */
void DGuiApplicationHelper::generatePaletteColor(DPalette &base, QPalette::ColorRole role, DGuiApplicationHelper::ColorType type)
{
    if (role == QPalette::Window) {
        const QBrush &window = base.brush(QPalette::Normal, role);
        base.setBrush(QPalette::Disabled, role, window);
        base.setBrush(QPalette::Inactive, role, window);
        return;
    } else if (role == QPalette::Highlight && toColorType(base) == DarkType) {
        // 暗色模式下的高亮色亮度要降低10%，避免太突兀
        QColor highlight = base.highlight().color();

        if (highlight.isValid()) {
            base.setColor(QPalette::Highlight, adjustColor(highlight, 0, 0, -20, 0, 0, 0, 0));
        }
    }

    generatePaletteColor_helper(base, role, type);
}

/*!
  \brief 加工调色板的颜色.
  \overload

   一般我们只会为调色板的 QPalette::Normal 组设置颜色值, 但是
  控件中也需要使用其他组的颜色, 此函数使用一些固定规则为 \a base 填充 QPalette::Disabled
  和 QPalette::Inactive 分组的颜色. 不同的颜色类型会使用不同的加工规则, 如果为 LightType
  类型, 则将颜色的alpha通道调整为 0.6 后作为 QPalette::Disabled 类的颜色使用, 调整为 0.4 后
  作为 QPalette::Inactive 类的颜色使用. 如果为 DarkType 类型, 则将颜色的alpha通道调整为
  0.7 后作为 QPalette::Disabled 类的颜色使用, 调整为 0.6 后作为 QPalette::Inactive
  类的颜色使用.

  \param base 被加工的调色板
  \param role 加工的项
  \param type 加工时所使用的颜色类型, 如果值为 UnknownType 将使用 toColorType 获取颜色类型
 */
void DGuiApplicationHelper::generatePaletteColor(DPalette &base, DPalette::ColorType role, DGuiApplicationHelper::ColorType type)
{
    generatePaletteColor_helper(base, role, type);
}

/*!
  \brief 加工调色板的颜色.
  \overload

  同 generatePaletteColor, 将直接调用 generatePaletteColor 加工
  所有类型的调色板颜色.

  \param base 被加工的调色板
  \param type 加工时所使用的颜色类型, 如果值为 UnknownType 将使用 toColorType 获取颜色类型
 */
void DGuiApplicationHelper::generatePalette(DPalette &base, ColorType type)
{
    // 先判断调色板的色调
    if (type == UnknownType) {
        type = toColorType(base);
    }

    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);
        generatePaletteColor(base, role, type);
    }

    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        DPalette::ColorType role = static_cast<DPalette::ColorType>(i);
        generatePaletteColor(base, role, type);
    }
}

/*!
  \brief 获取调色板数据.

  首先根据 DPlatformTheme::themeName 获取主题的颜色类型, 如果名称以
   "dark" 结尾则认为其颜色类型为 DarkType, 否则为 LightType.
  如果主题名称为空, 将使用其父主题的名称( DPlatformTheme::fallbackProperty ).
  根据颜色类型将使用 standardPalette 获取基础调色板数据, 在此基础上
  从 DPlatformTheme::fetchPalette 获取最终的调色板.

  \param theme 平台主题对象
  \return 调色板数据
 */
DPalette DGuiApplicationHelper::fetchPalette(const DPlatformTheme *theme)
{
    DPalette base_palette;
    const QByteArray theme_name = theme->themeName();
    ColorType type = LightType;

    // 深色主题
    if (theme_name.endsWith("dark")) {
        type = DarkType;
    }

    bool ok = false;
    base_palette = theme->fetchPalette(standardPalette(type), &ok);
    const QColor &active_color = theme->activeColor();

    if (active_color.isValid()) {
        base_palette.setColor(QPalette::Normal, QPalette::Highlight, active_color);

        // ok为true时会整体处理palette的所有颜色
        if (!ok) {
            generatePaletteColor(base_palette, QPalette::Highlight, type);
        }
    }

    if (ok) {
        generatePalette(base_palette, type);
    }

    return base_palette;
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
/*!
  \brief 设置是否将调色板的颜色改为半透明模式.

  一般用在主窗口背景为透明、模糊的程序中
  \param on 是否开启
 */
void DGuiApplicationHelper::setUseInactiveColorGroup(bool on)
{
    DGuiApplicationHelper::setAttribute(Attribute::UseInactiveColorGroup, on);
}

/*!
  \brief 设置是否开启混合颜色.

  \param on 是否开启
 */
void DGuiApplicationHelper::setColorCompositingEnabled(bool on)
{
    DGuiApplicationHelper::setAttribute(Attribute::ColorCompositing, on);
}
#endif

bool DGuiApplicationHelper::isXWindowPlatform()
{
    return DGuiApplicationHelper::testAttribute(Attribute::IsXWindowPlatform);
}

/*!
  \brief 用于判断当前桌面环境是否是平板电脑环境.

  \return true 是平板电脑环境 false不是平板电脑环境
 */
bool DGuiApplicationHelper::isTabletEnvironment()
{
    return DGuiApplicationHelper::testAttribute(Attribute::IsTableEnvironment);
}

/*!
   \brief isAnimationEnvironment 用于判断当前桌面环境是否是开启了动画等特效的环境
   \return true开启了 false没有开启
 */
bool DGuiApplicationHelper::isSpecialEffectsEnvironment()
{
    return DGuiApplicationHelper::testAttribute(Attribute::IsSpecialEffectsEnvironment);
}

/*!
  \brief DGuiApplicationHelper::systemTheme.

  返回系统级别的主题, 优先级低于 applicationTheme
  \return 平台主题对象
  \sa applicationTheme
  \sa windowTheme
 */
DPlatformTheme *DGuiApplicationHelper::systemTheme() const
{
    D_DC(DGuiApplicationHelper);

    return d->systemTheme;
}

/*!
  \brief DGuiApplicationHelper::applicationTheme.

  同 systemTheme
  \return 平台主题对象
  \sa systemTheme
  \sa windowTheme
 */
DPlatformTheme *DGuiApplicationHelper::applicationTheme() const
{
    D_DC(DGuiApplicationHelper);

    // 如果appTheme还未初始化，应当先初始化appTheme
    if (Q_UNLIKELY(!d->appTheme)) {
        // 初始程序级别的主题对象
        const_cast<DGuiApplicationHelperPrivate*>(d)->_q_initApplicationTheme(false);
    }

    return d->appTheme;
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
/*!
  \brief 返回窗口级别的主题, 优先级高于 windowTheme 和 systemTheme.

  \param window 主题对象对应的窗口
  \return 平台主题对象
  \sa applicationTheme()
  \warning 已废弃, 不再对外暴露为特定窗口设置主题的接口
 */
DPlatformTheme *DGuiApplicationHelper::windowTheme(QWindow *window) const
{
    DPlatformTheme *theme = qvariant_cast<DPlatformTheme*>(window->property(WINDOW_THEME_KEY));

    if (theme) {
        return theme;
    }

    D_DC(DGuiApplicationHelper);
    theme = d->initWindow(window);

    return theme;
}
#endif

/*!
  \brief 返回应用程序调色板.

  如果使用 setApplicationPalette 设置过一个有效的调色板, 将直接返回保存的调色板. 否则
  先计算调色板的ColorType, 再使用这个颜色类型通过 standardPalette 获取标准调色板. 计算
  ColorType的数据来源按优先级从高到低排列有以下几种方式:
  1. 如果使用 setThemeType 设置过一个有效的颜色类型, 将直接使用 themeType 的值.
  2. 如果为QGuiApplication设置过调色板(表现为 QGuiApplication::testAttribute(Qt::AA_SetPalette)
  为true), 则将使用 QGuiApplication::palette 通过 toColorType 获取颜色类型.
  3. 将根据 applicationTheme 的 DPlatformTheme::themeName 计算颜色类型.
  如果ColorType来源自第2种方式, 则会直接使用 QGuiApplication::palette 覆盖标准调色板中的
  QPalette 部分, 且程序不会再跟随系统的活动色自动更新调色板.
  \warning 不应该在DTK程序中使用QGuiApplication/QApplication::setPalette
  \return 应用程序调色板
 */
DPalette DGuiApplicationHelper::applicationPalette(ColorType paletteType) const
{
    D_DC(DGuiApplicationHelper);

    if (d->appPalette) {
        return *d->appPalette;
    }

    ColorType type = paletteType;
    bool aa_setPalette = qGuiApp && qGuiApp->testAttribute(Qt::AA_SetPalette);
    // 此时appTheme可能还未初始化, 因此先使用systemTheme, 待appTheme初始化之后会
    // 通知程序调色板发生改变
    auto theme = Q_LIKELY(d->appTheme) ? d->appTheme : d->systemTheme;

    if (type == UnknownType) {
        if (aa_setPalette) {
            type = toColorType(qGuiApp->palette());
        } else {
            // 如果程序未自定义调色板, 则直接从平台主题中获取调色板数据
            return fetchPalette(theme);
        }
    }

    // 如果程序自定义了palette的类型，将忽略 appTheme 中设置的调色板数据.
    DPalette pa = standardPalette(type);

    if (aa_setPalette) {
        // 如果程序通过QGuiApplication::setPalette自定义了调色板, 则应当尊重程序的选择
        // 覆盖DPalette中的的QPalette数据
        pa.QPalette::operator =(qGuiApp->palette());
    } else {
        const QColor &active_color = theme->activeColor();

        if (active_color.isValid()) {
            // 应用Active Color
            pa.setColor(QPalette::Normal, QPalette::Highlight, active_color);
            generatePaletteColor(pa, QPalette::Highlight, type);
        }
    }

    return pa;
}

DPalette DGuiApplicationHelper::applicationPalette() const
{
    D_DC(DGuiApplicationHelper);

    return applicationPalette(d->paletteType);
}

/*!
  \brief DGuiApplicationHelper::setApplicationPalette.

  自定义应用程序调色板, 如果没有为 QGuiApplication 设置过 QPalette, 则
  将触发 QGuiApplication::palette 的更新. 如果仅需要控制程序使用亮色还是暗色的
  调色板, 请使用 setThemeType.
  \note 主动设置调色板的操作会导致程序不再使用 DPlatformTheme 中调色板相关的数据, 也
  包括窗口级别的 windowTheme 所对应的 DPlatformTheme, 届时设置 DPlatformTheme
  的 themeName 和所有与 \a palette 相关的属性都不再生效.
  \warning 使用此方式设置的调色板将不会自动跟随活动色的变化
  \warning 如果使用过QGuiApplication::setPalette, 此方式可能不会生效
  \param palette 要设置的调色板
 */
void DGuiApplicationHelper::setApplicationPalette(const DPalette &palette)
{
    D_D(DGuiApplicationHelper);

    if (qGuiApp && qGuiApp->testAttribute(Qt::AA_SetPalette)) {
        qWarning() << "DGuiApplicationHelper: Plase check 'QGuiApplication::setPalette', Don't use it on DTK application.";
    }

    auto resolve = [](const DPalette &palette) ->bool {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return palette.resolveMask();
#else
    return palette.resolve();
#endif
    };

    if (d->appPalette) {
        if (resolve(palette)) {
            *d->appPalette = palette;
        } else {
            d->appPalette.reset();
        }
    } else if (resolve(palette)) {
        d->appPalette.reset(new DPalette(palette));
    } else {
        return;
    }

    // 通知QGuiApplication更新自身的palette和font
    d->notifyAppThemeChanged();
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
/*!
  \brief DGuiApplicationHelper::windowPalette.

  返回窗口所对应的调色板数据, 同 applicationPalette, 如果程序中自定义了
  调色板, 则直接使用 applicationPalette. 自定义调色板的三种方式如下:
  1. 通过 setApplicationPalette 固定调色板
  2. 通过 setThemeType 固定调色板的类型
  3. 通过 QGuiApplication::setPalette 固定调色板, 需要注意此方法不可逆.
  否则将基于窗口所对应的 DPlatformTheme 获取调色板( fetchPalette).
  \param window
  \return 调色板
  \sa windowTheme(), fetchPalette(), standardPalette(), generatePalette(), applicationPalette()
  \warning 使用时要同时关注 applicationPaletteChanged() , 收到此信号后可能需要重新获取窗口的调色板
  \warning 已废弃, 不再对外暴露控制窗口级别调色板的接口
 */
DPalette DGuiApplicationHelper::windowPalette(QWindow *window) const
{
    D_DC(DGuiApplicationHelper);

    // 如果程序自定义了调色版, 则不再关心窗口对应的平台主题上的设置
    if (Q_UNLIKELY(d->isCustomPalette())) {
        return applicationPalette();
    }

    DPlatformTheme *theme = windowTheme(window);

    if (Q_UNLIKELY(!theme)) {
        return applicationPalette();
    }

    return fetchPalette(theme);
}
#endif

/*!
  \brief DGuiApplicationHelper::fontManager.

  程序中唯一的DFontManager对象, 会根据程序的fontChanged信号
  更新 DFontManager::baseFontPixelSize
  \warning 请不要尝试更改它的 baseFontPixelSize 属性
 */
const DFontManager *DGuiApplicationHelper::fontManager() const
{
    // 为对象初始化信号链接
    if (!_globalFM.exists()) {
        // fontManager's base font is update from qGuiApp.
        _globalFM->setBaseFont(qGuiApp->font());
        connect(this, &DGuiApplicationHelper::fontChanged, _globalFM, &DFontManager::setBaseFont);
    }

    return _globalFM;
}

/*!
  \brief 获取颜色的明亮度，将其转换为主题类型的枚举值.

  转换的策略为：先将颜色转换为rgb格式，再根据 Y = 0.299R + 0.587G + 0.114B 的公式
  计算出颜色的亮度，亮度大于 191 时认为其为浅色，否则认为其为深色。
  \param color 需要转换为主题的类型的颜色
  \return 颜色类型的枚举值
 */
DGuiApplicationHelper::ColorType DGuiApplicationHelper::toColorType(const QColor &color)
{
    if (!color.isValid())
        return UnknownType;

    QColor rgb_color = color.toRgb();
    // 获取rgb颜色的亮度(转换为YUV格式)
    float luminance = 0.299 * rgb_color.redF() + 0.587 * rgb_color.greenF() + 0.114 * rgb_color.blueF();

    if (qRound(luminance * 255) > 191) {
        return LightType;
    }

    return DarkType;
}

/*!
  \brief 将调色板 \a palette 转换为主题类型的枚举.
  \overload

  使用 QPalette::background 获取颜色的明亮度，将其转换为主题类型的枚举值。
  返回调色板的颜色类型

  \param palette 调色板
  \return 颜色类型的枚举值
 */
DGuiApplicationHelper::ColorType DGuiApplicationHelper::toColorType(const QPalette &palette)
{
    return toColorType(palette.window().color());
}

/*!
  \brief 返回程序的主题类型.

  当themeType为UnknownType时, 将自动根据
  GuiApplication::palette的QPalette::background颜色计算主题
  类型, 否则与 paletteType 的值一致. 程序中应当使用此值作为
  暗色/亮色主题类型的判断.

  \return 主题的颜色类型.
  \sa toColorType
 */
DGuiApplicationHelper::ColorType DGuiApplicationHelper::themeType() const
{
    D_DC(DGuiApplicationHelper);

    d->initPaletteType();
    if (d->paletteType != UnknownType) {
        return d->paletteType;
    }

    return toColorType(qGuiApp->palette());
}

/*!
  \brief 返回当前已设置的调色板类型.

  如果未调用过 setPaletteType, 默认为 UnknownType.

  \return 返回当前已设置的调色板类型.
  \warning 与 themetype 不同，此值与程序当前的 QPalette 没有关系。
  \sa DGuiApplicationHelper::themeType
 */
DGuiApplicationHelper::ColorType DGuiApplicationHelper::paletteType() const
{
    D_DC(DGuiApplicationHelper);

    d->initPaletteType();
    return d->paletteType;
}

/*!
  \brief 设置DGuiApplicationHelper实例.

  \param key 实例关键字
  \param singleScope 实例使用范围
  \return 设置是否成功
  \note 此处所用到DGuiApplicationHelperPrivate::waitTime默认值为3000ms，可通过
  \note DGuiApplicationHelper::setSingleInstanceInterval设置
 */
bool DGuiApplicationHelper::setSingleInstance(const QString &key, DGuiApplicationHelper::SingleScope singleScope)
{
    bool new_server = !_d_singleServer.exists();

    if (_d_singleServer->isListening()) {
        _d_singleServer->close();
    }

    QString socket_key = "_d_dtk_single_instance_";

    switch (singleScope) {
    case GroupScope:
        _d_singleServer->setSocketOptions(QLocalServer::GroupAccessOption);
#ifdef Q_OS_LINUX
        socket_key += QString("%1_").arg(getgid());
#endif
        break;
    case WorldScope:
        _d_singleServer->setSocketOptions(QLocalServer::WorldAccessOption);
        break;
    default:
        _d_singleServer->setSocketOptions(QLocalServer::UserAccessOption);
#ifdef Q_OS_LINUX
        socket_key += QString("%1_").arg(getuid());
#endif
        break;
    }

    socket_key += key;
    QString lockfile = socket_key;
    if (!lockfile.startsWith(QLatin1Char('/'))) {
        lockfile = QDir::cleanPath(QDir::tempPath());
        lockfile += QLatin1Char('/') + socket_key;
    }
    lockfile += QStringLiteral(".lock");
    static QScopedPointer <QLockFile> lock(new QLockFile(lockfile));
    // 同一个进程多次调用本接口使用最后一次设置的 key
    // FIX dcc 使用不同的 key 两次调用 setSingleInstance 后无法启动的问题
    qint64 pid = -1;
    QString hostname, appname;
    if (lock->isLocked() && lock->getLockInfo(&pid, &hostname, &appname) && pid == getpid()) {
        qCWarning(dgAppHelper) << "call setSingleInstance again within the same process";
        lock->unlock();
        lock.reset(new QLockFile(lockfile));
    }

    if (!lock->tryLock()) {
        qCDebug(dgAppHelper) <<  "===> new client <===" << getpid();
        // 通知别的实例
        QLocalSocket socket;
        socket.connectToServer(socket_key);

        // 等待到有效数据时认为server实例有效
        if (socket.waitForConnected(DGuiApplicationHelperPrivate::waitTime) &&
                socket.waitForReadyRead(DGuiApplicationHelperPrivate::waitTime)) {
            // 读取数据
            qint8 version;
            qint64 pid;
            QStringList arguments;

            QDataStream ds(&socket);
            ds >> version >> pid >> arguments;
            qCInfo(dgAppHelper) << "Process is started: pid=" << pid << "arguments=" << arguments;

            // 把自己的信息告诉第一个实例
            ds << _d_singleServerVersion << qApp->applicationPid() << qApp->arguments();
            socket.flush();
        }

        return false;
    }

    if (!_d_singleServer->listen(socket_key)) {
        qCWarning(dgAppHelper) << "listen failed:" <<  _d_singleServer->errorString();
        return false;
    } else {
        qCDebug(dgAppHelper) << "===> listen <===" << _d_singleServer->serverName() << getpid();
    }

    if (new_server) {
        qCDebug(dgAppHelper) << "===> new server <===" << _d_singleServer->serverName() << getpid();
        QObject::connect(_d_singleServer, &QLocalServer::newConnection, qApp, [] {
            QLocalSocket *instance = _d_singleServer->nextPendingConnection();
            // 先发送数据告诉新的实例自己收到了它的请求
            QDataStream ds(instance);
            ds << _d_singleServerVersion // 协议版本
               << qApp->applicationPid() // 进程id
               << qApp->arguments(); // 启动时的参数

            QObject::connect(instance, &QLocalSocket::readyRead, qApp, [instance] {
                // 读取数据
                QDataStream ds(instance);

                qint8 version;
                qint64 pid;
                QStringList arguments;

                ds >> version >> pid >> arguments;
                instance->close();

                qCInfo(dgAppHelper) << "New instance: pid=" << pid << "arguments=" << arguments;

                // 通知新进程的信息
                if (_globalHelper.exists() && _globalHelper->helper())
                    Q_EMIT _globalHelper->helper()->newProcessInstance(pid, arguments);
            });

            instance->flush(); //发送数据给新的实例
        });
    }

    return true;
}

/*!
  \brief 设置从QLocalServer获取消息的等待时间.

  用于在重新创建DGuiApplicationHelper单例时，检测DGuiApplicationHelper单例是否存在且有响应
  \param interval 等待时间，如 \a interval 为 -1 则没有超时一直等待，默认和 QLocalSocket 一致 3000ms
  \note 需要在 DGuiApplicationHelper::setSingleInstance 之前调用否则无效。
 */
void DGuiApplicationHelper::setSingleInstanceInterval(int interval)
{
    Q_ASSERT_X(!_d_singleServer->isListening(), "DGuiApplicationHelper::setSingleInstanceInterval","Must call before setSingleInstance");
    DGuiApplicationHelperPrivate::waitTime = interval;
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
/*!
  \brief 设置从QLocalServer获取消息的等待时间.

  \param interval 等待时间，typo 请使用 DGuiApplicationHelper::setSingleInstanceInterval
 */
void DGuiApplicationHelper::setSingelInstanceInterval(int interval)
{
    DGuiApplicationHelperPrivate::waitTime = interval;
}
#endif

/*!
  \brief 获取帮助手册目录

  \return 帮助手册目录
 */
QStringList DGuiApplicationHelper::userManualPaths(const QString &appName)
{
    Q_ASSERT(!appName.isEmpty());

    // 获取主目录
    DCORE_USE_NAMESPACE
    QStringList manualPaths;
    const auto &pathlist = DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    // 获取所有已存在的帮助手册目录
    for (int i = 0; i < pathlist.size(); ++i) {
        QString strAssetPath = QStringList{pathlist[i], "deepin-manual/manual-assets"}.join(QDir::separator());
        for (const QString &type : QDir(strAssetPath).entryList(QDir::NoDotAndDotDot | QDir::Dirs)) {
            QString appDirPath = QStringList{strAssetPath, type, appName}.join(QDir::separator());
            if (QDir(appDirPath).exists())
                manualPaths.push_back(appDirPath);
        }
    }

    return manualPaths;
}

/*!
 * \brief Determine whether it's a user manual for this application.
 * \return
 */
bool DGuiApplicationHelper::hasUserManual() const
{
    return userManualPaths(qApp->applicationName()).size() > 0;
}

bool DGuiApplicationHelper::loadTranslator(const QString &fileName, const QList<QString> &translateDirs, const QList<QLocale> &localeFallback)
{
    DCORE_USE_NAMESPACE;

    QList<QString> dirs = translateDirs;
    const QList<DPathBuf> defaultDirPrefix {
        qApp->applicationDirPath(),
        QDir::currentPath()
    };
    for (auto item : defaultDirPrefix)
        dirs << item.join("translations").toString();

    QStringList missingQmfiles;
    for (const auto &locale : localeFallback) {
        QStringList translateFilenames {QString("%1_%2").arg(fileName).arg(locale.name())};
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        auto behavior = Qt::SkipEmptyParts;
#else
        auto behavior = QString::SkipEmptyParts;
#endif
        const QStringList parseLocalNameList = locale.name().split("_", behavior);
        if (parseLocalNameList.length() > 0)
            translateFilenames << QString("%1_%2").arg(fileName).arg(parseLocalNameList.at(0));

        for (const auto &translateFilename : translateFilenames) {
            for (const auto &dir : dirs) {
                DPathBuf path(dir);
                QString translatePath = (path / translateFilename).toString();
                if (QFile::exists(translatePath + ".qm")) {
                    qDebug() << "load translate" << translatePath;
                    auto translator = new QTranslator(qApp);
                    translator->load(translatePath);
                    qApp->installTranslator(translator);
                    qApp->setProperty("dapp_locale", locale.name());
                    return true;
                }
            }

            // fix english does not need to translation.
            if (locale.language() != QLocale::English)
                missingQmfiles << translateFilename + ".qm";
        }
    }

    if (missingQmfiles.size() > 0) {
        qWarning() << fileName << "can not find qm files" << missingQmfiles;
    }
    return false;
}

bool DGuiApplicationHelper::loadTranslator(const QList<QLocale> &localeFallback)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto qTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#else
    auto qTranslationsPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#endif
    loadTranslator("qt", {qTranslationsPath}, localeFallback);
    loadTranslator("qtbase", {qTranslationsPath}, localeFallback);

    DCORE_USE_NAMESPACE
    QList<QString> translateDirs;
    auto appName = qApp->applicationName();
    //("/home/user/.local/share", "/usr/local/share", "/usr/share")
    auto dataDirs = DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const auto &path : dataDirs) {
        DPathBuf DPathBuf(path);
        translateDirs << (DPathBuf / appName / "translations").toString();
    }

    // ${translateDir}/${appName}_${localeName}.qm
    return loadTranslator(appName, translateDirs, localeFallback);
}

DGuiApplicationHelper::SizeMode DGuiApplicationHelper::sizeMode() const
{
    D_DC(DGuiApplicationHelper);
    return d->fetchSizeMode();
}

void DGuiApplicationHelper::setSizeMode(const DGuiApplicationHelper::SizeMode mode)
{
    D_D(DGuiApplicationHelper);
    const auto old = d->fetchSizeMode();
    d->explicitSizeMode = mode;
    const auto current = d->fetchSizeMode();
    if (old != current)
        Q_EMIT sizeModeChanged(current);
}

void DGuiApplicationHelper::resetSizeMode()
{
    D_D(DGuiApplicationHelper);
    const auto old = d->fetchSizeMode();
    d->explicitSizeMode = InvalidSizeMode;
    const auto current = d->fetchSizeMode();
    if (current != old)
        Q_EMIT sizeModeChanged(current);
}

void DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::Attribute attribute, bool enable)
{
    if (attribute < Attribute::ReadOnlyLimit) {
        DGuiApplicationHelperPrivate::attributes.setFlag(attribute, enable);
    } else {
        qWarning() << "You are setting for the read-only option.";
        return;
    }
}

bool DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::Attribute attribute)
{
    switch (attribute) {
    case IsXWindowPlatform:
        return qGuiApp->platformName() == QByteArrayLiteral("xcb")
                || qGuiApp->platformName() == QByteArrayLiteral("dxcb");
    case IsDXcbPlatform:
        return DPlatformHandle::isDXcbPlatform();
    case IsTableEnvironment:
        return QGuiApplicationPrivate::instance()->platformIntegration()->services()->desktopEnvironment().toLower().endsWith("tablet");
    case IsDeepinPlatformTheme:
        if (!QGuiApplicationPrivate::platform_name) {
            return false;
        }
        return QString(typeid(*QGuiApplicationPrivate::platform_theme).name()).contains("QDeepinTheme");
    case IsDeepinEnvironment: {
        const auto &de = QGuiApplicationPrivate::instance()->platformIntegration()->services()->desktopEnvironment();
        return de.toLower().contains("deepin") || de == "DDE";
    }
    case IsSpecialEffectsEnvironment: {
        return qgetenv("DTK_DISABLED_SPECIAL_EFFECTS").toInt() != 1;
    }
    default:
        return DGuiApplicationHelperPrivate::attributes.testFlag(attribute);
    }
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
/*!
  \brief DGuiApplicationHelper::setThemeType.
  \deprecated 同 setPaletteType， 已废弃，请不要再使用。
  \param themeType 主题类型.
 */
void DGuiApplicationHelper::setThemeType(DGuiApplicationHelper::ColorType themeType)
{
    setPaletteType(themeType);
}
#endif

/*!
  \brief 设置程序所应用的调色板类型.

  将固定程序的调色板类型, 此行为可能导致 applicationPalette 变化, 前提是未使用
  setApplicationPalette 固定过程序的调色板, 此方法不影响程序的调色板跟随
  活动色改变, 可用于控制程序使用亮色还是暗色调色板.
  \note 主动设置调色板颜色类型的操作会导致程序不再使用 DPlatformTheme 中调色板相关的数据, 也
  包括窗口级别的 windowTheme 所对应的 DPlatformTheme, 届时设置 DPlatformTheme
  的 themeName 和所有与 palette 相关的属性都不再生效.
  \param paletteType 主题类型的枚举值
 */
void DGuiApplicationHelper::setPaletteType(DGuiApplicationHelper::ColorType paletteType)
{
    if (!qApp) {
        qWarning() << "Can't call `DGuiApplicationHelper::setPaletteType` before QCoreApplication constructed.";
        return;
    }

    D_D(DGuiApplicationHelper);

    d->initPaletteType();
    d->setPaletteType(paletteType, true);
    _d_dconfig->setValue(APP_THEME_TYPE, paletteType);
}

/*!
 * \brief Open manual for this application.
 */
void DGuiApplicationHelper::handleHelpAction()
{
    if (!hasUserManual()) {
        return;
    }
#ifdef Q_OS_LINUX
    QString appid = qApp->applicationName();

    // new interface use applicationName as id
    QDBusInterface manual("com.deepin.Manual.Open",
                          "/com/deepin/Manual/Open",
                          "com.deepin.Manual.Open");
    QDBusPendingCall call = manual.asyncCall("ShowManual", appid);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [appid](QDBusPendingCallWatcher *pWatcher) {
        QDBusPendingReply<void> reply = *pWatcher;
        if (reply.isError()) {
            // fallback to old interface
            qWarning() << reply.error() << "fallback to dman appid";
            QProcess::startDetached("dman", QStringList() << appid);
        }

        pWatcher->deleteLater();
    });

#else
    qWarning() << "not support dman now";
#endif
}

void DGuiApplicationHelper::openUrl(const QString &url)
{
#ifdef Q_OS_UNIX
    // workaround for pkexec apps
    bool ok = false;
    const int pkexecUid = qEnvironmentVariableIntValue("PKEXEC_UID", &ok);

    if (ok)
    {
        EnvReplaceGuard _env_guard(pkexecUid);
        Q_UNUSED(_env_guard);

        QDesktopServices::openUrl(url);
    }
    else
#endif
    {
        QDesktopServices::openUrl(url);
    }
}
DGUI_END_NAMESPACE

#include "moc_dguiapplicationhelper.cpp"

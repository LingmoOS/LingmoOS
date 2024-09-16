// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accessibilitycheckerex.h"
#include "appeventfilter.h"
#include "constants.h"
#include "greeterworker.h"
#include "loginframe.h"
#include "modules_loader.h"
#include "multiscreenmanager.h"
#include "propertygroup.h"
#include "sessionbasemodel.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>
#include <DPlatformTheme>

#include <QGuiApplication>
#include <QScreen>
#include <QSettings>

#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>
#include <cstdlib>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

//Load the System cursor --begin
static XcursorImages*
xcLoadImages(const char *image, int size)
{
    QSettings settings(DDESESSIONCC::DEFAULT_CURSOR_THEME, QSettings::IniFormat);
    //The default cursor theme path
    qDebug() << "Theme Path:" << DDESESSIONCC::DEFAULT_CURSOR_THEME;
    QString item = "Icon Theme";
    const char* defaultTheme = "";
    QVariant tmpCursorTheme = settings.value(item + "/Inherits");
    if (tmpCursorTheme.isNull()) {
        qDebug() << "Set a default one instead, if get the cursor theme failed!";
        defaultTheme = "Deepin";
    } else {
        defaultTheme = tmpCursorTheme.toString().toLocal8Bit().data();
    }

    qDebug() << "Get defaultTheme:" << tmpCursorTheme.isNull()
             << defaultTheme;
    return XcursorLibraryLoadImages(image, defaultTheme, size);
}

static unsigned long loadCursorHandle(Display *dpy, const char *name, int size)
{
    if (size == -1) {
        size = XcursorGetDefaultSize(dpy);
    }

    // Load the cursor images
    XcursorImages *images = nullptr;
    images = xcLoadImages(name, size);

    if (!images) {
        images = xcLoadImages(name, size);
        if (!images) {
            return 0;
        }
    }

    unsigned long handle = static_cast<unsigned long>(XcursorImagesLoadCursor(dpy,images));
    XcursorImagesDestroy(images);

    return handle;
}

static int set_rootwindow_cursor() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        qDebug() << "Open display failed";
        return -1;
    }

    const char *cursorPath = qApp->devicePixelRatio() > 1.7
            ? "/usr/share/icons/bloom/cursors/loginspinner@2x"
            : "/usr/share/icons/bloom/cursors/loginspinner";

    Cursor cursor = static_cast<Cursor>(XcursorFilenameLoadCursor(display, cursorPath));
    if (cursor == 0) {
        cursor = static_cast<Cursor>(loadCursorHandle(display, "watch", 24));
    }
    XDefineCursor(display, XDefaultRootWindow(display),cursor);

    // XFixesChangeCursorByName is the key to change the cursor
    // and the XFreeCursor and XCloseDisplay is also essential.

    XFixesChangeCursorByName(display, cursor, "watch");

    XFreeCursor(display, cursor);
    XCloseDisplay(display);

    return 0;
}
// Load system cursor --end

// 参照后端算法，保持一致
float toListedScaleFactor(float s)  {
    const float min = 1.0, max = 3.0, step = 0.25;
    if (s <= min) {
        return min;
    } else if (s >= max) {
        return max;
    }

    for (float i = min; i <= max; i += step) {
        if (i > s) {
            float ii = i - step;
            float d1 = s - ii;
            float d2 = i - s;
            if (d1 >= d2) {
                return i;
            } else {
                return ii;
            }
        }
    }
    return max;
}

static double get_scale_ratio() {
    Display *display = XOpenDisplay(nullptr);
    double scaleRatio = 0.0;

    if (!display) {
        return scaleRatio;
    }

    XRRScreenResources *resources = XRRGetScreenResourcesCurrent(display, DefaultRootWindow(display));

    if (!resources) {
        resources = XRRGetScreenResources(display, DefaultRootWindow(display));
        qWarning() << "get XRRGetScreenResourcesCurrent failed, use XRRGetScreenResources.";
    }

    if (resources) {
        for (int i = 0; i < resources->noutput; i++) {
            XRROutputInfo* outputInfo = XRRGetOutputInfo(display, resources, resources->outputs[i]);
            if (outputInfo->crtc == 0 || outputInfo->mm_width == 0) continue;

            XRRCrtcInfo *crtInfo = XRRGetCrtcInfo(display, resources, outputInfo->crtc);
            if (crtInfo == nullptr) continue;
            // 参照后端的算法，与后端保持一致
            float lenPx = hypot(static_cast<double>(crtInfo->width), static_cast<double>(crtInfo->height));
            float lenMm = hypot(static_cast<double>(outputInfo->mm_width), static_cast<double>(outputInfo->mm_height));
            float lenPxStd = hypot(1920, 1080);
            float lenMmStd = hypot(477, 268);
            float fix = (lenMm - lenMmStd) * (lenPx / lenPxStd) * 0.00158;
            float scale = (lenPx / lenMm) / (lenPxStd / lenMmStd) + fix;
            scaleRatio = toListedScaleFactor(scale);
        }
    }
    else {
        qWarning() << "get scale radio failed, please check X11 Extension.";
    }

    return scaleRatio;
}

static void set_auto_QT_SCALE_FACTOR() {
    const double ratio = get_scale_ratio();
    if (ratio > 0.0) {
        setenv("QT_SCALE_FACTOR", QByteArray::number(ratio).constData(), 1);
    }

    if (!qEnvironmentVariableIsSet("QT_SCALE_FACTOR")) {
        setenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1", 1);
    }
}

// 初次启动时，设置一下鼠标的默认位置
void set_pointer()
{
    auto set_position = [ = ] (QPoint p) {
        Display *dpy;
        dpy = XOpenDisplay(nullptr);

        if (!dpy) return;

        Window root_window;
        root_window = XRootWindow(dpy, 0);
        XSelectInput(dpy, root_window, KeyReleaseMask);
        XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, p.x(), p.y());
        XFlush(dpy);
    };

    if (!qApp->primaryScreen()) {
        QObject::connect(qApp, &QGuiApplication::primaryScreenChanged, [ = ] {
            static bool first = true;
            if (first) {
                set_position(qApp->primaryScreen()->geometry().center());
                first = false;
            }
        });
    } else {
        set_position(qApp->primaryScreen()->geometry().center());
    }
}

int main(int argc, char* argv[])
{
    // 正确加载dxcb插件
    //for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")){
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    setenv("DSG_APP_ID", "org.deepin.dde.lightdm-deepin-greeter", 1);

    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);
    // 设置缩放，文件存在的情况下，由后端去设置，否则前端自行设置
    if (!QFile::exists("/etc/lightdm/deepin/xsettingsd.conf")) {
        set_auto_QT_SCALE_FACTOR();
    }

    DApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName("org.deepin.dde.lightdm-deepin-greeter");
    qApp->setApplicationVersion("2015.1.0");
    qApp->setAttribute(Qt::AA_ForceRasterWidgets);

    DLogManager::registerConsoleAppender();
    DLogManager::registerJournalAppender();

    set_pointer();

    //注册全局事件过滤器
    AppEventFilter appEventFilter;
    a.installEventFilter(&appEventFilter);
    setAppType(APP_TYPE_LOGIN);

    DPalette pa = DGuiApplicationHelper::instance()->standardPalette(DGuiApplicationHelper::LightType);
    pa.setColor(QPalette::Normal, DPalette::WindowText, QColor("#FFFFFF"));
    pa.setColor(QPalette::Normal, DPalette::Text, QColor("#FFFFFF"));
    pa.setColor(QPalette::Normal, DPalette::AlternateBase, QColor(0, 0, 0, 76));
    pa.setColor(QPalette::Normal, DPalette::Button, QColor(255, 255, 255, 76));
    pa.setColor(QPalette::Normal, DPalette::Light, QColor(255, 255, 255, 76));
    pa.setColor(QPalette::Normal, DPalette::Dark, QColor(255, 255, 255, 76));
    pa.setColor(QPalette::Normal, DPalette::ButtonText, QColor("#FFFFFF"));
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::WindowText, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::Text, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::AlternateBase, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::Button, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::Light, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::Dark, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::ButtonText, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::instance()->setApplicationPalette(pa);

    dss::module::ModulesLoader *modulesLoader = &dss::module::ModulesLoader::instance();

    QCommandLineParser cmdParser;
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();

    QCommandLineOption modulePath("p", "Paths to load modules in debug mode, separated by semicolon.", "paths", QString());
    cmdParser.addOption(modulePath);

    cmdParser.process(a);

    if (cmdParser.isSet(modulePath)) {
        QString modulePathValue = cmdParser.value(modulePath);
        QStringList paths = modulePathValue.split(":");
        modulesLoader->setModulePaths(paths);
    }

    modulesLoader->start(QThread::LowestPriority);

    const QString serviceName = "org.deepin.dde.Accounts1";
    QDBusConnectionInterface *interface = QDBusConnection::systemBus().interface();
    if (!interface->isServiceRegistered(serviceName)) {
        qWarning() << "accounts service is not registered wait...";

        QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(serviceName, QDBusConnection::systemBus());
        QObject::connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, [ = ] (const QString &service) {
            if (service == serviceName) {
                qCritical() << "ERROR, accounts service unregistered, what should I do?";
            }
        });

#ifdef ENABLE_WAITING_ACCOUNTS_SERVICE
        qDebug() << "waiting for deepin accounts service";
        QEventLoop eventLoop;
        QObject::connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, &eventLoop, &QEventLoop::quit);
#ifdef  QT_DEBUG
        QTimer::singleShot(10000, &eventLoop, &QEventLoop::quit);
#endif
        eventLoop.exec();
        qDebug() << "service registered!";
#endif
    }

    SessionBaseModel *model = new SessionBaseModel();
    model->setAppType(Login);
    GreeterWorker *worker = new GreeterWorker(model);
    QObject::connect(&appEventFilter, &AppEventFilter::userIsActive, worker, &GreeterWorker::restartResetSessionTimer);

    /* load translation files */
    loadTranslation(model->currentUser()->locale());

    // 设置系统登录成功的加载光标
    QObject::connect(model, &SessionBaseModel::authFinished, model, [ = ](bool is_success) {
        if (is_success)
            set_rootwindow_cursor();
    });

    // 保证多个屏幕的情况下，始终只有一个屏幕显示
    PropertyGroup *property_group = new PropertyGroup(worker);
    property_group->addProperty("contentVisible");

    auto createFrame = [&](QScreen *screen, int count) -> QWidget * {
        LoginFrame *loginFrame = new LoginFrame(model);
        loginFrame->setScreen(screen, count <= 0);
        property_group->addObject(loginFrame);
        QObject::connect(loginFrame, &LoginFrame::requestSwitchToUser, worker, &GreeterWorker::switchToUser);
        QObject::connect(loginFrame, &LoginFrame::requestSetKeyboardLayout, worker, &GreeterWorker::setKeyboardLayout);
        QObject::connect(loginFrame, &LoginFrame::requestCheckAccount, worker, &GreeterWorker::checkAccount);
        QObject::connect(loginFrame, &LoginFrame::requestStartAuthentication, worker, &GreeterWorker::startAuthentication);
        QObject::connect(loginFrame, &LoginFrame::sendTokenToAuth, worker, &GreeterWorker::sendTokenToAuth);
        QObject::connect(loginFrame, &LoginFrame::requestEndAuthentication, worker, &GreeterWorker::endAuthentication);
        QObject::connect(loginFrame, &LoginFrame::authFinished, worker, &GreeterWorker::onAuthFinished);
        QObject::connect(worker, &GreeterWorker::requestUpdateBackground, loginFrame, &LoginFrame::updateBackground);
        loginFrame->show();
        return loginFrame;
    };

    MultiScreenManager multi_screen_manager;
    multi_screen_manager.register_for_mutil_screen(createFrame);
    QObject::connect(model, &SessionBaseModel::visibleChanged, &multi_screen_manager, &MultiScreenManager::startRaiseContentFrame);

#if defined(DSS_CHECK_ACCESSIBILITY) && defined(QT_DEBUG)
    AccessibilityCheckerEx checker;
    checker.addIgnoreClasses(QStringList()
                             << "Dtk::Widget::DBlurEffectWidget");
    checker.setOutputFormat(DAccessibilityChecker::FullFormat);
    checker.start();
#endif

    return a.exec();
}

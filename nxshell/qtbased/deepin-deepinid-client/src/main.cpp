// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QIcon>
#include <QDBusError>
#include <QDBusConnection>
#include <QSurfaceFormat>
#include <QProcess>

#include <DApplication>
#include <DLog>
#include <DPlatformWindowHandle>
#include <DStandardPaths>
#include <DGuiApplicationHelper>

#include "ui/login_window.h"

namespace Const
{

const char DBusService[] = "com.deepin.deepinid.Client";

const char DBusPath[] = "/com/deepin/deepinid/Client";

const char WAYLAND[] = "wayland";
} // namespace Const

int main(int argc, char **argv)
{
#ifdef __sw_64__
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox");
#endif

    QString sessionType = qEnvironmentVariable("XDG_SESSION_TYPE");
    if(sessionType == Const::WAYLAND) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
#ifndef __x86_64__
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        QSurfaceFormat::setDefaultFormat(format);
#endif
    }

    //Disable function: Qt::AA_ForceRasterWidgets, solve the display problem of domestic platform (loongson mips)
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu");
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-web-security");
    qputenv("DXCB_FAKE_PLATFORM_NAME_XCB", "true");

    // 系统使用自动代理时，程序会闪退，不知道原因暂时规避
    const QString auto_proxy = qgetenv("auto_proxy");
    if (!auto_proxy.isEmpty()) {
        qputenv("auto_proxy", "");
    }

    //龙芯机器配置,使得DApplication能正确加载QTWEBENGINE
    qputenv("DTK_FORCE_RASTER_WIDGETS", "FALSE");

    qputenv("_d_disableDBusFileDialog", "true");
    setenv("PULSE_PROP_media.role", "video", 1);

    QString glstring = QString::fromLocal8Bit(qgetenv("DEBUG_OPENGL"));
    if(!glstring.isEmpty())
    {
        QSurfaceFormat format;
        int gltype = glstring.toInt();
        qDebug() << "set gltype:" << gltype;
        if(gltype == 1)
        {
            format.setRenderableType(QSurfaceFormat::DefaultRenderableType);
        }
        else if (gltype == 2)
        {
            format.setRenderableType(QSurfaceFormat::OpenGL);
        }
        else if (gltype == 3)
        {
            format.setRenderableType(QSurfaceFormat::OpenGLES);
        }

        qDebug() << "set surface format:" << format.renderableType();
        QSurfaceFormat::setDefaultFormat(format);
    }

    //qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--single-process");

    Dtk::Widget::DApplication app(argc, argv);
    app.setAttribute(Qt::AA_ForceRasterWidgets, false);
    app.setOrganizationName("deepin");
    app.setApplicationName(QObject::tr("deepin ID"));
    app.setApplicationDisplayName(QObject::tr("deepin ID"));
    app.setProductIcon(QIcon::fromTheme("deepin-id"));
    app.setWindowIcon(QIcon::fromTheme("deepin-id"));
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerJournalAppender();

#ifdef CVERSION
    QString verstr(CVERSION);
    if (verstr.isEmpty() || verstr.contains("1.0.0"))
        verstr = "5.3.1.1";
    app.setApplicationVersion(verstr);
#else
    app.setApplicationVersion("5.3.1.2");
#endif

    if (!DGuiApplicationHelper::setSingleInstance("com.deepin.deepinid.Client")) {
        qWarning() << "another client is running";
        return 0;
    }

    QCommandLineParser parser;
    QCommandLineOption bootstrap({"b", "bootstrap"}, "start up url", "url", "");
    QCommandLineOption daemon({"d", "daemon"}, "run in background");
    QCommandLineOption verbose("verbose", "debug switch");
    parser.addOption(bootstrap);
    parser.addOption(daemon);
    parser.addOption(verbose);
    parser.addHelpOption();

    parser.process(app);

    app.loadTranslator();

    ddc::LoginWindow lw;
    lw.setWindowIcon(QIcon::fromTheme("deepin-id"));

    auto sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService(Const::DBusService)) {
        qDebug() << "register service failed" << sessionBus.lastError();
        return -1;
    }
    if (!sessionBus.registerObject(Const::DBusPath,
                                   &lw,
                                   QDBusConnection::ExportScriptableSlots)) {
        qDebug() << "register object failed" << sessionBus.lastError();
        return -2;
    }

    Dtk::Widget::DApplication::setQuitOnLastWindowClosed(true);

    if (parser.isSet(bootstrap)) {
        lw.setURL(parser.value(bootstrap));
        lw.load();
        lw.show();
    }

    QLoggingCategory::setFilterRules(QLatin1String("*.*.debug=false"));

    if (parser.isSet(verbose)) {
        QLoggingCategory::setFilterRules(QLatin1String("ui.sync_client.debug=true"));
    }

    return Dtk::Widget::DApplication::exec();
}

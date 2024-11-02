/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "explor-desktop-application.h"

#include "fm-dbus-service.h"
#include "desktop-menu-plugin-manager.h"

#include "volume-manager.h"

#include "desktop-icon-view.h"

#include "global-settings.h"
#include "plasma-shell-manager.h"
#include "desktop-menu.h"
#include "global-settings.h"
#include "file-enumerator.h"
#include "desktop-background-manager.h"
#include "desktopbackgroundwindow.h"
#include "desktop-item-model.h"
#include "windowmanager/windowmanager.h"

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTimer>

#include <QProcess>
#include <QFile>
#include <QLabel>
#include <QtConcurrent>

#include <QTranslator>
#include <QLocale>

#include <QSystemTrayIcon>
#include <QGSettings>

#include <gio/gio.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDesktopServices>

#include <QScreen>
#include <QTimeLine>

#include <QSessionManager>
#include <KWindowSystem>
#include <QX11Info>
#include <X11/Xlib.h>

#include <QMessageBox>

#define LINGMO_USER_GUIDE_PATH "/"
#define LINGMO_USER_GUIDE_SERVICE QString("com.lingmoUserGuide.hotel_%1").arg(getuid())
#define LINGMO_USER_GUIDE_INTERFACE "com.guide.hotel"

#define DESKTOP_MEDIA_HANDLE "org.gnome.desktop.media-handling"

static bool has_desktop = false;
static bool has_daemon = false;
static bool has_background = false;
static QRect max_size = QRect(0, 0, 0, 0);
static Peony::DesktopItemModel *desktop_model = nullptr;
static int desktop_window_id = 0;
static bool g_emitFinish = false;

//record of desktop start time
qint64 PeonyDesktopApplication::explor_desktop_start_time = 0;

void guessContentTypeCallback(GObject* object,GAsyncResult *res,gpointer data);
static void volume_mount_cb (GObject* source, GAsyncResult* res, gpointer udata);

static void mount_added_cb (GVolumeMonitor *volume_monitor,
                            GMount         *mount,
                            gpointer        user_data);

static void mount_changed_cb (GVolumeMonitor *volume_monitor,
                              GMount         *mount,
                              gpointer        user_data);

void trySetDefaultFolderUrlHandler() {
    //NOTE:
    //There is a bug in qt concurrent. If we use QtConcurrent::run()
    //to start a async function in the init stage, the qapplication will
    //not quit when we call "explor-qt-desktop -q" in command line.
    //
    //To solve this problem, the simplest way is delaying a while to execute the
    //async lambda function.
    QTimer::singleShot(1000, []() {
        QtConcurrent::run([=]() {
            GList *apps = g_app_info_get_all_for_type("inode/directory");
            bool hasPeonyQtAppInfo = false;
            GList *l = apps;
            while (l) {
                GAppInfo *info = static_cast<GAppInfo*>(l->data);
                QString cmd = g_app_info_get_executable(info);
                if (cmd.contains("explor")) {
                    hasPeonyQtAppInfo = true;
                    g_app_info_set_as_default_for_type(info, "inode/directory", nullptr);
                    break;
                }
                l = l->next;
            }
            if (apps) {
                g_list_free_full(apps, g_object_unref);
            }

            if (!hasPeonyQtAppInfo) {
                GAppInfo *explor_qt = g_app_info_create_from_commandline("explor",
                                     nullptr,
                                     G_APP_INFO_CREATE_SUPPORTS_URIS,
                                     nullptr);
                g_app_info_set_as_default_for_type(explor_qt, "inode/directory", nullptr);
                g_object_unref(explor_qt);
            }
            return;
        });
    });
}

//task#147972, story#21858, GreatWall svn and SE9215 controler, all file provide trash option
void greatWallDeviceInit()
{
    QtConcurrent::run([=]() {
        QProcess process;
        //process 中管道命令不生效，需要自己对读取的字符串进行处理
        //process.start("lspci | grep -c 88SE9215");
        process.start("lspci");
        process.waitForFinished();
        QString result = process.readAllStandardOutput();
        QStringList retList = result.split("\n");
        qDebug() << "greatWallDeviceInit lspci:"<<result;
        bool isTargetController = false;
        for(QString match : retList){
            if (match.indexOf("88SE9215") >=0){
                qDebug() << "match info:"<<match.indexOf("88SE9215")<<match;
                isTargetController = true;
                break;
            }
        }

        //not SE9215 controler
        if (! isTargetController)
            return;

        QProcess process1;
        process1.start("cat /sys/class/dmi/id/modalias");
        process1.waitForFinished();
        QString ret = process1.readAllStandardOutput();
        QStringList infoList = ret.split(":");
        for(QString info:infoList){
            if (info.startsWith("cvn", Qt::CaseInsensitive)){
                QString manufacturer = info.replace("cvn", "", Qt::CaseInsensitive);
                qDebug() << "greatWallDeviceInit manufacturer:"<<manufacturer;
                if (manufacturer.toUpper() == "GREATWALL")
                {
                    //setGsetting value
                    GlobalSettings::getInstance()->setValue(TRASH_MOBILE_FILES, true);
                }
                break;
            }
        }
    });
}

QRect caculateVirtualDesktopGeometry() {
    QRegion screensRegion;
    for (auto screen : qApp->screens()) {
        screensRegion += screen->geometry();
    }

    auto rect = screensRegion.boundingRect();
    if (rect.size().width()> rect.size().height())
    {
        rect = QRect(0, 0, rect.size().width(), rect.size().width());
    }
    else
    {
        rect = QRect(0, 0, rect.size().height(), rect.size().height());
    }
    if (rect.size().width()> max_size.size().width())
    {
        max_size = rect;
    }
    return max_size;
}

PeonyDesktopApplication::PeonyDesktopApplication(int &argc, char *argv[], const QString &applicationName) : QtSingleApplication (applicationName, argc, argv)
{
    // fix #172774
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths()<<"/usr/share/pixmaps");

    qDebug()<<"explor desktop constructor start";
    setApplicationVersion(QString("v%1").arg(VERSION));
    //setApplicationDisplayName(tr("Peony-Qt Desktop"));

    QTranslator *t = new QTranslator(this);
    t->load("/usr/share/libexplor-qt/libexplor-qt_"+QLocale::system().name());
    QApplication::installTranslator(t);
    QTranslator *t2 = new QTranslator(this);
    t2->load("/usr/share/explor-qt-desktop/explor-qt-desktop_"+QLocale::system().name());
    QApplication::installTranslator(t2);
    QTranslator *t3 = new QTranslator(this);
    t3->load("/usr/share/qt5/translations/qt_"+QLocale::system().name());
    QApplication::installTranslator(t3);
    setApplicationName(tr("explor-qt-desktop"));
    setWindowIcon(QIcon::fromTheme("system-file-manager"));

    //added for session call interactive
    QGuiApplication::setFallbackSessionManagementEnabled(true);
    QGuiApplication::setQuitOnLastWindowClosed(false);
    // fix #145378, logout background displayment issue.
    QObject::connect(this, &QGuiApplication::saveStateRequest, this, [=](QSessionManager &manager){
        if (KWindowSystem::isPlatformX11()) {
            if (manager.allowsErrorInteraction()) {
                qInfo()<<"session about to logout, clear the root window background";
//                XSetWindowBackground(QX11Info::display(), QX11Info::appRootWindow(), 0);
//                XSync(QX11Info::display(), false);
                manager.release();
            } else {
                qInfo()<<"session not support interaction or not x11 platform";
//                XSetWindowBackground(QX11Info::display(), QX11Info::appRootWindow(), 0);
//                XSync(QX11Info::display(), false);
            }
        }
    });

    // global settings
    if (QGSettings::isSchemaInstalled (FONT_SETTINGS)) {
        //font monitor
        QGSettings *fontSetting = new QGSettings (FONT_SETTINGS, QByteArray(), this);
        connect (fontSetting, &QGSettings::changed, this, [=] (const QString &key) {
            if (key == "systemFont" || key == "systemFontSize") {
                QFont font = this->font();
                for(auto widget : qApp->allWidgets()) {
                    widget->setFont(font);
                }
            }
        });
    }

    if (!this->isRunning()) {
        qDebug()<<"isPrimary screen";
        connect(this, &QtSingleApplication::messageReceived, [=](QString msg) {
            this->parseCmd(msg, true);
        });
        QFile file(":/desktop-icon-view.qss");
        file.open(QFile::ReadOnly);
        setStyleSheet(QString::fromLatin1(file.readAll()));
        file.close();

        //add 5 seconds delay to load plugins
        //try to fix first time enter desktop right menu not show open terminal issue
        //fix need wait 5 seconds open terminal issue, link to bug#75654
        //目前新的加载流程不需要等待那么久，加载出桌面时，应用都注册好了
        QTimer::singleShot(1000, [=]() {
            Peony::DesktopMenuPluginManager::getInstance();
        });

        /*
        QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
        auto volumeManager = Peony::VolumeManager::getInstance();
        connect(volumeManager, &Peony::VolumeManager::driveConnected, this, [=](const std::shared_ptr<Peony::Drive> &drive){
            trayIcon->show();
            trayIcon->showMessage(tr("Drive"), tr("%1 connected").arg(drive->name()));
        });
        connect(volumeManager, &Peony::VolumeManager::driveDisconnected, this, [=](const std::shared_ptr<Peony::Drive> &drive){
            trayIcon->show();
            trayIcon->showMessage(tr("Drive"), tr("%1 disconnected").arg(drive->name()));
        });

        connect(trayIcon, &QSystemTrayIcon::messageClicked, trayIcon, &QSystemTrayIcon::hide);
        */

        // auto mount local driver
        qDebug()<<"auto mount local volumes";
        GVolumeMonitor* vm = g_volume_monitor_get ();
        if (vm) {
            GList* drives = g_volume_monitor_get_connected_drives(vm);
            if (drives) {
                for (GList* i = drives; nullptr != i; i = i->next) {
                    GDrive * d = static_cast<GDrive*>(i->data);
                    if (G_IS_DRIVE(d)) {
                        GList* volumes = g_drive_get_volumes(d);
                        if (volumes) {
                            for (GList* j = volumes; nullptr != j; j = j->next) {
                                GVolume* v = static_cast<GVolume*>(j->data);
                                if (G_IS_VOLUME(v)) {
                                    g_autofree char* uuid = g_volume_get_uuid(v);
                                    if (0 != g_strcmp0("2691-6AB8", uuid)) {
                                        g_volume_mount(v, G_MOUNT_MOUNT_NONE, nullptr, nullptr, volume_mount_cb, nullptr);
                                    }
                                    g_object_unref(v);
                                }
                            }
                            g_list_free(volumes);
                        }
                        g_object_unref(d);
                    }
                }
                g_list_free(drives);
            }
        }
        g_object_unref(vm);

        g_signal_connect (g_volume_monitor_get(), "mount-added", G_CALLBACK(mount_added_cb), nullptr);
        g_signal_connect (g_volume_monitor_get(), "mount-changed", G_CALLBACK(mount_added_cb), nullptr);

        // enumerat network:///
        QThread* t = QThread::create ([=] () {
            FileEnumerator e;
            e.setEnumerateDirectory ("network:///");
            e.enumerateSync();
            e.getChildrenUris ();
        });
        connect (t, &QThread::finished, t, &QObject::deleteLater);
        t->start ();

    }

//    connect(this, &SingleApplicatioan::layoutDirectionChanged, this, &PeonyDesktopApplication::layoutDirectionChangedProcess);
////    connect(this, &SingleApplication::primaryScreenChanged, this, &PeonyDesktopApplication::primaryScreenChangedProcess);
//    connect(this, &SingleApplication::screenAdded, this, &PeonyDesktopApplication::screenAddedProcess);
//    connect(this, &SingleApplication::screenRemoved, this, &PeonyDesktopApplication::screenRemovedProcess);

    //parse cmd
    qDebug()<<"parse cmd";
    auto message = this->arguments().join(' ').toUtf8();
    parseCmd(message, !isRunning());

    //check if is great wall device and init settings
    connect(this, &PeonyDesktopApplication::emitFinish, this , [=](){
        if (g_emitFinish)
            return;
        g_emitFinish = true;
        QDBusMessage message = QDBusMessage::createMethodCall("org.gnome.SessionManager",
                                                              "/org/gnome/SessionManager",
                                                              "org.gnome.SessionManager",
                                                              "startupfinished");
        QList<QVariant> args;
        args.append("explor-qt-desktop");
        args.append("startupfinished");
        message.setArguments(args);
        QDBusConnection::sessionBus().send(message);

        greatWallDeviceInit();
    });

    qDebug()<<"monitor volumes change";
    auto volumeManager = Peony::VolumeManager::getInstance();
    connect(volumeManager,&Peony::VolumeManager::mountAdded,this,[=](const std::shared_ptr<Peony::Mount> &mount){
        // auto open dir for inserted dvd.
        GMount* newMount = (GMount*) g_object_ref(mount->getGMount());
        //special Volumn of 839 M upgrade part can not mount
        if (mount->uuid() != "2691-6AB8")
            g_mount_guess_content_type(newMount, FALSE, NULL, guessContentTypeCallback, NULL);

        // mount
    });
    connect(volumeManager,&Peony::VolumeManager::volumeRemoved,this,&PeonyDesktopApplication::volumeRemovedProcess);
    // 获取max_size初始值
    //caculateVirtualDesktopGeometry();
    qDebug()<<"explor desktop application constructor end";
}

Peony::DesktopItemModel *PeonyDesktopApplication::getModel()
{
    if (!desktop_model)
        desktop_model =  new DesktopItemModel();
    return desktop_model;
}

Peony::DesktopIconView *PeonyDesktopApplication::getIconView(QPoint pos)
{
    //获取当前屏幕的view,如果是镜像直接返回主屏
    Peony::DesktopIconView *desktopIconView = getIconView(0);
    QRegion screenRegion(m_config->primaryOutput()->geometry());
    if (screenRegion.contains(pos)) {
        return desktopIconView;
    };

    for (auto window : m_bg_windows) {
        QRegion screenRegion(window->getLogicalGeometryFromScreen());
        if (screenRegion.contains(pos)) {
            desktopIconView = window->getIconView();
            break;
        };
    }
    return desktopIconView;
}

Peony::DesktopIconView *PeonyDesktopApplication::getIconView(int id)
{
    for (auto window : m_bg_windows) {
        if (id == window->id()) {
            return window->getIconView();
        }
    }
    return m_bg_windows[0]->getIconView();
}

DesktopBackgroundWindow *PeonyDesktopApplication::getWindow(int id)
{
    for (auto window : m_bg_windows) {
        if (id == window->id()) {
            return window;
        }
    }
    return m_bg_windows[0];
}

Peony::DesktopIconView *PeonyDesktopApplication::getIconView(const KScreen::OutputPtr &output)
{
    for (auto window : m_bg_windows) {
        if (output == window->screen()) {
            return window->getIconView();
        }
    }
    return m_bg_windows[0]->getIconView();
}

Peony::DesktopIconView *PeonyDesktopApplication::removeUri(const QString& uri)
{
    for (auto window : m_bg_windows) {
        auto view = window->getIconView();
        if (view->removeItemRect(uri) == 1) {
            return view;
        }
    }
    return m_bg_windows[0]->getIconView();
}

bool PeonyDesktopApplication::userGuideDaemonRunning()
{
    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.isConnected())
        return false;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", LINGMO_USER_GUIDE_SERVICE);

    return reply != "";
}

void PeonyDesktopApplication::showGuide(const QString &appName)
{
    if (!userGuideDaemonRunning()) {
        QUrl url = QUrl("help:ubuntu-lingmo-help", QUrl::TolerantMode);
        QDesktopServices::openUrl(url);
        return;
    }

    bool bRet  = false;

    QDBusMessage m = QDBusMessage::createMethodCall(LINGMO_USER_GUIDE_SERVICE,
                     LINGMO_USER_GUIDE_PATH,
                     LINGMO_USER_GUIDE_INTERFACE,
                     "showGuide");
    m << appName;

    QDBusMessage response = QDBusConnection::sessionBus().call(m);
    if (response.type()== QDBusMessage::ReplyMessage)
    {
        //bRet = response.arguments().at(0).toBool();
    }
    else {
        QUrl url = QUrl("help:ubuntu-lingmo-help", QUrl::TolerantMode);
        QDesktopServices::openUrl(url);
    }
}

void PeonyDesktopApplication::gotoSetBackground()
{
    QProcess p;
    p.setProgram("/usr/bin/lingmo-control-center");
    //old version use -b, new version use -m Wallpaper as para
    p.setArguments(QStringList()<<"-m" <<"Wallpaper");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/lingmo-control-center", QStringList()<<"-m" <<"Wallpaper");
#endif
    p.waitForFinished(-1);
}

void PeonyDesktopApplication::gotoSetResolution()
{
    QProcess p;
    p.setProgram("/usr/bin/lingmo-control-center");
    p.setArguments(QStringList()<<"-m" <<"display");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/lingmo-control-center", QStringList()<<"-m" <<"display");
#endif
    p.waitForFinished(-1);

}

void PeonyDesktopApplication::relocateIconView(const KScreen::OutputPtr &output)
{
    qInfo()<<"start relocate icon view";
    //task#74174 更新多屏显示,根据id过滤元素
    int id = -1;
    DesktopBackgroundWindow *primaryWindow = nullptr;
    for (auto window : m_bg_windows) {
        if (window->screen()->isPrimary()) {
            id = window->id();
            primaryWindow = window;
            qDebug() << "primary name:" << primaryWindow->screen().data()->name() << " primary id:" << primaryWindow->screen()->id() ;
            break;
        }
    }
    qDebug() << "[PeonyDesktopApplication::relocateIconView] id:" << id;
    if (0 < id) {
        for (auto window : m_bg_windows) {
            if (0 == window->id()) {
                primaryWindow->getIconView()->refreshResolutionChange();
                window->getIconView()->refreshResolutionChange();
                window->setId(id);
                primaryWindow->setId(0);
                window->getIconView()->clearItemRect();
                primaryWindow->getIconView()->clearItemRect();
            }
        }
    }

    for (auto window : m_bg_windows) {
        qDebug() << "screen name :" << window->screen()->name() << " id:" << window->id() ;
        Q_EMIT window->getIconView()->updateView();
        window->setCentralView();
        if (!window->screen()->isPrimary() && 1 != m_mode) {
            KWindowSystem::raiseWindow(window->winId());
            kdk::WindowManager::activateWindow(window->getWindowId());
        }
        if (0 < id) {
            window->getIconView()->resolutionChange();
        }
    }
    if(primaryWindow) {
        KWindowSystem::raiseWindow(primaryWindow->winId());
        kdk::WindowManager::activateWindow(primaryWindow->getWindowId());
    }
}

void PeonyDesktopApplication::parseCmd(QString msg, bool isPrimary)
{
    QCommandLineParser parser;

    QCommandLineOption quitOption(QStringList()<<"q"<<"quit", tr("Close the explor desktop window"));
    parser.addOption(quitOption);

    QCommandLineOption daemonOption(QStringList()<<"d"<<"daemon", tr("Take over the dbus service."));
    parser.addOption(daemonOption);

    QCommandLineOption desktopOption(QStringList()<<"w"<<"desktop-window", tr("Take over the desktop displaying"));
    parser.addOption(desktopOption);

    QCommandLineOption backgroundOption(QStringList()<<"b"<<"background", tr("Setup backgrounds"));
//    parser.addOption(backgroundOption);

    QCommandLineOption clearIconsOption(QStringList()<<"c"<<"clear-icons", tr("Clear standard icons"));
    parser.addOption(clearIconsOption);

    if (isPrimary) {
        if (m_first_parse) {
            auto helpOption = parser.addHelpOption();
            auto versionOption = parser.addVersionOption();
            m_first_parse = false;
        }

        const QStringList args = QString(msg).split(' ');

        parser.process(args);
        if (parser.isSet(quitOption)) {
            QTimer::singleShot(1000, this, [=]() {
                qApp->quit();
            });
            return;
        }

        if (parser.isSet(daemonOption)) {
            if (!has_daemon) {
                has_daemon = true;
                qDebug()<<"-d";

                trySetDefaultFolderUrlHandler();

                //FIXME: take over org.freedesktop.FileManager1
                Peony::FMDBusService *service = new Peony::FMDBusService(this);
                connect(service, &Peony::FMDBusService::showItemsRequest, [=](const QStringList &urisList) {
                    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                    p.setProgram("/usr/bin/explor");
                    p.setArguments(QStringList()<<"--show-items"<<urisList);
                    p.startDetached();
#else
                    p.startDetached("/usr/bin/explor", QStringList()<<"--show-items"<<urisList, nullptr);
#endif
                });
                connect(service, &Peony::FMDBusService::showFolderRequest, [=](const QStringList &urisList) {
                    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                    p.setProgram("/usr/bin/explor");
                    p.setArguments(QStringList()<<"--show-folders"<<urisList);
                    p.startDetached();
#else
                    p.startDetached("/usr/bin/explor", QStringList()<<"--show-folders"<<urisList, nullptr);
#endif
                });
                connect(service, &Peony::FMDBusService::showItemPropertiesRequest, [=](const QStringList &urisList) {
                    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                    p.setProgram("/usr/bin/explor");
                    p.setArguments(QStringList()<<"--show-properties"<<urisList);
                    p.startDetached();
#else
                    p.startDetached("/usr/bin/explor", QStringList()<<"--show-properties"<<urisList, nullptr);
#endif
                });
            }
            has_daemon = true;
        }

        if (parser.isSet(desktopOption)) {
            setupBgAndDesktop();
        }

        if (parser.isSet(backgroundOption)) {
            setupBgAndDesktop();
        }

        if (parser.isSet(clearIconsOption)) {
            if (has_desktop) {
                clearIcons(args);
            }
        }

        connect(this, &QApplication::paletteChanged, this, [=](const QPalette &pal) {
            for (auto w : allWidgets()) {
                w->setPalette(pal);
                w->update();
            }
        });
    }
    else {
        auto helpOption = parser.addHelpOption();
        auto versionOption = parser.addVersionOption();

        if (arguments().count() < 2) {
            parser.showHelp();
        }
        parser.process(arguments());

        sendMessage(msg);
    }
}

void PeonyDesktopApplication::clearIcons(const QStringList &args)
{
    //定制需求
    int index = -1;
    if (args.contains("-c")) {
        index = args.indexOf("-c");

    } else if (args.contains("--clear-icons")) {
        index = args.indexOf("--clear-icons");
    }

    index++;
    if ((index <= 0) || (index >= args.count())) {
        return;
    }

    const QString &operationStr = args.value(index);
    bool isSuccess = false;
    int operationNum = operationStr.toInt(&isSuccess);
    for (auto window : m_bg_windows) {
        if (!window->getIconView()) {
            return;
        }
    }
    if (isSuccess) {
        switch (operationNum) {
            case 1:
                GlobalSettings::getInstance()->setValue(DISPLAY_STANDARD_ICONS, false);
                break;
            default:
                GlobalSettings::getInstance()->setValue(DISPLAY_STANDARD_ICONS, true);
        }
    }
}

void PeonyDesktopApplication::addWindow(QScreen *screen, bool checkPrimay)
{

}

void PeonyDesktopApplication::layoutDirectionChangedProcess(Qt::LayoutDirection direction)
{
    //not regular operate, to complete later
    qDebug()<<"layoutDirectionChangedProcess"<<direction;
}

void PeonyDesktopApplication::primaryScreenChangedProcess(QScreen *screen)
{
}

void PeonyDesktopApplication::screenAddedProcess(QScreen *screen)
{
    addWindow(screen, false);
}

void PeonyDesktopApplication::screenRemovedProcess(QScreen *screen)
{

}

bool PeonyDesktopApplication::isPrimaryScreen(QScreen *screen)
{
    if (screen == this->primaryScreen() && screen)
        return true;

    return false;
}

void PeonyDesktopApplication::changeBgProcess(const QString& bgPath)
{

}

void PeonyDesktopApplication::checkWindowProcess()
{

}

void PeonyDesktopApplication::updateVirtualDesktopGeometryByWindows()
{

}

void PeonyDesktopApplication::addBgWindow(const KScreen::OutputPtr &output)
{
    if (output.isNull() || !output->isEnabled()) {
        return;
    }
    qDebug() << "output added:" << output.data()->name()<< output->id() << output->geometry();
    int desktopWindowId = getDesktopWindowId();
    auto window = new DesktopBackgroundWindow(output, desktopWindowId);
    window->getIconView()->refresh();
    m_bg_windows.append(window);
    desktop_window_id = m_bg_windows.count();
    qDebug()<<"[PeonyDesktopApplication::addBgWindow] screen name:"<<window->screen()->name()<<"  IP:"<<window->screen();
    window->show();

    connect(window->getIconView(), &DesktopIconView::resetGridSize, this, [=](const QSize &gridSize){
        for (auto bgWindow : m_bg_windows) {
            bgWindow->getIconView()->setGridSize(gridSize);
        }
    });

    //task#74174 更新图标大小
    connect(window, &DesktopBackgroundWindow::setDefaultZoomLevel, this, [=](DesktopIconView::ZoomLevel level){
        for (auto bgWindow : m_bg_windows) {
            if (bgWindow->getIconView()->zoomLevel() != level) {
                bgWindow->getIconView()->setDefaultZoomLevel(level);
                bgWindow->getIconView()->clearExtendItemPos(true);
            }
        }
    });
    //task#74174 更新排序方式
    connect(window, &DesktopBackgroundWindow::setSortType, this, [=](int sortType){
        for (auto bgWindow : m_bg_windows) {
            bgWindow->getIconView()->setSortType(sortType);
        }
    });
    //task#74174 恢复扩展屏
    m_mode = checkScreenMode(output->geometry());
    if (2 == m_mode) {
        multiscreenMode();
    }

    relocateIconView(output);
}

void PeonyDesktopApplication::outputAdded(const KScreen::OutputPtr &output)
{
    for (auto bgWindow : m_bg_windows) {
        if (output == bgWindow->screen()) {
            qWarning("DesktopBackgroundWindow already exists and cannot be added");
            return;
        }
    }
    if (output.isNull()) {
        return;
    }
    addBgWindow(output);
    connect(output.data(), &KScreen::Output::isEnabledChanged,
            this, [=](){
        if(output->isEnabled()) {
            QTimer::singleShot(1, output.data() , [=]() {
                addBgWindow(output);
            });
        } else {
            outputRemoved(output->id());
        }
    });
}

void PeonyDesktopApplication::setupDesktop()
{
    connect(kdk::WindowManager::self(),&kdk::WindowManager::windowAdded,this,[=](const kdk::WindowId& windowId){
        if((quint32)getpid() == kdk::WindowManager::getPid(windowId)) {
            for (auto window : m_bg_windows) {
               QString title = kdk::WindowManager::getWindowTitle(windowId);
                QString windowTitle = window->windowTitle();
                if (title == windowTitle) {
                    window->setWindowId(windowId);
                    if (window->screen()->isPrimary()) {
                        kdk::WindowManager::activateWindow(window->getWindowId());
                    }
                    break;
                }
            }
        }
    });

    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation();
    if (op->exec()) {
        setConfig(op);
    } else {
        for (auto screen : qApp->screens()) {
            KScreen::OutputPtr output(new KScreen::Output);
            output->setId(qApp->screens().indexOf(screen));
            output->setName(qApp->primaryScreen()->name());
            output->setPos(screen->geometry().topLeft() * screen->devicePixelRatio());
            output->setExplicitLogicalSize(screen->size() * devicePixelRatio());
            output->setScale(1.0);
            output->setSize(screen->size() * screen->devicePixelRatio());
            if (screen == qApp->primaryScreen()) {
                output->setPrimary(true);
            } else {
                output->setPrimary(false);
            }
            output->setConnected(true);
            output->setEnabled(true);
            addBgWindow(output);
        }
        QMessageBox::warning(0, tr("Failed to get screen config"), tr("Error message is: %1. Using fallback config to setup desktop."), op->errorString());
    }
//    connect(op, &KScreen::GetConfigOperation::finished, this, [this](KScreen::ConfigOperation *op) {
//        setConfig(op);
//    });
}

void PeonyDesktopApplication::setupBgAndDesktop()
{
    if (!has_background) {
        setupDesktop();
    }
    has_background = true;
}

int PeonyDesktopApplication::getDesktopWindowId()
{
    int desktopWindowId = 0;
    for (; desktopWindowId <= desktop_window_id; desktopWindowId++) {
        bool find = false;
        for (auto bgWindow : m_bg_windows) {
            if (bgWindow->id() == desktopWindowId) {
                find = true;
                break;
            }
        }
        if (!find) {
            break;
        }
    }
    return desktopWindowId;
}

void PeonyDesktopApplication::singleScreenMode()
{
    auto primayView = getIconView(0);
    qDebug() << "primay view item" << primayView->model()->rowCount() << "total item:" << getModel()->rowCount();
    if ( 2 == m_mode  && primayView->model()->rowCount() == getModel()->rowCount()) {
        primayView->clearExtendItemPos();
        return;
    }
    for (auto bgWindow : m_bg_windows) {
        auto view = bgWindow->getIconView();
        view->getAllRestoreInfo();
        view->clearItemRect();
        view->saveExtendItemInfo();
    }

    Q_EMIT primayView->updateView();
}

void PeonyDesktopApplication::multiscreenMode()
{
    for (auto bgWindow : m_bg_windows) {
        auto view = bgWindow->getIconView();
        view->getAllRestoreInfo();
        view->clearItemRect();
        view->resetExtendItemInfo();
    }
    for (auto bgWindow : m_bg_windows) {
        auto view = bgWindow->getIconView();
        Q_EMIT view->updateView();
    }
}

void PeonyDesktopApplication::clearViewCache()
{
    for (auto bgWindow : m_bg_windows) {
        auto view = bgWindow->getIconView();
        view->clearCache();
    }
}

void guessContentTypeCallback(GObject* object, GAsyncResult *res,gpointer data)
{
    char **guessType;
    GError *error;
    QString openFolderCmd;
    GFile* root;
    char *mountUri;
    bool openFolder;
    QProcess process;
    static QString lastMountUri;

    error = NULL;
    openFolder = true;
    root = g_mount_get_default_location(G_MOUNT(object));

    // fix #205338
    g_autoptr(GFileInfo) access_info = g_file_query_info(root, G_FILE_ATTRIBUTE_ACCESS_CAN_READ","G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE, G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    openFolder = g_file_info_get_attribute_boolean(access_info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ) && g_file_info_get_attribute_boolean(access_info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);

    mountUri = g_file_get_uri(root);

    openFolderCmd = "/usr/bin/explor " + QString(mountUri);
    guessType = g_mount_guess_content_type_finish(G_MOUNT(object),res,&error);

    if (error) {
        g_error_free(error);
        error = NULL;
    } else {
        GDrive *drive = g_mount_get_drive(G_MOUNT(object));
        char *unixDevice = NULL;
        if(drive){
            unixDevice = g_drive_get_identifier(drive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
            g_object_unref(drive);
        }

        if(guessType && g_strv_length(guessType) > 0){
            int n;
            for(n = 0; guessType[n]; ++n){
                if(g_content_type_is_a(guessType[n],"x-content/image-dcf"))
                    openFolder = false;
                if(g_content_type_is_a(guessType[n],"x-content/win32-software"))
                    openFolder = false;
                if(unixDevice && !strcmp(guessType[n],"x-content/bootable-media") && !strstr(unixDevice,"/dev/sr"))
                    openFolder = false;
                if(!strcmp(guessType[n],"x-content/blank-dvd") || !strcmp(guessType[n],"x-content/blank-cd"))
                    openFolder = false;
                if(strstr(guessType[n],"x-content/audio-")){
                    if(!lastMountUri.compare(mountUri)){
                        lastMountUri.clear();
                        break;
                    }
                    lastMountUri = mountUri;
                }

                QString uri = mountUri;
                if (uri.startsWith("gphoto") || uri.startsWith("mtp"))
                    break;

                if(openFolder)
                    process.startDetached(openFolderCmd);

                openFolder = true;
            }
            g_strfreev(guessType);
        }else{
            //Only DvD devices are allowed to open folder automatically.
            if(unixDevice && !strcmp(unixDevice,"/dev/sr") && QGSettings::isSchemaInstalled(DESKTOP_MEDIA_HANDLE)){
                QGSettings* autoMountSettings =  new QGSettings(DESKTOP_MEDIA_HANDLE);
                if(autoMountSettings->get("automount-open").toBool()){
                    process.startDetached(openFolderCmd);
                    delete autoMountSettings;
                }
            }

        }
        g_free(unixDevice);
    }

    g_free(mountUri);
    g_object_unref(root);
    g_object_unref(object);
}

void PeonyDesktopApplication::volumeRemovedProcess(const std::shared_ptr<Peony::Volume> &volume)
{
    GVolume *gvolume = NULL;
    GDrive *gdrive = NULL;
    char *gdriveName = NULL;

    if(volume)
        gvolume = volume->getGVolume();
    if(gvolume)
        gdrive = g_volume_get_drive(gvolume);

    //Do not stop the DVD/CD driver.
    if(gdrive){
        gdriveName = g_drive_get_name(gdrive);
        if(gdriveName){
            if(strstr(gdriveName,"DVD") || strstr(gdriveName,"CD")){
                g_free(gdriveName);
                g_object_unref(gdrive);
                return;
            }
            g_free(gdriveName);
        }
    }

    //if it is possible, we stop it's drive after eject successfully.
   // if(gdrive && g_drive_can_stop(gdrive))
   //     g_drive_stop(gdrive,G_MOUNT_UNMOUNT_NONE,NULL,NULL,NULL,NULL);
};

int PeonyDesktopApplication::checkScreenMode(const QRect &geometry)
{
    int mode = 1;
    if (m_bg_windows.count() == 1) {
        mode = 0;
        return mode;
    }

    for (auto window : m_bg_windows) {
        if (window->screen()->geometry() != geometry) {
            mode = 2;
            break;
        }
    }
    return mode;
}

Peony::DesktopIconView *PeonyDesktopApplication::getNotFullView()
{
    for (auto window : m_bg_windows) {
        Peony::DesktopIconView *view = window->getIconView();
        if (view && !view->isFull()) {
            return view;
        }
    }
    return nullptr;
}

void PeonyDesktopApplication::setConfig(KScreen::ConfigOperation *op)
{
    m_config = op->config();
    if (!m_config) {
        qCritical("[PeonyDesktopApplication::setConfig] config is null");
        return;
    }
    KScreen::ConfigMonitor::instance()->addConfig(m_config);

    for (const KScreen::OutputPtr &output : m_config->outputs()) {
        outputAdded(output);
    }
    connect(m_config.data(), &KScreen::Config::primaryOutputChanged,
            this, &PeonyDesktopApplication::relocateIconView);
    connect(m_config.data(), &KScreen::Config::outputAdded,
            this, &PeonyDesktopApplication::outputAdded);
    connect(m_config.data(), &KScreen::Config::outputRemoved,
            this, &PeonyDesktopApplication::outputRemoved);

}

void PeonyDesktopApplication::outputRemoved(int outputId)
{
    for (auto window : m_bg_windows) {
        if (outputId == window->screen()->id()) {
            if (m_bg_windows.count() - 1 != window->id() && m_bg_windows.count() > 1) {
                auto lastWindow = getWindow(m_bg_windows.count() - 1);
                lastWindow->setId(m_bg_windows.count() - 2);
                window->setId(m_bg_windows.count() - 1);
                lastWindow->getIconView()->updateView();
                window->getIconView()->updateView();
            }
            if (m_mode == 2) {
                if (m_bg_windows.count() > 2) {
                    //task#74174 销毁时保存扩展屏元素的坐标点
                    window->getIconView()->saveExtendItemInfo();
                    getIconView(0)->updateView();
                } else if (m_bg_windows.count() == 2) {
                    singleScreenMode();
                    m_mode = 0;
                }
            }
            window->invaidScreen();
            m_bg_windows.removeOne(window);
            window->deleteLater();
            return;
        }
    }
}

void PeonyDesktopApplication::changeMode(int mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        if (1 == mode) {
            singleScreenMode();
         } else if (2 == mode) {
            multiscreenMode();
        }
    }
}
static void volume_mount_cb (GObject* source, GAsyncResult* res, gpointer udata)
{
    g_volume_mount_finish(G_VOLUME (source), res, nullptr);

    Q_UNUSED(udata);
}

void mount_added_cb (GVolumeMonitor *volume_monitor,
                     GMount         *mount,
                     gpointer        user_data)
{
    Q_UNUSED (volume_monitor)
    Q_UNUSED (user_data)
    g_autoptr (GFile) file = g_mount_get_root(mount);
    g_file_query_info_async(file, "*", G_FILE_QUERY_INFO_NONE, 0, 0, 0, 0);
}

void mount_changed_cb (GVolumeMonitor *volume_monitor,
                       GMount         *mount,
                       gpointer        user_data)
{
    Q_UNUSED (volume_monitor)
    Q_UNUSED (user_data)
    g_autoptr (GFile) file = g_mount_get_root(mount);
    g_file_query_info_async(file, "*", G_FILE_QUERY_INFO_NONE, 0, 0, 0, 0);
}

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
 *
 */

#include "explorer-application.h"
#include "menu-plugin-iface.h"

#include "main-window-factory-plugin-manager.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-utils.h"

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QUrl>

#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QString>
#include <QMenu>
#include <QTimer>
#include <QVBoxLayout>

#include "preview-page-factory-manager.h"
#include "preview-page-plugin-iface.h"
#include "directory-view-factory-manager.h"
#include "directory-view-plugin-iface.h"
#include "directory-view-container.h"

#include "path-edit.h"
#include "location-bar.h"
#include <QStandardPaths>
#include <QStackedLayout>

#include "tool-bar.h"
#include <QMainWindow>

#include "tab-page.h"

#include "side-bar.h"

#include "navigation-tool-bar.h"

#include "navigation-bar.h"

#include "fm-window.h"
//#include "main-window.h"
#include "global-settings.h"

#include <QFile>

#include <QStyleFactory>

#include "search-vfs-register.h"

#include <QMessageBox>

#include "menu-plugin-manager.h"
#include "directory-view-menu.h"
#include "icon-view.h"

#include "side-bar-factory-manager.h"
#include "intel/tablet-side-bar-factory.h"

#include "plugin-manager.h"

#include "list-view.h"

#include "basic-properties-page.h"

#include "file-count-operation.h"
#include <QThreadPool>

#include "properties-window-factory-plugin-manager.h"
//#include "properties-window.h"

#include "complementary-style.h"

#include "volume-manager.h"

#include "file-enumerator.h"
#include "gerror-wrapper.h"

#include <QTranslator>
#include <QLocale>

#include <QStyleFactory>
#include <QDesktopServices>

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusConnectionInterface>

#include <KWindowSystem>

#define LINGMO_USER_GUIDE_PATH "/"
#define LINGMO_USER_GUIDE_SERVICE QString("com.lingmoUserGuide.hotel_%1").arg(getuid())
#define LINGMO_USER_GUIDE_INTERFACE "com.guide.hotel"

//record of explorer start time
qint64 PeonyApplication::explorer_start_time = 0;
static bool m_resident = false;

PeonyApplication::PeonyApplication(int &argc, char *argv[], const char *applicationName) : SingleApplication (argc, argv, applicationName, true)
{
    // fix #172774
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths()<<"/usr/share/pixmaps");

    bool isWayland = QString(qgetenv("XDG_SESSION_DESKTOP")).contains("lingmo-wayland");
    setProperty("isWayland", isWayland);

    setApplicationVersion(QString("v%1").arg(VERSION));
    //setApplicationDisplayName(tr("Peony-Qt"));

    QFile file(":/data/libexplorer-qt-styled.qss");
    file.open(QFile::ReadOnly);
    setStyleSheet(QString::fromLatin1(file.readAll()));
    //qDebug()<<file.readAll();
    file.close();

    QTranslator *t = new QTranslator(this);
    qDebug()<<"\n\n\n\n\n\n\ntranslate:"<<t->load("/usr/share/libexplorer-qt/libexplorer-qt_"+QLocale::system().name());
    QApplication::installTranslator(t);
    QTranslator *t2 = new QTranslator(this);
    t2->load("/usr/share/explorer-qt/explorer-qt_"+QLocale::system().name());
    QApplication::installTranslator(t2);
    QTranslator *t3 = new QTranslator(this);
    t3->load("/usr/share/qt5/translations/qt_"+QLocale::system().name());
    QApplication::installTranslator(t3);
    QTranslator *t4 = new QTranslator(this);
    t4->load("/usr/share/qt5/translations/qtbase_"+QLocale::system().name());
    QApplication::installTranslator(t4);
    //setStyle(Peony::ComplementaryStyle::getStyle());
    QTranslator *sdkTrans = new QTranslator(this);
    if (sdkTrans->load(":/translations/gui_" + QLocale::system().name() + ".qm")) {
        QApplication::installTranslator(sdkTrans);
    }

#ifdef LINGMO_UDF_BURN
    QTranslator *tUdfBrun = new QTranslator(this);
    auto udfBurnTranslationFilePath = QString("/usr/share/kyudfburn/translations/kyudfburn_%1.qm").arg(QLocale::system().name());
    bool ok = tUdfBrun->load(udfBurnTranslationFilePath);
    if (!ok) {
        qWarning()<<"can not load kyudfburn translation files, path is"<<udfBurnTranslationFilePath;
    } else {
        QApplication::installTranslator(tUdfBrun);
    }
#endif

    setApplicationName(tr("explorer-qt"));

    parser.addOption(quitOption);
    parser.addOption(showItemsOption);
    parser.addOption(showFoldersOption);
    parser.addOption(showPropertiesOption);

    parser.addPositionalArgument("files", tr("Files or directories to open"), tr("[FILE1, FILE2,...]"));

    //unmount all ftp node when close all window
    connect(qApp, &QApplication::lastWindowClosed, this, &PeonyApplication::unmountAllFtpLinks);

    if (this->isSecondary()) {
        parser.addHelpOption();
        parser.addVersionOption();
        if (this->arguments().count() == 2 && arguments().last() == ".") {
            QStringList args;
            auto dir = g_get_current_dir();
            args<<"explorer"<<dir;
            g_free(dir);
            auto message = getUriMessage(args).toUtf8();
            bool msgSent = sendMessage(message);
            if (msgSent)
                return;
        }
        parser.process(arguments());
        QStringList allArgs = arguments();
        auto message = getUriMessage(allArgs).toUtf8();
        bool msgSent = sendMessage(message);
        if (msgSent)
            return;
    }

    if (this->isPrimary()) {
        connect(this, &SingleApplication::receivedMessage, this, &PeonyApplication::parseCmd);
        Peony::SideBarFactoryManager::getInstance()->registerFactory(new Peony::Intel::TabletSideBarFactory);
    } else {
        qCritical()<<"secondary process send message to old primary process failed, try changed this process to primary one";
        this->startPrimary();
        connect(this, &SingleApplication::receivedMessage, this, &PeonyApplication::parseCmd);
    }

    //parse cmd
    parser.process(arguments());
    QStringList allArgs = arguments();
    auto message = getUriMessage(allArgs).toUtf8();
    parseCmd(this->instanceId(), message);

    auto testIcon = QIcon::fromTheme("folder");
    if (testIcon.isNull()) {
        QIcon::setThemeName("lingmo-icon-theme-default");
        if (QStyleFactory::keys().contains("gtk2")) {
            setStyle("gtk2");
        }
        QMessageBox::warning(nullptr, tr("Warning"), tr("Peony-Qt can not get the system's icon theme. "
                             "There are 2 reasons might lead to this problem:\n\n"
                             "1. Peony-Qt might be running as root, "
                             "that means you have the higher permission "
                             "and can do some things which normally forbidden. "
                             "But, you should learn that if you were in a "
                             "root, the virtual file system will lose some "
                             "featrue such as you can not use \"My Computer\", "
                             "the theme and icons might also went wrong. So, run "
                             "explorer-qt in a root is not recommended.\n\n"
                             "2. You are using a non-qt theme for your system but "
                             "you didn't install the platform theme plugin for qt's "
                             "applications. If you are using gtk-theme, try installing "
                             "the qt5-gtk2-platformtheme package to resolve this problem."));
    }
    //Peony::SearchVFSRegister::registSearchVFS();
    //QIcon::setThemeName("lingmo-icon-theme-one");
    //setAttribute(Qt::AA_UseHighDpiPixmaps);
    //setAttribute(Qt::AA_EnableHighDpiScaling);

    //setStyle(QStyleFactory::create("windows"));

    //check if first run
    //if not send message to server
    //else
    //  load plgin
    //  read from command line
    //  do with args
}

static void unmount_finished(GFile* file, GAsyncResult* result, gpointer udata)
{

    int flags = 0;
    GError *err = nullptr;

    if (g_file_unmount_mountable_with_operation_finish (file, result, &err) == TRUE){
        flags = 1;
        char *uri = g_file_get_uri(file);
        Peony::VolumeManager::getInstance()->fileUnmounted(uri);
        if (uri)
            g_free(uri);
    }

    if (! m_resident)
    {
        qApp->setQuitOnLastWindowClosed(true);
    }

    if (err){
        qCritical() << "main window unmount_finished error:"<<err->message<<flags;
        g_error_free(err);
    }

    //when has no new window, force quit explorer
    if (qApp->topLevelWindows().count() <= 0 && ! m_resident){
        qDebug() << "has no new window, exit";
        qApp->exit();
    }
}

void PeonyApplication::unmountAllFtpLinks()
{
    qDebug() << "lastWindowClosed unmountAllFtpLinks";
    auto allUris = Peony::FileUtils::getChildrenUris("computer:///");
    for(auto uri : allUris)
    {
        auto targetUri = Peony::FileUtils::getTargetUri(uri);
        qDebug() << "unmountAllFtpLinks targetUri:" <<targetUri;
        if (! targetUri.startsWith("smb://") &&
            ! targetUri.startsWith("sftp://") &&
            ! targetUri.startsWith("ftp://"))
            continue;

        m_resident = Peony::GlobalSettings::getInstance()->getValue(RESIDENT_IN_BACKEND).toBool();
        qApp->setQuitOnLastWindowClosed(false);
        GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
        g_file_unmount_mountable_with_operation(file,
                                                G_MOUNT_UNMOUNT_NONE,
                                                nullptr,
                                                nullptr,
                                                GAsyncReadyCallback(unmount_finished),
                                                this);
        g_object_unref(file);
    }
}

QString PeonyApplication::getUriMessage(QStringList& strList)
{
    QStringList args;

    for (auto uri = strList.constBegin(); uri != strList.constEnd(); ++uri) {
        if ("explorer" == (*uri) || (*uri).startsWith("-") || (*uri).startsWith("--") || (*uri).startsWith("%")
                || (*uri).startsWith("/usr/") || (*uri).startsWith("/bin/") || (*uri).startsWith("/sbin/")) {
            args << *uri;
        } else if ((*uri).startsWith("/")) {
            args << Peony::FileUtils::urlEncode("file://" + *uri);
        } else if ((*uri).startsWith("mtp://") || (*uri).startsWith("gphoto2://")) {
            args << *uri;
        } else if ((*uri).contains("://")) {
            args << Peony::FileUtils::urlEncode(*uri);
        } else {
            args << Peony::FileUtils::urlEncode(QString("file://%1/%2").arg(g_get_current_dir()).arg(*uri));
        }
    }

    return args.join(' ');
}

void PeonyApplication::parseCmd(quint32 id, QByteArray msg)
{
    QCommandLineParser parser;
    if (m_first_parse) {
        parser.addHelpOption();
        parser.addVersionOption();
        m_first_parse = false;
    }
    parser.addOption(quitOption);
    parser.addOption(showItemsOption);
    parser.addOption(showFoldersOption);
    parser.addOption(showPropertiesOption);

    //qDebug()<<"parse cmd:"<<"id:"<<id<<"msg:"<<msg;
    const QStringList args = QString(msg).split(' ');
    //qDebug()<<args;

    parser.process(args);
    if (parser.isSet(quitOption)) {
        QTimer::singleShot(1, [=]() {
            qApp->quit();
        });
        return;
    }

    MainWindowFactoryPluginManager::getInstance()->setVersion();

    //FIXME: should I load plugins async?
    Peony::PluginManager::init();

    if (!parser.optionNames().isEmpty()) {
        if (parser.isSet(showItemsOption)) {
            //FIXME: show item parent folder and set selection for item.
            QHash<QString, QStringList> itemHash;
            auto uris = Peony::FileUtils::toDisplayUris(parser.positionalArguments());
            if (uris.isEmpty()) {
                return;
            }
            for (auto uri : uris) {
                auto parentUri = Peony::FileUtils::getParentUri(uri);
                if (itemHash.value(parentUri).isEmpty()) {
                    QStringList l;
                    l<<uri;
                    itemHash.insert(parentUri, l);
                } else {
                    auto l = itemHash.value(parentUri);
                    l<<uri;
                    itemHash.remove(parentUri);
                    itemHash.insert(parentUri, l);
                }
            }
            auto parentUris = itemHash.keys();

            for (auto parentUri : parentUris) {
                QWidget *window = MainWindowFactoryPluginManager::getInstance()->create(parentUri,itemHash.value(parentUri));
//                Peony::MainWindowIface* mainWindowIface = dynamic_cast<Peony::MainWindowIface*>(window);
                //auto window = new MainWindow(parentUri);
                //Peony::FMWindow *window = new Peony::FMWindow(parentUri);
//                connect(mainWindowIface, &Peony::MainWindowIface::locationChangeEnd, [=]() {
//                    QTimer::singleShot(500, [=] {
//                        Peony::FMWindowIface* iface = dynamic_cast<Peony::FMWindowIface*>(window);
//                        iface->getCurrentPage()->getView()->setSelections(itemHash.value(parentUri));
//                        iface->getCurrentPage()->getView()->scrollToSelection(itemHash.value(parentUri).first());
//                    });
//                });
                window->show();
                KWindowSystem::raiseWindow(window->winId());
                if (KWindowSystem::activeWindow() != window->winId()) {
                    KWindowSystem::activateWindow(window->winId());
                }
            }
        }

        if (parser.isSet(showFoldersOption)) {
            QStringList uris = Peony::FileUtils::toDisplayUris(parser.positionalArguments());
            if (uris.isEmpty()) {
                return;
            }
            QWidget *window = MainWindowFactoryPluginManager::getInstance()->create(uris.first());
            //auto window = new MainWindow(uris.first());
            //Peony::FMWindow *window = new Peony::FMWindow(uris.first());
            uris.removeAt(0);
            if (!uris.isEmpty()) {
                Peony::FMWindowIface* iface = dynamic_cast<Peony::FMWindowIface*>(window);
                iface->addNewTabs(uris);
            }
            window->show();
            KWindowSystem::raiseWindow(window->winId());
            if (KWindowSystem::activeWindow() != window->winId()) {
                KWindowSystem::activateWindow(window->winId());
            }
        }
        if (parser.isSet(showPropertiesOption)) {
            QStringList uris = Peony::FileUtils::toDisplayUris(parser.positionalArguments());
            if (uris.isEmpty()) {
                return;
            }

            qApp->setProperty("showProperties", true);
            QMainWindow *window = Peony::PropertiesWindowFactoryPluginManager::getInstance()->create(uris);
            //Peony::PropertiesWindow *window = new Peony::PropertiesWindow(uris);
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->show();
            KWindowSystem::raiseWindow(window->winId());
            if (KWindowSystem::activeWindow() != window->winId()) {
                KWindowSystem::activateWindow(window->winId());
            }
        }
    } else {
        if (!parser.positionalArguments().isEmpty()) {
            auto arguments = parser.positionalArguments();
            arguments.removeOne("%U");
            arguments.removeOne("%U&");
            QStringList uris = Peony::FileUtils::toDisplayUris(arguments);
            if (!uris.isEmpty()) {
                QWidget *window = MainWindowFactoryPluginManager::getInstance()->create(uris.first());
                Peony::FMWindowIface* iface = dynamic_cast<Peony::FMWindowIface*>(window);
                //auto window = new MainWindow(uris.first());
                uris.removeAt(0);
                if (!uris.isEmpty()) {
                    iface->addNewTabs(uris);
                }
                window->setAttribute(Qt::WA_DeleteOnClose);
                window->show();
                KWindowSystem::raiseWindow(window->winId());
                if (KWindowSystem::activeWindow() != window->winId()) {
                    KWindowSystem::activateWindow(window->winId());
                }
            } else {
                QWidget *window = MainWindowFactoryPluginManager::getInstance()->create(QString());
              //  auto window = new MainWindow();
                window->setAttribute(Qt::WA_DeleteOnClose);
                window->show();
                KWindowSystem::raiseWindow(window->winId());
                if (KWindowSystem::activeWindow() != window->winId()) {
                    KWindowSystem::activateWindow(window->winId());
                }
            }
        } else {
            QWidget *window = MainWindowFactoryPluginManager::getInstance()->create(QString());
            //auto window = new MainWindow;
            //auto window = new Peony::FMWindow;
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->show();
            KWindowSystem::raiseWindow(window->winId());
            if (KWindowSystem::activeWindow() != window->winId()) {
                KWindowSystem::activateWindow(window->winId());
            }
        }
    }

    connect(this, &QApplication::paletteChanged, this, [=](const QPalette &pal) {
        for (auto w : allWidgets()) {
            w->setPalette(pal);
            w->update();
        }
    });
}

void PeonyApplication::about()
{
    QMessageBox *msgBox = new QMessageBox();
    msgBox->setWindowTitle(tr("Peony Qt"));
    msgBox->setText(tr("Author:\n"
                       "\tYue Lan <lanyue@kylinos.cn>\n"
                       "\tMeihong He <hemeihong@kylinos.cn>\n"
                       "\n"
                       "Copyright (C): 2020, KylinSoft Co., Ltd."));
    msgBox->setModal(false);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->show();
}

void PeonyApplication::help()
{
    showGuide();
}

bool PeonyApplication::userGuideDaemonRunning()
{
    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.isConnected())
        return false;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", LINGMO_USER_GUIDE_SERVICE);

    return reply != "";
}

void PeonyApplication::showGuide(const QString &appName)
{
    auto s = LINGMO_USER_GUIDE_SERVICE;
    qDebug()<<s;
    if (!userGuideDaemonRunning()) {
        QUrl url = QUrl("help:ubuntu-lingmo-help/files", QUrl::TolerantMode);
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
        QUrl url = QUrl("help:ubuntu-lingmo-help/files", QUrl::TolerantMode);
        QDesktopServices::openUrl(url);
    }
}

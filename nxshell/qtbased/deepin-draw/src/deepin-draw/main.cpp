// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "application.h"
//#include "frame/cviewmanagement.h"
//#include "frame/ccentralwidget.h"

#include <DGuiApplicationHelper>
#include <DApplicationSettings>

#include <QCommandLineOption>
#include <QObject>
#include <QTranslator>
#include <QMessageBox>
#include <QDebug>
#include <QDBusMetaType>
#include <fcntl.h>
#include "drawinterface.h"
#include "drawConfig.h"


#include <DLog>

QStringList getFilesFromQCommandLineParser(const QCommandLineParser &parser)
{
    QStringList files;
    QStringList pas = parser.positionalArguments();
    for (int  i = 0; i < pas.count(); i++) {
        QString file = pas.at(i);
        files.append(file);
    }
    return files;
}

bool checkOnly()
{
    //single
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-draw/").toStdString();
    QDir tdir(path.c_str());
    if (!tdir.exists()) {
        bool ret =  tdir.mkpath(path.c_str());
        qDebug() << ret ;
    }

    path += "single";
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    int flock = lockf(fd, F_TLOCK, 0);

    if (fd == -1) {
        perror("open lockfile/n");
        return false;
    }
    if (flock == -1) {
        perror("lock file error/n");
        return false;
    }
    return true;
}

bool isRunning(Application &a)
{
    //判断实例是否已经运行
    return (!a.dApplication()->setSingleInstance("deepinDraw") || !checkOnly());
}

int main(int argc, char *argv[])
{
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
#if defined(STATIC_LIB)
    DWIDGET_INIT_RESOURCE();
#endif

    if (Application::isWaylandPlatform()) {
        //添加kwayland-shell，修改了窗口顶部栏无右键菜单
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }

#ifdef LINK_DRAWBASELIB_STATIC
//    Q_INIT_RESOURCE(drawBaseRes);
//    //Q_INIT_RESOURCE(frameRes);
//    Q_INIT_RESOURCE(images);
//    Q_INIT_RESOURCE(cursorIcons);
//    Q_INIT_RESOURCE(widgetsRes);
#endif
    DApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    Application a(argc, argv);
    a.dApplication()->loadTranslator();

    QCommandLineOption openImageOption(QStringList() << "o" << "open",
                                       "Specify a path to load an image.", "PATH");
    QCommandLineOption activeWindowOption(QStringList() << "s" << "show",
                                          "Show deepin draw.");
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("deepin-draw");
    cmdParser.addOption(openImageOption);
    cmdParser.addOption(activeWindowOption);
    cmdParser.process(*a.dApplication());

    QStringList paths = getFilesFromQCommandLineParser(cmdParser);
    //判断实例是否已经运行
    if (isRunning(a)) {
        DrawInterface *m_draw = new DrawInterface("com.deepin.Draw",
                                                  "/com/deepin/Draw", QDBusConnection::sessionBus(), &a);
        m_draw->openFiles(paths);
        return 0;
    }
    a.dApplication()->setApplicationVersion(VERSION);

    return a.execDraw(paths);
}

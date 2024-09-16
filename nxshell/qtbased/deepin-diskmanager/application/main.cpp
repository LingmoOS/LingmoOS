// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "environments.h"
#include "cusapplication.h"
#include "common.h"
#include "widgets/mainwindow.h"
#include "partedproxy/dmdbushandler.h"
#include "widgets/accessible/accessiblewidget.h"

#include <DMainWindow>
#include <DWidgetUtil>
#include <DApplicationSettings>
#include <DLog>
#include <DMainWindow>

#include <QPixmap>
#include <QIcon>
#include <QDebug>
#include <QProcess>
#include <QLabel>
#include <QComboBox>
#include <QAccessible>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

DWIDGET_USE_NAMESPACE

//void dealsig(int sig)
//{
//    qDebug() << "kdsfjjjjjj------";
//}

// 接口工厂
QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;

    if (object && object->isWidgetType()) {
        if (classname == "QLabel")
            interface = new AccessibleLabel(qobject_cast<QLabel *>(object));

        if (classname == "QPushButton")
            interface = new AccessibleButton(qobject_cast<QPushButton *>(object));

        if (classname == "QComboBox")
            interface = new AccessibleComboBox(qobject_cast<QComboBox *>(object));
    }

    return interface;
}

/**
 * @brief 执行外部命令
 * @param strCmd:外部命令字符串
 * @param outPut:命令控制台输出
 * @param error:错误信息
 * @return exitcode:退出码
 */
int executCmd(const QString &strCmd, QString &outPut, QString &error)
{
    QProcess proc;
    proc.start(strCmd);
    proc.waitForFinished(-1);
    outPut = proc.readAllStandardOutput();
    error = proc.readAllStandardError();
    error = proc.errorString();
    int exitcode = proc.exitCode();
    proc.close();
    return exitcode;

}

int main(int argc, char *argv[])
{
    // signal(SIGINT, SIG_IGN);
    // signal(SIGKILL, SIG_IGN);

    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")){
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    if (XDG_SESSION_TYPE == QLatin1String("x11")) {
        CusApplication::loadDXcbPlugin();
    }

    CusApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.loadTranslator();
    a.setApplicationName(appName);
    a.setOrganizationName("deepin");
    a.setWindowIcon(QIcon::fromTheme(appName));
    a.setApplicationDisplayName(QObject::tr("Disk Utility"));
    a.setApplicationVersion(/*DApplication::buildVersion("20191227")*/VERSION);
    const QString acknowledgementLink = "https://www.deepin.org/acknowledgments/deepin_reader";
    a.setApplicationAcknowledgementPage(acknowledgementLink);

    QPixmap px(QIcon::fromTheme(appName).pixmap(static_cast<int>(256 * qApp->devicePixelRatio()), static_cast<int>(256 * qApp->devicePixelRatio())));
    px.setDevicePixelRatio(qApp->devicePixelRatio());
    a.setProductIcon(QIcon(px));
    a.setApplicationDescription(QObject::tr("Disk Utility is a disk management tool for creating, reorganizing and formatting partitions."));
    DApplicationSettings savetheme;
    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    // 安装工厂
    QAccessible::installFactory(accessibleFactory);

//    MainWindow w;
//    if (a.setSingleInstance(appName)) {
//        QObject::connect(&a, &DApplication::newInstanceStarted, &w, [&] {qDebug() << "======"; w.activateWindow(); });
//    } else {
//        exit(0);
//    }
    if (a.setSingleInstance(appName)) {
        QProcess proc;
        QString cmd, oldPid, newPid, error;
        //先判断后台服务进程是否存在,如果存在可能是强制退出导致,应先退出后台程序再重新启动磁盘管理器
        cmd = QString("pidof deepin-diskmanager-service");

        if (!executCmd(cmd, oldPid, error)) {
            proc.startDetached("/usr/bin/dbus-send --system --type=method_call --dest=com.deepin.diskmanager /com/deepin/diskmanager com.deepin.diskmanager.Quit");
        }

        QStringList argList;
        argList << QString::number(QCoreApplication::applicationPid()) << QDBusConnection::systemBus().baseService();
        proc.startDetached("/usr/bin/deepin-diskmanager-authenticateProxy", argList);

        //正常启动程序后,循环查询后台服务是否已经启动,如果后台服务启动说明鉴权成功,启动前端界面
        while (1) {
            cmd = QString("pidof deepin-diskmanager-service");

            if (!executCmd(cmd, newPid, error) && oldPid != newPid) {
                break;
            }
            QThread::msleep(300);
        }

        proc.close();

    } else {
        exit(0);
    }

    MainWindow w;
    QObject::connect(&a, &DApplication::newInstanceStarted, &w, [&] {qDebug() << "======"; w.activateWindow(); });
    QObject::connect(&a, &CusApplication::handleQuitActionChanged, &w, &MainWindow::onHandleQuitAction);

//    DMDbusHandler::instance()->startService(static_cast<qint64>(getpid()));
//    QObject::connect(DMDbusHandler::instance(), &DMDbusHandler::rootLogin, &w, [&] {
//        qDebug() << "Root Authentication Result:" << w.getRootLoginResult();
//        if (w.getRootLoginResult() == "1") {
//            w.show();
//            Dtk::Widget::moveToCenter(&w);
//        } else {
//            exit(0);
//        }
//    });

    Dtk::Widget::moveToCenter(&w);
    w.show();

    return a.exec();
}


// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"


#include <DApplication>
#include <DLog>
#include <DDialog>
#include <QDebug>
#include "graphicsdriverproxy.h"
#include <QObject>
#include <DSpinner>
#include <QCloseEvent>
#include <QEvent>


DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

const QString GraphicMangerServiceName = "com.deepin.graphicmanger";
const QString GraphicMangerPath = "/com/deepin/graphicmanger";

class My_DDialog : public DDialog
{
protected:
    void closeEvent(QCloseEvent *event){
        event->ignore();
    }
};

ComDeepinDaemonGraphicsDriverInterface *g_graphicsDriver = nullptr;

DDialog *dialog(const QString &message, const QString &iconName)
{
    const auto ratio = qApp->devicePixelRatio();
    QPixmap iconPix = QIcon::fromTheme(iconName).pixmap(QSize(32, 32) * ratio);
    iconPix.setDevicePixelRatio(ratio);

    DDialog *d = new DDialog;
    d->setMessage(message);
    d->setIcon(iconPix, QSize(32, 32));
    return d;
}


My_DDialog *my_dialog(const QString &message, const QString &iconName)
{
    const auto ratio = qApp->devicePixelRatio();
    QPixmap iconPix = QIcon::fromTheme(iconName).pixmap(QSize(32, 32) * ratio);
    iconPix.setDevicePixelRatio(ratio);

    My_DDialog *d = new My_DDialog;
    d->setMessage(message);
    d->setIcon(iconPix, QSize(32, 32));
    return d;
}



void show_success_dialog()
{
    QString new_driver = "new_driver";
    QDBusPendingReply<QString> reply = g_graphicsDriver->GetNewDriverName();
    reply.waitForFinished();
    if (reply.isValid()) {
        new_driver = reply.value();
        qDebug() << "new dirver" << new_driver;
        new_driver = Utils::QStringToJson(new_driver)["title"].toString();
    } else {
        qDebug() << reply.error();
    }

    qDebug() << "new_driver = " << new_driver;
    new_driver = qApp->translate("Resolution", new_driver.toStdString().c_str());
    const QString &message = qApp->translate("main", "Congratulations, you have switched to %1, please reboot to take effect.");

    DDialog *d = dialog(message.arg(new_driver), "://resources/icons/deepin-graphics-driver-manager-success.svg");
    d->addButton(qApp->translate("main", "Cancel"));
    d->addButton(qApp->translate("main", "Reboot"));
    d->setDefaultButton(1);

    QObject::connect(d, &DDialog::buttonClicked, [=](int index, const QString &text) {
        Q_UNUSED(text);
        if (index == 1)
        {
            QProcess::startDetached("dbus-send --system --print-reply --dest=org.freedesktop.login1 /org/freedesktop/login1 org.freedesktop.login1.Manager.Reboot boolean:true");
        }
    });

    d->exec();
}

void show_fail_dialog()
{
    QString old_driver;
    QString new_driver;
    QDBusPendingReply<QString> oldDriverReply = g_graphicsDriver->GetOldDriverName();
    oldDriverReply.waitForFinished();
    if (oldDriverReply.isValid()) {
        old_driver = oldDriverReply.value();
        qDebug() << "old driver" << old_driver;
        old_driver = Utils::QStringToJson(old_driver)["title"].toString();
    } else {
        qDebug() << oldDriverReply.error();
    }

    QDBusPendingReply<QString> newDriverReply = g_graphicsDriver->GetNewDriverName();
    newDriverReply.waitForFinished();
    if (newDriverReply.isValid()) {
        new_driver = newDriverReply.value();
        qDebug() << "new dirver" << new_driver;
        new_driver = Utils::QStringToJson(new_driver)["title"].toString();
    } else {
        qDebug() << newDriverReply.error();
    }

    const QString &message = qApp->translate("main", "Auto restore to %2 after failed to switch to %1");

    new_driver = qApp->translate("Resolution", new_driver.toStdString().c_str());
    old_driver = qApp->translate("Resolution", old_driver.toStdString().c_str());
    DDialog *d = dialog(message.arg(new_driver).arg(old_driver), "://resources/icons/deepin-dialog-warning.svg");
    d->addButton(qApp->translate("main", "Submit Feedback"));

    QObject::connect(d, &DDialog::buttonClicked, [=] { QProcess::startDetached("deepin-feedback"); });

    d->exec();
}

int show_install_dialog() {

    My_DDialog *installDialog = my_dialog(qApp->translate("main", "Updating the driver, please wait..."), "://resources/icons/deepin-graphics-driver-manager-installing.svg");
    DSpinner *spinner = new DSpinner(installDialog);
    installDialog->addContent(spinner, Qt::AlignCenter);
    spinner->setFixedSize(32, 32);
    spinner->setVisible(true);
    spinner->start();
    QDBusPendingReply<void> realInstallReply =  g_graphicsDriver->RealInstall();
    realInstallReply.waitForFinished();
    if (realInstallReply.isValid()) {
        QObject::connect(g_graphicsDriver, &ComDeepinDaemonGraphicsDriverInterface::ReportProgress, [=](QString ratio){
            if (ratio.toInt() >= 100) {
                installDialog->done(0);
            } else if (ratio.toInt() < 0) {
                installDialog->done(1);
            }
        });
    } else {
        qDebug() << realInstallReply.error();
    }
    return installDialog->exec();
}


void init()
{
    Utils::resetDisablePluginList();
    bool testSuccess = false;
    QDBusPendingReply<bool> reply = g_graphicsDriver->IsTestSuccess();
    reply.waitForFinished();
    if (reply.isValid()) {
        testSuccess = reply.value();
    } else {
        qDebug() << reply.error();
    }

    qDebug() << "testSuccess is:" << testSuccess;
    if (testSuccess) {
        const int exitCode = show_install_dialog();
        qDebug() << "show_install_dialog exitCode" << exitCode;
        if (exitCode == 0) {
            qDebug() << "show_success_dialog";
            show_success_dialog();
        } else {
            qDebug() << "show_fail_dialog1";
            show_fail_dialog();
        }
    }
    else {
        qDebug() << "show_fail_dialog2";
        show_fail_dialog();
    }
}


int main(int argc, char *args[])
{
    DApplication dapp(argc, args);
    dapp.setQuitOnLastWindowClosed(true);
    dapp.setOrganizationName("deepin");
    dapp.setApplicationName("deepin-graphics-driver-manager-installer");

    QTranslator translator;
    translator.load(QString(TRANSLATIONS_DIR"deepin-graphics-driver-manager_%1.qm").arg(QLocale::system().name()));
    dapp.installTranslator(&translator);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    g_graphicsDriver = new ComDeepinDaemonGraphicsDriverInterface(
                GraphicMangerServiceName,
                GraphicMangerPath,
                QDBusConnection::systemBus());

    QTimer::singleShot(1, nullptr, init);

    return dapp.exec();
}


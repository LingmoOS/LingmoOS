/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "screenshotview.h"

#include <QClipboard>
#include <QEventLoop>
#include <QTimer>

#include <QGuiApplication>
#include <QQmlContext>
#include <QScreen>
#include <QPixmap>
#include <QStandardPaths>
#include <QDateTime>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDir>

ScreenshotView::ScreenshotView(QQuickView *parent)
    : QQuickView(parent)
{
    rootContext()->setContextProperty("view", this);
    QString filePath = "/usr/bin/lingmo-ocr";
    QFile file(filePath);
    if(file.exists())
    {
        m_ocrEnabled=true;
    }
    else
    {
        m_ocrEnabled=false;
    }
    emit ocrEnabledChanged();
    setFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    setScreen(qGuiApp->primaryScreen());
    setResizeMode(QQuickView::SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/main.qml"));
    setGeometry(screen()->geometry());
}

bool ScreenshotView::ocrEnabled() const
{
    return m_ocrEnabled;
}

void ScreenshotView::start()
{
    // 保存图片
    QPixmap p = qGuiApp->primaryScreen()->grabWindow(0);
    p.save("/tmp/lingmo-screenshot.png");

    setVisible(true);
    setKeyboardGrabEnabled(true);

    emit refresh();
}

void ScreenshotView::delay(int value)
{
    QEventLoop waitLoop;
    QTimer::singleShot(value, &waitLoop, SLOT(quit()));
    waitLoop.exec();

    start();
}

void ScreenshotView::quit()
{
    qGuiApp->quit();
}

void ScreenshotView::ocr(QRect rect)
{
    setVisible(false);
    QDir dir;
    dir.mkpath("/tmp/lingmo-screenshot/");
    QString fileName = QString("%1/Screenshot_%2.png")
                              .arg("/tmp/lingmo-screenshot")
                              .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QImage image("/tmp/lingmo-screenshot.png");
    QImage cropped = image.copy(rect);
    bool saved = cropped.save(fileName);

    if (saved) {
        QProcess process(this);
        process.startDetached("lingmo-ocr "+fileName);
    }

    removeTmpFile();
    this->quit();
}

void ScreenshotView::saveFile(QRect rect)
{
    setVisible(false);

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString fileName = QString("%1/Screenshot_%2.png")
                              .arg(desktopPath)
                              .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QImage image("/tmp/lingmo-screenshot.png");
    QImage cropped = image.copy(rect);
    bool saved = cropped.save(fileName);

    if (saved) {
        QDBusInterface iface("org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QList<QVariant> args;
            args << "lingmo-screenshot";
            args << ((unsigned int) 0);
            args << "lingmo-screenshot";
            args << "";
            args << tr("The picture has been saved to %1").arg(fileName);
            args << QStringList();
            args << QVariantMap();
            args << (int) 10;
            iface.asyncCallWithArgumentList("Notify", args);
        }
    }

    removeTmpFile();
    this->quit();
}

void ScreenshotView::copyToClipboard(QRect rect)
{
    setVisible(false);

    QImage image("/tmp/lingmo-screenshot.png");
    QImage cropped = image.copy(rect);
    QClipboard *clipboard = qGuiApp->clipboard();
    clipboard->setImage(cropped);

    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    if (iface.isValid()) {
        QList<QVariant> args;
        args << "lingmo-screenshot";
        args << ((unsigned int) 0);
        args << "lingmo-screenshot";
        args << "";
        args << tr("The picture has been saved to the clipboard");
        args << QStringList();
        args << QVariantMap();
        args << (int) 10;
        iface.asyncCallWithArgumentList("Notify", args);
    }

    removeTmpFile();

    QTimer::singleShot(100, qGuiApp, &QGuiApplication::quit);
}

void ScreenshotView::removeTmpFile()
{
    QFile("/tmp/lingmo-screenshot.png").remove();
}

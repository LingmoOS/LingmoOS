// Copyright (C) 2024 rewine <luhongxu@lingmo.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusArgument>
#include <QVariantMap>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->setButton, &QPushButton::clicked, this, &MainWindow::setWallpaper);

    interface = new QDBusInterface("org.freedesktop.portal.Desktop",
                                   "/org/freedesktop/portal/desktop",
                                   "org.freedesktop.portal.Wallpaper",
                                   QDBusConnection::sessionBus(),
                                   this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setWallpaper()
{
    QString app_id = "G4";
    QString parent_window = "";
    QString config = "test_config.ini";
    QSettings settings(config, QSettings::IniFormat);
    QString uri = settings.value("Wallpaper/path", "/home/uos/Pictures/Wallpapers/abc-123.jpg").toString();
    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=========" << uri;

    QVariantMap options;
    options.insert("output", "X11-2");
    options.insert("set-on", "background");

    QDBusMessage reply = interface->call("SetWallpaperURI",
                                         parent_window,
                                         uri,
                                         QVariant::fromValue(options));

           // 检查返回值
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Failed to set wallpaper:" << reply.errorMessage();
        return ;
    }

           // 处理返回值
    uint response = reply.arguments().at(0).toUInt();
    qDebug() << "Wallpaper set with response code:" << response;
}

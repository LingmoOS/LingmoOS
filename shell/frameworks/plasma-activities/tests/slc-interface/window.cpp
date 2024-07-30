/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "window.h"

#include "ui_window.h"

#include <QDBusConnection>

Window::Window()
    : ui(new Ui::MainWindow())
    , slc(new org::kde::ActivityManager::SLC("org.kde.ActivityManager", "/SLC", QDBusConnection::sessionBus(), this))
{
    ui->setupUi(this);

    connect(slc, &org::kde::ActivityManager::SLC::focusChanged, this, &Window::focusChanged);
}

Window::~Window()
{
    delete ui;
}

void Window::focusChanged(const QString &uri, const QString &mimetype, const QString &title)
{
    Q_UNUSED(mimetype);
    Q_UNUSED(title);
    ui->textCurrentResource->setText(uri);
}

#include "moc_window.cpp"

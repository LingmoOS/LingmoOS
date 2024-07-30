/*
    SPDX-FileCopyrightText: 2013-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "slc_interface.h"
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window();
    ~Window();

private Q_SLOTS:
    void focusChanged(const QString &uri, const QString &mimetype, const QString &title);

private:
    Ui::MainWindow *ui;
    org::kde::ActivityManager::SLC *slc;
};

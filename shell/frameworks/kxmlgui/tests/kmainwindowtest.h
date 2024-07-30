/*
    SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KMAINWINDOWTEST_H
#define KMAINWINDOWTEST_H

#include <kmainwindow.h>

class MainWindow : public KMainWindow
{
    Q_OBJECT
public:
    MainWindow();

private Q_SLOTS:
    void showMessage();
};

#endif // KMAINWINDOWTEST_H

// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>

#include "previewwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    PreviewWindow w(argc > 1 ? a.arguments().last() : "/usr/lib/xscreensaver");
    w.show();

    return a.exec();
}

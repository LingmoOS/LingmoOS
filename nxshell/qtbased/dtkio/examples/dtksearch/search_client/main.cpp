// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mainwindow.h"

#include <DApplication>

#include <memory>

DWIDGET_USE_NAMESPACE
int main(int argc, char *argv[])
{
    std::unique_ptr<DApplication> a(DApplication::globalApplication(argc, argv));

    MainWindow w;
    w.show();

    return a->exec();
}

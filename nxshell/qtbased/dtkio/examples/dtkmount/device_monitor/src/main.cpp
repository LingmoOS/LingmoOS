// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "window.h"
#include <DApplication>
#include <QTranslator>
#include <memory>

int main(int argc, char *argv[])
{
    std::unique_ptr<DApplication> a(DApplication::globalApplication(argc, argv));
    DApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    a->loadTranslator();

    Window w;
    w.show();
    return a->exec();
}

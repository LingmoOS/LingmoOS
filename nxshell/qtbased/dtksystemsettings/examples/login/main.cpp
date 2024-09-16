// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "demo.h"
#include "powerdemo.h"
#include "propertydemo.h"
#include "signaldemo.h"

#include <QCoreApplication>

DLOGIN_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QList<LoginDemo *> demos = { new SignalDemo(&app),
                                 new PropertyDemo(&app),
                                 new PowerDemo(&app) };
    foreach (const auto demo, demos) {
        demo->run();
    }
    return app.exec();
}

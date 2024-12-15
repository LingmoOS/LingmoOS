// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbus/DimDBusAdaptor.h"
#include "dimadaptor.h"
#include "dimcore/Dim.h"

#include <QGuiApplication>

using namespace org::lingmo::dim;

#ifdef Dtk6Core_FOUND
#  include <DLog>

using Dtk::Core::DLogManager;
#endif

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

#ifdef Dtk6Core_FOUND
    DLogManager::registerJournalAppender();
    DLogManager::registerConsoleAppender();
#endif

    Dim *dim = new Dim();

    auto dimDBusAdaptor = new DimDBusAdaptor(dim);
    new DimAdaptor(dimDBusAdaptor);
    QDBusConnection::sessionBus().registerService("org.lingmo.ocean.Dim");
    QDBusConnection::sessionBus().registerObject("/org/lingmo/ocean/Dim",
                                                 "org.lingmo.ocean.Dim",
                                                 dimDBusAdaptor);

    return a.exec();
}

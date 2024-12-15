// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLIPBOARDCONTROLLER_H
#define CLIPBOARDCONTROLLER_H

#include <DSingleton>

#include <QDBusInterface>
#include <QObject>

class ClipboardController : public QObject, public Dtk::Core::DSingleton<ClipboardController>
{
    friend Dtk::Core::DSingleton<ClipboardController>;

    Q_OBJECT
public:
    ClipboardController()
    {
    }

    ~ClipboardController() { }

    void toggle()
    {
        QDBusInterface interface("org.lingmo.ocean.Clipboard1",
                                 "/org/lingmo/ocean/Clipboard1",
                                 "org.lingmo.ocean.Clipboard1",
                                 QDBusConnection::sessionBus());
        if (interface.isValid()) {
            interface.call("Toggle");
        }
    }
};

#endif

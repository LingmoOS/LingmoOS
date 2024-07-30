/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQTEXTENSIONPLUGIN_H
#define BLUEZQTEXTENSIONPLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "pendingcall.h"
#include "rfkill.h"

struct PendingCallForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(PendingCall)
    QML_FOREIGN(BluezQt::PendingCall)
    QML_UNCREATABLE("")
};

struct RfkillForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(Rfkill)
    QML_FOREIGN(BluezQt::Rfkill)
    QML_UNCREATABLE("")
};

class BluezQtExtensionPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // BLUEZQTEXTENSIONPLUGIN_H

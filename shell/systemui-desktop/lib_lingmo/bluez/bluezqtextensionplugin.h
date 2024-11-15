/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQTEXTENSIONPLUGIN_H
#define BLUEZQTEXTENSIONPLUGIN_H

#include <QQmlExtensionPlugin>

class BluezQtExtensionPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Lingmo.Bluez")

public:
    void registerTypes(const char *uri) override;
};

#endif // BLUEZQTEXTENSIONPLUGIN_H

/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "DiskQuota.h"
#include "QuotaListModel.h"

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class DiskQuotaPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<DiskQuota>(uri, 1, 0, "DiskQuota");
        qmlRegisterType<QuotaListModel>(uri, 1, 0, "QuotaListModel");
    }
};

#include "plugin.moc"

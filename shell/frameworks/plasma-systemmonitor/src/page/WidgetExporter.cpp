/*
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "WidgetExporter.h"

#include "FaceLoader.h"
#include "PageDataObject.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

static const QString plasmashellService = QStringLiteral("org.kde.plasmashell");

WidgetExporter::WidgetExporter(QObject *parent)
    : QObject(parent)
{
    m_plasmashellAvailable = QDBusConnection::sessionBus().interface()->isServiceRegistered(plasmashellService);
    auto serviceWatcher = new QDBusServiceWatcher(plasmashellService, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this] {
        m_plasmashellAvailable = true;
        Q_EMIT plasmashellAvailableChanged();
    });
    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this] {
        m_plasmashellAvailable = false;
        Q_EMIT plasmashellAvailableChanged();
    });
}

void WidgetExporter::exportAsWidget(FaceLoader *loader) const
{
    const PageDataObject *data = loader->dataObject();
    const QString group = data->value(QStringLiteral("face")).toString();
    const KConfigGroup configGroup = data->config()->group(group);

    QString script = QStringLiteral(R"(a = currentActivity()
        d = desktopsForActivity(a)[0]
        w = d.addWidget('org.kde.plasma.systemmonitor'))");
    script += QChar('\n');
    script += configEntriesScript(configGroup);
    script += QStringLiteral("w.reloadConfig\n");

    auto call = QDBusMessage::createMethodCall(plasmashellService,
                                               QStringLiteral("/PlasmaShell"),
                                               QStringLiteral("org.kde.PlasmaShell"),
                                               QStringLiteral("evaluateScript"));
    call.setArguments({script});
    QDBusConnection::sessionBus().asyncCall(call);
}

QString WidgetExporter::configEntriesScript(const KConfigGroup &group, const QStringList &path) const
{
    QString script = QStringLiteral("w.currentConfigGroup = new Array(%1)\n").arg(path.join(','));
    for (const auto &entry : group.keyList()) {
        script += QStringLiteral("w.writeConfig('%1','%2')\n").arg(entry, group.readEntry(entry));
    }
    for (const auto &groupName : group.groupList()) {
        QStringList groupPath = path;
        groupPath.append(QStringLiteral("'%1'").arg(groupName));
        script += configEntriesScript(group.group(groupName), groupPath);
    }
    return script;
}

#include "moc_WidgetExporter.cpp"

/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "ToolsModel.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingCall>

#include <QDebug>

// #include <KRun>
#include <KGlobalAccel>
#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>

ToolsModel::ToolsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    addFromService(QStringLiteral("org.kde.konsole"));
    //     addFromService(QStringLiteral("org.kde.ksysguard"));
    addFromService(QStringLiteral("org.kde.ksystemlog"));
    addFromService(QStringLiteral("org.kde.kinfocenter"));
    addFromService(QStringLiteral("org.kde.filelight"));
    addFromService(QStringLiteral("org.kde.sweeper"));
    addFromService(QStringLiteral("org.kde.kmag"));
    addFromService(QStringLiteral("htop"));

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.KWin"))) {
        auto entry = Entry{};
        entry.id = QStringLiteral("killWindow");
        entry.icon = QStringLiteral("document-close");
        entry.name = i18nc("@action:inmenu", "Kill a Window…");
        const auto killWindowShortcutList = KGlobalAccel::self()->globalShortcut(QStringLiteral("kwin"), QStringLiteral("Kill Window"));
        if (!killWindowShortcutList.isEmpty()) {
            entry.shortcut = killWindowShortcutList.first().toString();
        }
        m_entries << entry;
    }
}

QHash<int, QByteArray> ToolsModel::roleNames() const
{
    static QHash<int, QByteArray> names = {//
                                           {IdRole, "id"},
                                           {NameRole, "name"},
                                           {IconRole, "icon"},
                                           {ShortcutRole, "shortcut"}};
    return names;
}

int ToolsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_entries.count();
}

QVariant ToolsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::DoNotUseParent)) {
        return QVariant{};
    }

    auto entry = m_entries.at(index.row());
    switch (role) {
    case IdRole:
        return entry.id;
    case NameRole:
        return entry.name;
    case IconRole:
        return entry.icon;
    case ShortcutRole:
        return entry.shortcut;
    }

    return QVariant{};
}

void ToolsModel::trigger(const QString &id)
{
    auto itr = std::find_if(m_entries.cbegin(), m_entries.cend(), [id](const Entry &entry) {
        return entry.id == id;
    });
    if (itr == m_entries.cend()) {
        return;
    }

    if (itr->service) {
        auto job = new KIO::ApplicationLauncherJob(itr->service);
        job->start();
    }

    if (itr->id == QStringLiteral("killWindow")) {
        auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/KWin"),
                                                      QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("killWindow"));
        QDBusConnection::sessionBus().asyncCall(message);
    }
}

void ToolsModel::addFromService(const QString &serviceName)
{
    auto service = KService::serviceByDesktopName(serviceName);
    if (service) {
        Entry entry;
        entry.id = serviceName;
        entry.name = i18nc("@action:inmenu %1 is application name", "Launch %1", service->name());
        entry.icon = service->icon();
        entry.service = service;
        m_entries << entry;
    }
}

#include "moc_ToolsModel.cpp"

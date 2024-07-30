/*
    SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ksambasharemodel.h"

#include <QApplication> // for kpropertiesdialog parenting
#include <QDBusPendingCallWatcher>
#include <QMetaEnum>

#include <KPropertiesDialog>
#include <KSambaShare>

#include "org.freedesktop.Avahi.Server.h"

KSambaShareModel::KSambaShareModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(KSambaShare::instance(), &KSambaShare::changed, this, &KSambaShareModel::reloadData);
    metaObject()->invokeMethod(this, &KSambaShareModel::reloadData);
}

KSambaShareModel::~KSambaShareModel() = default;

int KSambaShareModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_list.size();
}

QVariant KSambaShareModel::data(const QModelIndex &index, int intRole) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Q_ASSERT(index.row() < m_list.length());

    static QMetaEnum roleEnum = QMetaEnum::fromType<Role>();
    if (roleEnum.valueToKey(intRole) == nullptr) {
        return QVariant();
    }
    const auto role = static_cast<Role>(intRole);

    const KSambaShareData &share = m_list.at(index.row());
    switch (role) {
    case Role::Name:
        return share.name();
    case Role::Path:
        return share.path();
    case Role::Comment:
        return share.comment();
    case Role::ShareUrl: {
        if (m_fqdn.isEmpty()) {
            return QVariant();
        }
        QUrl url;
        url.setScheme(QStringLiteral("smb"));
        url.setHost(m_fqdn);
        url.setPath(QStringLiteral("/") + share.name());
        return url;
    }
    }

    return QVariant();
}

Q_INVOKABLE void KSambaShareModel::showPropertiesDialog(int index)
{
    auto dialog = new KPropertiesDialog(QUrl::fromUserInput(m_list.at(index).path()), QApplication::activeWindow());
    dialog->setFileNameReadOnly(true);
    dialog->showFileSharingPage();
    dialog->show();
}

void KSambaShareModel::reloadData()
{
    beginResetModel();
    m_list.clear();
    const auto samba = KSambaShare::instance();
    const QStringList sharedDirectories = samba->sharedDirectories();
    for (const auto &path : sharedDirectories) {
        m_list += samba->getSharesByPath(path);
    }
    endResetModel();

    // Reload FQDN
    m_fqdn.clear();
    auto avahi = new OrgFreedesktopAvahiServerInterface(QStringLiteral("org.freedesktop.Avahi"), QStringLiteral("/"), QDBusConnection::systemBus(), this);
    auto watcher = new QDBusPendingCallWatcher(avahi->GetHostNameFqdn(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, avahi, watcher] {
        watcher->deleteLater();
        avahi->deleteLater();
        QDBusPendingReply<QString> reply = *watcher;
        if (reply.isError()) {
            // When Avahi isn't available there's not really a good way to resolve the FQDN. The user could drive
            // resolution through LLMNR or NetBios or some other ad-hoc system, neither provide us with an easy
            // way to get their configured FQDN. We are therefor opting to not render URLs in that scenario since
            // we can't get a name that will reliably work.
            m_fqdn.clear();
            return;
        }
        m_fqdn = reply.argumentAt(0).toString();
        Q_EMIT dataChanged(createIndex(0, 0), createIndex(m_list.count(), 0), {static_cast<int>(Role::ShareUrl)});
    });
}

bool KSambaShareModel::hasChildren(const QModelIndex &parent) const
{
    return !parent.isValid() ? false : (rowCount(parent) > 0);
}

QHash<int, QByteArray> KSambaShareModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (!roles.isEmpty()) {
        return roles;
    }

    const QMetaEnum roleEnum = QMetaEnum::fromType<Role>();
    for (int i = 0; i < roleEnum.keyCount(); ++i) {
        const int value = roleEnum.value(i);
        Q_ASSERT(value != -1);
        roles[static_cast<int>(value)] = QByteArray("ROLE_") + roleEnum.valueToKey(value);
    }
    return roles;
}

#include "moc_ksambasharemodel.cpp"

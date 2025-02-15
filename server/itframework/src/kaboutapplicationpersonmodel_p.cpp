/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kaboutapplicationpersonmodel_p.h"
#include "debug.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>

namespace KDEPrivate
{
KAboutApplicationPersonModel::KAboutApplicationPersonModel(const QList<KAboutPerson> &personList, QObject *parent)
    : QAbstractListModel(parent)
    , m_personList(personList)
{
    m_profileList.reserve(m_personList.size());
    bool hasAnyAvatars{false};
    for (const auto &person : std::as_const(m_personList)) {
        KAboutApplicationPersonProfile profile = KAboutApplicationPersonProfile(person.name(), person.task(), person.emailAddress(), person.ocsUsername());
        profile.setHomepage(QUrl(person.webAddress()));
        if (!profile.ocsUsername().isEmpty()) {
            hasAnyAvatars = true;
            profile.setOcsProfileUrl(QLatin1String{"https://store.kde.org/u/%1"}.arg(profile.ocsUsername()));
        }
        m_profileList.append(profile);
    }
    m_hasAnyAvatars = hasAnyAvatars;

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    connect(this, &KAboutApplicationPersonModel::showRemoteAvatarsChanged, [this, manager]() {
        if (showRemoteAvatars()) {
            int i = 0;
            for (const auto &profile : std::as_const(m_profileList)) {
                if (!profile.ocsUsername().isEmpty()) {
                    if (profile.avatar().isNull()) {
                        QNetworkRequest request(QUrl(QLatin1String{"https://store.kde.org/avatar/%1?s=%2"}.arg(profile.ocsUsername()).arg(AVATAR_HEIGHT)));
                        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
                        QNetworkReply *reply = manager->get(request);
                        reply->setProperty("personProfile", i);
                        connect(reply, &QNetworkReply::finished, this, &KAboutApplicationPersonModel::onAvatarJobFinished);
                        m_ongoingAvatarFetches << reply;
                    } else {
                        Q_EMIT dataChanged(index(i), index(i));
                    }
                }
                ++i;
            }
        } else {
            // We keep the avatars around, no use deleting them - just hide them in the UI
            // This way we don't cause unnecessary crunching on the user's connection if
            // they do a bunch of toggling. Just stop the current fetches, and trust the
            // consumer to understand it should not show avatars if this property is false.
            for (QNetworkReply *reply : m_ongoingAvatarFetches) {
                reply->abort();
            }
            m_ongoingAvatarFetches.clear();
            Q_EMIT dataChanged(index(0), index(m_profileList.count() - 1));
        }
    });
}

int KAboutApplicationPersonModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_personList.count();
}

QVariant KAboutApplicationPersonModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qCWarning(DEBUG_KXMLGUI) << "ERROR: invalid index";
        return QVariant();
    }
    if (index.row() >= rowCount()) {
        qCWarning(DEBUG_KXMLGUI) << "ERROR: index out of bounds";
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        //        qCDebug(DEBUG_KXMLGUI) << "Spitting data for name " << m_profileList.at( index.row() ).name();
        QVariant var;
        var.setValue(m_profileList.at(index.row()));
        return var;
    } else {
        return QVariant();
    }
}

Qt::ItemFlags KAboutApplicationPersonModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

void KAboutApplicationPersonModel::onAvatarJobFinished() // SLOT
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply) {
        m_ongoingAvatarFetches.removeAll(reply);
        if (reply->error() == QNetworkReply::NoError) {
            int personProfileListIndex = reply->property("personProfile").toInt();
            QNetworkAccessManager *manager = reply->manager();
            if (manager) {
                if (reply->error() != QNetworkReply::NoError) {
                    Q_EMIT dataChanged(index(personProfileListIndex), index(personProfileListIndex));
                    return;
                }
                reply->waitForReadyRead(1000);
                QByteArray data = reply->readAll();
                QPixmap pixmap;
                pixmap.loadFromData(data);

                KAboutApplicationPersonProfile profile = m_profileList.value(personProfileListIndex);
                if (!pixmap.isNull()) {
                    profile.setAvatar(pixmap);
                    if (!m_hasAvatarPixmaps) {
                        m_hasAvatarPixmaps = true;
                        // Data has changed for all the elements now... otherwise layouts will be all wonky in our delegates
                        Q_EMIT dataChanged(index(0), index(m_profileList.count() - 1));
                    }
                } else {
                    // Failed to read pixmap data, so... let's load something useful maybe?
                }

                m_profileList.replace(personProfileListIndex, profile);
                Q_EMIT dataChanged(index(personProfileListIndex), index(personProfileListIndex));
            }
        }
        reply->deleteLater();
    }
}

} // namespace KDEPrivate

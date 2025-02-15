/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KABOUT_APPLICATION_PERSON_MODEL_H
#define KABOUT_APPLICATION_PERSON_MODEL_H

#include <KAboutData>

#include <QAbstractListModel>
#include <QIcon>
#include <QNetworkReply>
#include <QPixmap>
#include <QUrl>

namespace KDEPrivate
{
enum {
    AVATAR_HEIGHT = 50,
    AVATAR_WIDTH = 50,
    MAIN_LINKS_HEIGHT = 32,
    SOCIAL_LINKS_HEIGHT = 26,
    MAX_SOCIAL_LINKS = 9,
};
class KAboutApplicationPersonProfile;

class KAboutApplicationPersonModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool hasAnyAvatars READ hasAnyAvatars NOTIFY hasAnyAvatarsChanged)
    Q_PROPERTY(bool showRemoteAvatars READ showRemoteAvatars WRITE setShowRemoteAvatars NOTIFY showRemoteAvatarsChanged)
public:
    explicit KAboutApplicationPersonModel(const QList<KAboutPerson> &personList, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return 1;
    }
    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool hasAvatarPixmaps() const
    {
        return m_hasAvatarPixmaps;
    }
    bool hasAnyAvatars() const
    {
        return m_hasAnyAvatars;
    }
    Q_SIGNAL void hasAnyAvatarsChanged();

    const QString &providerName() const
    {
        return m_providerName;
    }

    bool showRemoteAvatars() const
    {
        return m_showRemoteAvatars;
    }
    void setShowRemoteAvatars(bool value)
    {
        if (m_showRemoteAvatars != value) {
            m_showRemoteAvatars = value;
            Q_EMIT showRemoteAvatarsChanged();
        }
    }
    Q_SIGNAL void showRemoteAvatarsChanged();
private Q_SLOTS:
    void onAvatarJobFinished();

private:
    const QList<KAboutPerson> m_personList;
    QList<KAboutApplicationPersonProfile> m_profileList;

    bool m_hasAnyAvatars = false;
    bool m_hasAvatarPixmaps = false;
    bool m_showRemoteAvatars = false;

    QString m_providerName;

    QList<QNetworkReply *> m_ongoingAvatarFetches;
};

class KAboutApplicationPersonProfile
{
public:
    KAboutApplicationPersonProfile()
        : m_name()
        , m_task()
        , m_email()
        , m_ocsUsername()
    {
    } // needed for QVariant

    KAboutApplicationPersonProfile(const QString &name, const QString &task, const QString &email, const QString &ocsUsername = QString())
        : m_name(name)
        , m_task(task)
        , m_email(email)
        , m_ocsUsername(ocsUsername)
    {
    }

    void setHomepage(const QUrl &url)
    {
        m_homepage = url;
    }
    void setAvatar(const QPixmap &pixmap)
    {
        m_avatar = pixmap;
    }
    void setLocation(const QString &location)
    {
        m_location = location;
    }
    void setOcsProfileUrl(const QString &url)
    {
        m_ocsProfileUrl = url;
    }

    const QString &name() const
    {
        return m_name;
    }
    const QString &task() const
    {
        return m_task;
    }
    const QString &email() const
    {
        return m_email;
    }
    const QString &ocsUsername() const
    {
        return m_ocsUsername;
    }
    const QString &ocsProfileUrl() const
    {
        return m_ocsProfileUrl;
    }
    const QUrl &homepage() const
    {
        return m_homepage;
    }
    const QPixmap &avatar() const
    {
        return m_avatar;
    }
    const QString &location() const
    {
        return m_location;
    }

private:
    QString m_name;
    QString m_task;
    QString m_email;
    QString m_ocsUsername;
    QString m_ocsProfileUrl;
    QUrl m_homepage;
    QPixmap m_avatar;
    QString m_location;
};

} // namespace KDEPrivate

Q_DECLARE_METATYPE(KDEPrivate::KAboutApplicationPersonProfile)
#endif // KABOUT_APPLICATION_PERSON_MODEL_H

/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KUSERPROXY_H
#define KUSERPROXY_H

#include <QObject>
#include <QUrl>

#include <KDirWatch>
#include <KUser>

/**
 * KUserProxy (exposed as KUser to the QML runtime) is an object allowing
 * read-only access to the user's name, os and version and the configured
 * user image. This object can be used to personalize user interfaces.
 *
 * Example usage:
 * @code
    import org.kde.coreaddons as KCoreAddons
    [...]

    Item {
        [...]
        KCoreAddons.KUser {
            id: kuser
        }

        Image {
            id: faceIcon
            source: kuser.faceIconUrl
            [...]
        }

        Text {
            text: kuser.fullName
            [...]
        }
    }
    @endcode

 * @short KUser provides read-only access to the user's personal information
 * @see KUser
 */
class KUserProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString fullName READ fullName NOTIFY nameChanged)
    Q_PROPERTY(QString loginName READ loginName NOTIFY nameChanged)
    Q_PROPERTY(QUrl faceIconUrl READ faceIconUrl NOTIFY faceIconUrlChanged)
    Q_PROPERTY(QString os READ os CONSTANT)
    Q_PROPERTY(QString host READ host CONSTANT)

public:
    KUserProxy(QObject *parent = nullptr);
    ~KUserProxy() override;

    /**
     * @return the full name of the user
     * @see nameChanged
     */
    QString fullName() const;

    /**
     * @return the user's login name
     * @see nameChanged
     *
     */
    QString loginName() const;

    /**
     * @return the url of the user's configured image (including file:/)
     * @see faceIconUrlChanged
     */
    QUrl faceIconUrl() const;

    /**
     * @return pretty name indicating operating system and version
     * @see nameChanged
     */
    QString os();

    /**
     * @return the system's hostname
     * @see nameChanged
     */
    QString host() const;

Q_SIGNALS:
    /**
     * signal that the user's name or login name changed
     * @see fullName
     * @see loginName
     */
    void nameChanged();
    /**
     * signal that the user image changed
     * @see faceIconUrl
     */
    void faceIconUrlChanged();

private:
    void update(const QString &path);
    KDirWatch m_dirWatch;
    KUser m_user;
    QString m_os;
    bool m_temporaryEmptyFaceIconPath;
};

#endif // KUSERPROXY_H

/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kuserproxy.h"
#include <QDir>
#include <QFile>
#include <QHostInfo>
#include <QTextStream>
#include <QUrl>

const QString etcPasswd = QStringLiteral("/etc/passwd");
const QString accountsServiceIconPath = QStringLiteral("/var/lib/AccountsService/icons");

KUserProxy::KUserProxy(QObject *parent)
    : QObject(parent)
    , m_temporaryEmptyFaceIconPath(false)
{
    QString pathToFaceIcon(m_user.faceIconPath());
    if (pathToFaceIcon.isEmpty()) {
        // KUser returns null if the current faceIconPath is empty
        // so we should explicitly watch ~/.face.icon rather than faceIconPath()
        // as we want to watch for this file being created
        pathToFaceIcon = QDir::homePath() + QStringLiteral("/.face.icon");
    }

    m_dirWatch.addFile(pathToFaceIcon);
    m_dirWatch.addFile(accountsServiceIconPath + QLatin1Char('/') + m_user.loginName());
    if (QFile::exists(etcPasswd)) {
        m_dirWatch.addFile(etcPasswd);
    }

    connect(&m_dirWatch, &KDirWatch::dirty, this, &KUserProxy::update);
    connect(&m_dirWatch, &KDirWatch::created, this, &KUserProxy::update);
    connect(&m_dirWatch, &KDirWatch::deleted, this, &KUserProxy::update);
}

KUserProxy::~KUserProxy()
{
}

void KUserProxy::update(const QString &path)
{
    if (path == m_user.faceIconPath() || path == QDir::homePath() + QLatin1String("/.face.icon")
        || path == accountsServiceIconPath + QLatin1Char('/') + m_user.loginName()) {
        // we need to force updates, even when the path doesn't change,
        // but the underlying image does. Change path temporarily, to
        // make the Image reload.
        // Needs cache: false in the Image item to actually reload
        m_temporaryEmptyFaceIconPath = true;
        Q_EMIT faceIconUrlChanged();
        m_temporaryEmptyFaceIconPath = false;
        Q_EMIT faceIconUrlChanged();
    } else if (path == etcPasswd) {
        m_user = KUser();
        Q_EMIT nameChanged();
    }
}

QString KUserProxy::fullName() const
{
    QString fullName = m_user.property(KUser::FullName).toString();
    if (!fullName.isEmpty()) {
        return fullName;
    }
    return loginName();
}

QString KUserProxy::loginName() const
{
    return m_user.loginName();
}

QUrl KUserProxy::faceIconUrl() const
{
    if (m_temporaryEmptyFaceIconPath) {
        return QUrl();
    }
    const QString u = m_user.faceIconPath();
    const QFile f(u);
    if (f.exists(u)) {
        // We need to return a file URL, not a simple path
        return QUrl::fromLocalFile(u);
    }
    return QUrl();
}

QString KUserProxy::os()
{
    if (m_os.isEmpty()) {
        QFile osfile(QStringLiteral("/etc/os-release"));
        if (osfile.exists()) {
            if (!osfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return QString();
            }

            QTextStream in(&osfile);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith(QLatin1String("PRETTY_NAME"))) {
                    QStringList fields = line.split(QLatin1String("PRETTY_NAME=\""));
                    if (fields.count() == 2) {
                        osfile.close();
                        QString pretty = fields.at(1);
                        pretty.chop(1);
                        m_os = pretty;
                        return pretty;
                    }
                }
            }
        }
    }
    return m_os;
}

QString KUserProxy::host() const
{
    return QHostInfo::localHostName();
}

#include "moc_kuserproxy.cpp"

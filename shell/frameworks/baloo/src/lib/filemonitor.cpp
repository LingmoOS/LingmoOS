/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filemonitor.h"

#include <QSet>
#include <QString>
#include <QStringList>
#include <QDBusConnection>

using namespace Baloo;

class BALOO_CORE_NO_EXPORT FileMonitor::Private {
public:
    QSet<QString> m_files;
};

FileMonitor::FileMonitor(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    QDBusConnection con = QDBusConnection::sessionBus();
    con.connect(QString(), QStringLiteral("/files"), QStringLiteral("org.kde"),
                QStringLiteral("changed"), this, SLOT(slotFileMetaDataChanged(QStringList)));
}

FileMonitor::~FileMonitor() = default;

void FileMonitor::addFile(const QString& fileUrl)
{
    QString f = fileUrl;
    if (f.endsWith(QLatin1Char('/'))) {
        f = f.mid(0, f.length() - 1);
    }

    d->m_files.insert(f);
}

void FileMonitor::addFile(const QUrl& url)
{
    const QString localFile = url.toLocalFile();
    if (!localFile.isEmpty()) {
        addFile(localFile);
    }
}

void FileMonitor::setFiles(const QStringList& fileList)
{
    d->m_files = QSet<QString>(fileList.begin(), fileList.end());
}

QStringList FileMonitor::files() const
{
    return QList<QString>(d->m_files.begin(), d->m_files.end());
}

void FileMonitor::clear()
{
    d->m_files.clear();
}

void FileMonitor::slotFileMetaDataChanged(const QStringList& fileUrls)
{
    for (const QString& url : fileUrls) {
        if (d->m_files.contains(url)) {
            Q_EMIT fileMetaDataChanged(url);
        }
    }
}

#include "moc_filemonitor.cpp"

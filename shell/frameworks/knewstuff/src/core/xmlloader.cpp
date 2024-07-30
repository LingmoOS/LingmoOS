/*
    knewstuff3/xmlloader.cpp.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "xmlloader_p.h"

#include "jobs/httpjob.h"
#include "knewstuffcore_debug.h"

#include <KConfig>

#include <QByteArray>
#include <QFile>
#include <QTimer>

namespace KNSCore
{
void handleData(XmlLoader *q, const QByteArray &data)
{
    qCDebug(KNEWSTUFFCORE) << "--Xml Loader-START--";
    qCDebug(KNEWSTUFFCORE) << QString::fromUtf8(data);
    qCDebug(KNEWSTUFFCORE) << "--Xml Loader-END--";
    QDomDocument doc;
    if (doc.setContent(data)) {
        Q_EMIT q->signalLoaded(doc);
    } else {
        Q_EMIT q->signalFailed();
    }
}

XmlLoader::XmlLoader(QObject *parent)
    : QObject(parent)
{
}

void XmlLoader::load(const QUrl &url)
{
    qCDebug(KNEWSTUFFCORE) << "XmlLoader::load(): url: " << url;
    // The load call is expected to be asynchronous (to allow for people to connect to signals
    // after it is called), and so we need to postpone its implementation until the listeners
    // are actually listening
    QTimer::singleShot(0, this, [this, url]() {
        m_jobdata.clear();
        static const QStringList remoteSchemeOptions{QLatin1String{"http"}, QLatin1String{"https"}, QLatin1String{"ftp"}};
        if (remoteSchemeOptions.contains(url.scheme())) {
            HTTPJob *job = HTTPJob::get(url, Reload, JobFlag::HideProgressInfo);
            connect(job, &KJob::result, this, &XmlLoader::slotJobResult);
            connect(job, &HTTPJob::data, this, &XmlLoader::slotJobData);
            connect(job, &HTTPJob::httpError, this, &XmlLoader::signalHttpError);
            Q_EMIT jobStarted(job);
        } else if (url.isLocalFile()) {
            QFile localProvider(url.toLocalFile());
            if (localProvider.open(QFile::ReadOnly)) {
                m_jobdata = localProvider.readAll();
                handleData(this, m_jobdata);
            } else {
                Q_EMIT signalFailed();
            }
        } else {
            // This is not supported
            qCDebug(KNEWSTUFFCORE) << "Attempted to load data from unsupported URL:" << url;
            Q_EMIT signalFailed();
        }
    });
}

void XmlLoader::slotJobData(KJob *, const QByteArray &data)
{
    m_jobdata.append(data);
}

void XmlLoader::slotJobResult(KJob *job)
{
    deleteLater();
    if (job->error()) {
        Q_EMIT signalFailed();
    } else {
        handleData(this, m_jobdata);
    }
}

QDomElement addElement(QDomDocument &doc, QDomElement &parent, const QString &tag, const QString &value)
{
    QDomElement n = doc.createElement(tag);
    n.appendChild(doc.createTextNode(value));
    parent.appendChild(n);

    return n;
}
} // end KNS namespace

#include "moc_xmlloader_p.cpp"

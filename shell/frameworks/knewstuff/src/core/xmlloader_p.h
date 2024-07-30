/*
    knewstuff3/xmlloader.h.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_XMLLOADER_P_H
#define KNEWSTUFF3_XMLLOADER_P_H

#include "provider.h"
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>
#include <qdom.h>

class KJob;

namespace KNSCore
{
QDomElement addElement(QDomDocument &doc, QDomElement &parent, const QString &tag, const QString &value);

/**
 * KNewStuff xml loader.
 * This class loads an xml document from a kurl and returns the
 * resulting domdocument once completed.
 * It should probably not be used directly by the application.
 *
 * @internal
 */
class XmlLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit XmlLoader(QObject *parent);

    /**
     * Starts asynchronously loading the xml document from the
     * specified URL.
     *
     * @param url location of the XML file
     */
    void load(const QUrl &url);

    void setFilter(Provider::Filter filter)
    {
        m_filter = filter;
    }

    void setSearchTerm(const QString &searchTerm)
    {
        m_searchTerm = searchTerm;
    }

    Provider::Filter filter() const
    {
        return m_filter;
    }

    QString searchTerm() const
    {
        return m_searchTerm;
    }
Q_SIGNALS:
    /**
     * Indicates that the list of providers has been successfully loaded.
     */
    void signalLoaded(const QDomDocument &);
    void signalFailed();
    /**
     * Fired in case there is a http error reported
     * In some instances this is useful information for our users, and we want to make sure we report this centrally
     * @param status The HTTP status code (fired in cases where it is perceived by QNetworkReply as an error)
     * @param rawHeaders The raw HTTP headers for the errored-out network request
     */
    void signalHttpError(int status, QList<QNetworkReply::RawHeaderPair> rawHeaders);

    void jobStarted(KJob *);

protected Q_SLOTS:
    void slotJobData(KJob *, const QByteArray &);
    void slotJobResult(KJob *);

private:
    QByteArray m_jobdata;
    Provider::Filter m_filter;
    QString m_searchTerm;
};

}

#endif

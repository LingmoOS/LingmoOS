/*
    SPDX-FileCopyrightText: 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef SYNDICATION_DATARETRIEVER_H
#define SYNDICATION_DATARETRIEVER_H

#include "syndication_export.h"

#include <QObject>

class QUrl;

class QByteArray;

namespace Syndication
{
/**
 * Abstract baseclass for all data retriever classes. Subclass this to add
 * a new retrieval algorithm which can then be plugged into the RSS loader.
 * @see Loader, FileRetriever, OutputRetriever
 */
class SYNDICATION_EXPORT DataRetriever : public QObject
{
    Q_OBJECT
public:
    /**
     * Default constructor.
     */
    DataRetriever();

    /**
     * Destructor.
     */
    ~DataRetriever() override;

    /**
     * Retrieve data from the given URL. This method is supposed to get
     * reimplemented by subclasses. It will be called by the Loader
     * class in case it needs to retrieve the data.
     *
     * @param url the URL to retrieve data from
     *
     * @see Loader::loadFrom()
     */
    virtual void retrieveData(const QUrl &url) = 0;

    /**
     * @return An error code which might give a more precise information
     * about what went wrong in case the 'success' flag returned with
     * the dataRetrieved() signal was 'false'. Note that the meaning of
     * the returned integer depends on the actual data retriever.
     */
    virtual int errorCode() const = 0;

    /**
     * aborts the retrieval process.
     */
    virtual void abort() = 0;

Q_SIGNALS:
    /**
     * Emit this signal to tell the Loader class that the retrieval
     * process was finished.
     * @param data Should contain the retrieved data and will get
     * parsed by the Loader class.
     * @param success Indicates whether there were any problems during
     * the retrieval process. Pass 'true' to indicate that everything
     * went seamlessy, 'false' to tell the Loader that something went
     * wrong and that the data parameter might contain no or invalid
     * data.
     */

    void dataRetrieved(const QByteArray &data, bool success);

private:
    DataRetriever(const DataRetriever &other);
    DataRetriever &operator=(const DataRetriever &other);
};

} // namespace Syndication

#endif // SYNDICATION_DATARETRIEVER_H

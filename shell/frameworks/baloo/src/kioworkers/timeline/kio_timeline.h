/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KIO_TIMELINE_H_
#define KIO_TIMELINE_H_

#include <KIO/WorkerBase>

#include <QDate>

namespace Baloo
{

class TimelineProtocol : public KIO::WorkerBase
{
public:
    TimelineProtocol(const QByteArray& poolSocket, const QByteArray& appSocket);
    ~TimelineProtocol() override;

    /**
     * List all files and folders tagged with the corresponding tag.
     */
    KIO::WorkerResult listDir(const QUrl& url) override;

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    KIO::WorkerResult mimetype(const QUrl& url) override;

    /**
     * Files will be forwarded.
     * Folders will be created as virtual folders.
     */
    KIO::WorkerResult stat(const QUrl& url) override;

private:
    void listDays(int month, int year);
    void listThisYearsMonths();
    bool filesInDate(const QDate& date);

    /// temp vars for the currently handled URL
    QDate m_date;
    QString m_filename;
};
}

#endif // KIO_TIMELINE_H_

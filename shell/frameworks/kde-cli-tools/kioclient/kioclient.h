/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1999-2006 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __kioclient_h
#define __kioclient_h

#include <QApplication>
#include <kio/udsentry.h>

class QCommandLineParser;
class KJob;
namespace KIO
{
class Job;
}

class ClientApp : public QObject
{
    Q_OBJECT
public:
    ClientApp();

    /** Parse command-line arguments and "do it" */
    bool doIt(const QCommandLineParser &parser);

private Q_SLOTS:
    void slotPrintData(KIO::Job *job, const QByteArray &data);
    void slotEntries(KIO::Job *job, const KIO::UDSEntryList &);
    void slotResult(KJob *);
    void slotStatResult(KJob *);
    void slotDialogCanceled();

private:
    bool kde_open(const QString &url, const QString &mimeType, bool allowExec);
    bool doCopy(const QStringList &urls);
    bool doMove(const QStringList &urls);
    bool doList(const QStringList &urls);
    bool doRemove(const QStringList &urls);
    bool doStat(const QStringList &urls);

    static bool m_ok;
};

#endif

/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KPARTS_READONLYPART_P_H
#define _KPARTS_READONLYPART_P_H

#include "openurlarguments.h"
#include "part_p.h"
#include "readonlypart.h"

namespace KIO
{
class FileCopyJob;
class StatJob;
}

namespace KParts
{
class ReadOnlyPartPrivate : public PartPrivate
{
public:
    Q_DECLARE_PUBLIC(ReadOnlyPart)

    explicit ReadOnlyPartPrivate(ReadOnlyPart *qq, const KPluginMetaData &data)
        : PartPrivate(qq, data)
    {
        m_job = nullptr;
        m_statJob = nullptr;
        m_uploadJob = nullptr;
        m_showProgressInfo = true;
        m_saveOk = false;
        m_waitForSave = false;
        m_duringSaveAs = false;
        m_bTemp = false;
        m_bAutoDetectedMime = false;
        m_closeUrlFromOpenUrl = false;
        m_closeUrlFromDestructor = false;
    }

    ~ReadOnlyPartPrivate() override
    {
    }

    void slotJobFinished(KJob *job);
    void slotStatJobFinished(KJob *job);
    void slotGotMimeType(KIO::Job *job, const QString &mime);
    bool openLocalFile();
    void openRemoteFile();

    KIO::FileCopyJob *m_job;
    KIO::StatJob *m_statJob;
    KIO::FileCopyJob *m_uploadJob;
    QUrl m_originalURL; // for saveAs
    QString m_originalFilePath; // for saveAs
    bool m_showProgressInfo : 1;
    bool m_saveOk : 1;
    bool m_waitForSave : 1;
    bool m_duringSaveAs : 1;

    /**
     * If @p true, @p m_file is a temporary file that needs to be deleted later.
     */
    bool m_bTemp : 1;

    // whether the mimetype in the arguments was detected by the part itself
    bool m_bAutoDetectedMime : 1;
    // Whether we are calling closeUrl() from openUrl().
    bool m_closeUrlFromOpenUrl;
    // Whether we are calling closeUrl() from ~ReadOnlyPart().
    bool m_closeUrlFromDestructor;

    /**
     * Remote (or local) url - the one displayed to the user.
     */
    QUrl m_url;

    /**
     * Local file - the only one the part implementation should deal with.
     */
    QString m_file;

    OpenUrlArguments m_arguments;
};

} // namespace

#endif

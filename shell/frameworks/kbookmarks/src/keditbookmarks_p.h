/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000, 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QString>

#include "kbookmarks_export.h"

class KBOOKMARKS_EXPORT KEditBookmarks
{
public:
    class OpenResult
    {
    public:
        bool sucess()
        {
            return m_sucess;
        }

        QString errorMessage() const
        {
            return m_errorMessage;
        }

    private:
        bool m_sucess;
        QString m_errorMessage;
        friend KEditBookmarks;
    };

    void setBrowserMode(bool browserMode);
    bool browserMode() const;

    OpenResult openForFile(const QString &file);

    OpenResult openForFileAtAddress(const QString &file, const QString &address);

private:
    OpenResult startKEditBookmarks(const QStringList &args);

    bool m_browserMode = false;
};

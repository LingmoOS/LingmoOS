/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KPARTS_READWRITEPART_P_H
#define _KPARTS_READWRITEPART_P_H

#include "readonlypart_p.h"
#include "readwritepart.h"

#include <QEventLoop>

namespace KParts
{
class ReadWritePartPrivate : public ReadOnlyPartPrivate
{
public:
    Q_DECLARE_PUBLIC(ReadWritePart)

    explicit ReadWritePartPrivate(ReadWritePart *qq, const KPluginMetaData &data)
        : ReadOnlyPartPrivate(qq, data)
    {
        m_bModified = false;
        m_bReadWrite = true;
        m_bClosing = false;
    }

    void slotUploadFinished(KJob *job);

    void prepareSaving();

    bool m_bModified;
    bool m_bReadWrite;
    bool m_bClosing;
    QEventLoop m_eventLoop;
};

} // namespace

#endif

/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Lars Doelle <lars.doelle@on-line.de>

    SPDX-License-Identifier: GPL-2.0-only
*/

#ifndef KDESUPTYPROCESS_P_H
#define KDESUPTYPROCESS_P_H

#include <KPty>

#include <QByteArray>
#include <QList>

namespace KDESu
{
class PtyProcessPrivate
{
public:
    PtyProcessPrivate()
    {
    }
    virtual ~PtyProcessPrivate()
    {
        delete pty;
    }

    QList<QByteArray> env;
    KPty *pty = nullptr;
    QByteArray inputBuffer;
    // Whether to keep echo on after PTY creation
    bool wantLocalEcho = true;
};

}

#endif

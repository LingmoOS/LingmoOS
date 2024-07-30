/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPROCESS_P_H
#define KPROCESS_P_H

#include "kprocess.h"

class KProcessPrivate
{
    Q_DECLARE_PUBLIC(KProcess)
protected:
    KProcessPrivate(KProcess *qq)
        : openMode(QIODevice::ReadWrite)
        , q_ptr(qq)
    {
    }

    QIODevice::OpenMode openMode;

    KProcess *q_ptr;
};

#endif

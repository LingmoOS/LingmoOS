/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KPAGEMODEL_P_H
#define KPAGEMODEL_P_H

#include "kpagemodel.h"

class KPageModelPrivate
{
    Q_DECLARE_PUBLIC(KPageModel)
public:
    virtual ~KPageModelPrivate();

protected:
    KPageModel *q_ptr;
};

#endif // KPAGEMODEL_P_H

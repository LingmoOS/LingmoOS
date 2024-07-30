/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPAGEDIALOGTEST_H
#define KPAGEDIALOGTEST_H

#include "kpagedialog.h"

class KPageDialogTest : public KPageDialog
{
    Q_OBJECT

public:
    KPageDialogTest(QWidget *parent = nullptr);
    ~KPageDialogTest() override;
};

#endif

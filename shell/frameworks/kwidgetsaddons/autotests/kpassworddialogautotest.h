/*
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPASSWORDDIALOGAUTOTEST_H
#define KPASSWORDDIALOGAUTOTEST_H

#include <QObject>

class KPasswordDialogAutotest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void shouldNotHideVisibilityActionInPlaintextMode();
};

#endif

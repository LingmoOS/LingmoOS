/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NEWKEY_H
#define NEWKEY_H

#include "ui_newkey.h"

class newKey : public QWidget, public Ui_newKey
{
    Q_OBJECT

public:
    explicit newKey(QWidget* parent = nullptr);


public Q_SLOTS:
   void CBsave_toggled(bool);
};

#endif

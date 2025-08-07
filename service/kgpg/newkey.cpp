/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newkey.h"

newKey::newKey(QWidget* parent)
      : QWidget(parent), Ui_newKey()
{
    setupUi(this);
    connect(CBsave, &QCheckBox::toggled, this, &newKey::CBsave_toggled);
}

void newKey::CBsave_toggled(bool isOn)
{
    kURLRequester1->setEnabled(isOn);
}

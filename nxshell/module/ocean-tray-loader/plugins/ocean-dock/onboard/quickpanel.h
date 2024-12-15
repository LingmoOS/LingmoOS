// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUICKPANEL_H
#define QUICKPANEL_H

#include "singlequickpanel.h"

#include <QWidget>

class QuickPanel : public SignalQuickPanel
{
    Q_OBJECT
public:
    QuickPanel(QWidget *parent = nullptr);
    ~QuickPanel();

private:

};

#endif
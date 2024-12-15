// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "../../widgets/progresswidget.h"
#define ICON_SIZE 128

class UpgradeProgressWidget: public ProgressWidget
{
    Q_OBJECT
public:
    UpgradeProgressWidget(QWidget *parent);

Q_SIGNALS:
    void start();
    void done();

protected:
    void initUI();
    void initConnections();

};

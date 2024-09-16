// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DSuggestButton>

#include "../../widgets/progresswidget.h"
#define ICON_SIZE 128

class MigrateProgressWidget: public ProgressWidget
{
    Q_OBJECT
public:
    MigrateProgressWidget(QWidget *parent);

Q_SIGNALS:
    void start();
    void done();
    void networkError();

protected:
    void initUI();
    void initConnections();

private:
    DSuggestButton *m_reconnectButton;
};

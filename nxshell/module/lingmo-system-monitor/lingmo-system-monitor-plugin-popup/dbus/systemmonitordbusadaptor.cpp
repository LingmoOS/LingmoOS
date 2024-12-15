// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemmonitordbusadaptor.h"


SystemMonitorDBusAdaptor::SystemMonitorDBusAdaptor(QObject *parent) : QObject(parent)
{

}

SystemMonitorDBusAdaptor::~SystemMonitorDBusAdaptor()
{

}

void SystemMonitorDBusAdaptor::slotShowOrHideSystemMonitorPluginPopupWidget()
{
    emit sigSendShowOrHideSystemMonitorPluginPopupWidget();
}


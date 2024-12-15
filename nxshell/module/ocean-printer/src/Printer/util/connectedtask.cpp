// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "connectedtask.h"

ConnectedTask::ConnectedTask(const QString &name, QObject *parent)
    : QThread(parent)
    , m_printerName(name)
{
    m_pCheckCon = nullptr;
}

ConnectedTask::~ConnectedTask()
{
}

void ConnectedTask::run()
{
    m_pCheckCon = new CheckConnected(m_printerName);
    bool passed = m_pCheckCon->isPass();
    delete m_pCheckCon;
    //用printerName区分信号的来源
    emit signalResult(passed, m_printerName);
}

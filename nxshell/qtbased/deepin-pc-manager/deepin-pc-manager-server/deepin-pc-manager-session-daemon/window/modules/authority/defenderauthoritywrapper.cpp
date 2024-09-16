// Copyright (C) 2022 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderauthoritywrapper.h"
#include "defenderauthority.h"

DefenderAuthorityWrapper::DefenderAuthorityWrapper(QObject *parent)
    : QObject(parent)
{
    m_authority = new DefenderAuthority;

    if (m_authority) {
        initConnection();
        m_authority->moveToThread(&m_authThread);
        m_authThread.start();
    }
}

DefenderAuthorityWrapper::~DefenderAuthorityWrapper()
{
    if (m_authority) {
        m_authority->deleteLater();
        m_authority = nullptr;
    }
    m_authThread.quit();
    m_authThread.wait(1000);
}

// 只处理信号转发
void DefenderAuthorityWrapper::initConnection()
{
    connect(m_authority, &DefenderAuthority::notifyAuthStarted, this, &DefenderAuthorityWrapper::notifyAuthStarted);
    connect(m_authority, &DefenderAuthority::notifyAuthFinished, this, &DefenderAuthorityWrapper::notifyAuthFinished);
    connect(m_authority, &DefenderAuthority::notifyAuthResult, this, &DefenderAuthorityWrapper::notifyAuthResult);

    connect(this, &DefenderAuthorityWrapper::onModulesActived, m_authority, &DefenderAuthority::onModulesActived);
    connect(this, &DefenderAuthorityWrapper::onAuthRequest, m_authority, &DefenderAuthority::checkAuthorization);
}

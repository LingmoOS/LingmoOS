// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbus_adpator.h"

ApplicationAdaptor::ApplicationAdaptor(CMainWindow *mw)
    : QDBusAbstractAdaptor(mw), m_mw(mw)
{
}

void ApplicationAdaptor::Raise()
{
    qDebug() << "raise window from dbus";
    m_mw->showNormal();
    m_mw->raise();
    m_mw->activateWindow();
}

ApplicationAdaptor::~ApplicationAdaptor()
{
    if (m_mw) {
        m_mw->deleteLater();
        m_mw = nullptr;
    }
}

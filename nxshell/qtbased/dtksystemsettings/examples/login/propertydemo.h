// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "demo.h"
#include "dloginmanager.h"
#include "dloginsession.h"
DLOGIN_USE_NAMESPACE

class PropertyDemo : public LoginDemo
{
    Q_OBJECT
public:
    PropertyDemo(QObject *parent = nullptr);
    int run() override;

private:
    DLoginManager *m_manager;
    QSharedPointer<DLoginSession> m_currentSession;
};

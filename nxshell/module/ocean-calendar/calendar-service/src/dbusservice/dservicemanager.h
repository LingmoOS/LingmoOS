// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include "daccountmanagerservice.h"

/**
 * @brief The DServiceManager class  服务管理类
 */
class DServiceManager : public QObject
{
    Q_OBJECT
public:
    explicit DServiceManager(QObject *parent = nullptr);

    void updateRemindJob();

signals:

public slots:
private:
    DAccountManagerService *m_accountManagerService = nullptr;
};

#endif // SERVICEMANAGER_H

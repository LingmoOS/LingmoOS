// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSOPENTHREAD_H
#define PROCESSOPENTHREAD_H

#include "archiveinterface.h"
#include "plugin.h"

#include <QThread>

class ProcessOpenThread : public QThread
{
    Q_OBJECT
public:
    explicit ProcessOpenThread(QObject *parent = nullptr);

    /**
     * @brief setProgramPath    设置应用程序
     * @param strProgramPath    应用程序路径
     */
    void setProgramPath(const QString &strProgramPath);

    /**
     * @brief setArguments  设置参数
     * @param listArguments 启动参数
     */
    void setArguments(const QStringList &listArguments);

protected:
    void run() override;

private:
    QString m_strProgramPath;
    QStringList m_listArguments;
};


#endif  // UITOOLS_H

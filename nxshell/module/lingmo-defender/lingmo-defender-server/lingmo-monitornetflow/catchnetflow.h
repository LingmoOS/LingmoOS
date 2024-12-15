// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CATCHNETFLOW_H
#define CATCHNETFLOW_H

#include <QThread>
#include "writedbusdata.h"

class CatchNetFlow : public QThread
{
    Q_OBJECT
public:
    explicit CatchNetFlow(int argc, char *argv[], WriteDBusData *dbusData, QObject *parent = nullptr);
    ~CatchNetFlow() override;

    virtual void run() override;

signals:

public slots:

private:
    WriteDBusData *m_dbusData;
    int m_argc;
    char **m_argv;
};

#endif // CATCHNETFLOW_H

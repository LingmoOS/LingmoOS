// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSCONTROLLER_H
#define PROCESSCONTROLLER_H

#include "common/error_context.h"

#include <QObject>

class QProcess;

/**
 * @brief Proxy class to execute pkexec & kill as another user
 */
class ProcessController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Process controller constructor
     * @param pid Process to send signal to
     * @param signal Signal to send
     * @param parent Parent object
     */
    explicit ProcessController(pid_t pid, int signal, QObject *parent = nullptr);

    /**
     * @brief Execute pkexec in another process
     */
    void execute();

Q_SIGNALS:
    /**
     * @brief Process execute result ready signal
     * @param code Return code of the finished process
     */
    void resultReady(int code);
    /**
     * @brief Process finished signal
     */
    void finished();

private:
    // Process id to send signal to
    pid_t m_pid;
    // Signal to send
    int m_signal {0};

    // Process to run another executable binary
    QProcess *m_proc;
};

#endif  // PROCESSCONTROLLER_H

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "launcher.h"

QVariantMap LauncherCall::call()
{
    QScopedPointer<QProcess> process(new QProcess);
    process->setProgram(m_program);
    process->setArguments(m_arguments);
    process->setWorkingDirectory(m_workingDir);
    process->start();
    process->waitForFinished(m_timeout.value_or(-1));

    QVariantMap map;
    map["allStandardOutput"] = process->readAllStandardOutput();
    map["allStandardError"] = process->readAllStandardError();

    return map;
}

void LauncherCall::asyncCall(const QJSValue &jsCallback)
{
    QProcess *process = new QProcess;

    auto result = [=] {
        QJSValue callback = jsCallback;
        if (callback.isCallable()) {
            QVariantMap map;
            map["allStandardOutput"] = process->readAllStandardOutput();
            map["allStandardError"] = process->readAllStandardError();
            QJSValue value = jsCallback.engine()->toScriptValue<QVariantMap>(map);
            callback.call({value});
        }
        process->deleteLater();
    };

    connect(process,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            process,
            result);

    process->setProgram(m_program);
    process->setArguments(m_arguments);
    process->setWorkingDirectory(m_workingDir);
    process->start();

    if (m_timeout.has_value()) {
        QTimer::singleShot(m_timeout.value(), process, [=] {
            process->kill();
            process->deleteLater();
            result();
        });
    }
}

void LauncherCall::startDetached()
{
    QProcess::startDetached(m_program, m_arguments, m_workingDir);
}

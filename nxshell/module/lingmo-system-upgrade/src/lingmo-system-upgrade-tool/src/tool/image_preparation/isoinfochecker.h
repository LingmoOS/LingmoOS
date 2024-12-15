// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QProcess>

class IsoInfoChecker : public QObject
{
    Q_OBJECT
public:
    explicit IsoInfoChecker(QObject *parent = nullptr);
    void integrityCheck(const QString isoPath);
    void retrieveVersionValue(const QString isoPath, const QString key);
    bool isRunning() { return m_checkIsoProcess != nullptr && m_checkIsoProcess->state() == QProcess::Running; }
    void stop() { if (isRunning()) m_checkIsoProcess->terminate(); }

signals:
    void Stdout(const QString stdout);
    void ExitStatus(int status);

private slots:
    void onCheckDone();

private:
    QProcess*   m_checkIsoProcess;
    QStringList m_args;
};

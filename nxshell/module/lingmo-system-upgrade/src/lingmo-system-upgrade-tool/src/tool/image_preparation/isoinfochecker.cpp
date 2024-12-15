// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDebug>
#include <QFile>
#include <QProcess>

#include "isoinfochecker.h"

IsoInfoChecker::IsoInfoChecker(QObject *parent)
    : QObject(parent)
    , m_checkIsoProcess(new QProcess(this))
{
    m_args << "-c";

    QFile checkScriptFile(":/check_iso_info.sh");
    if (!checkScriptFile.open(QFile::ReadOnly | QFile::Text))
    {
        emit ExitStatus(1);
    }
    else
    {
        m_args << QString::fromLatin1(checkScriptFile.readAll());
    }
    checkScriptFile.close();

    connect(m_checkIsoProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &IsoInfoChecker::ExitStatus);
    connect(m_checkIsoProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &IsoInfoChecker::onCheckDone);
    connect(m_checkIsoProcess, &QProcess::readyReadStandardError, this, [this] {
        qDebug() << "IsoInfoChecker STDERR:" << QString::fromLatin1(m_checkIsoProcess->readAllStandardError());
    });
}

void IsoInfoChecker::integrityCheck(const QString isoPath)
{
    // bash -c时会将命令后的第一个参数放到$0，所以需要加个占位参数
    m_args << "arg_placeholder" << isoPath << "-c";
    m_checkIsoProcess->start("/bin/bash", m_args);
}

void IsoInfoChecker::retrieveVersionValue(const QString isoPath, const QString key)
{
    // bash -c时会将命令后的第一个参数放到$0，所以需要加个占位参数
    m_args << "arg_placeholder" << isoPath << key;
    m_checkIsoProcess->start("/bin/bash", m_args);
}

void IsoInfoChecker::onCheckDone()
{
    emit Stdout(QString::fromLatin1(m_checkIsoProcess->readAllStandardOutput()));
}

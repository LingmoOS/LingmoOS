// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_NAME_H
#define PROCESS_NAME_H

#include <QString>
#include <QByteArrayList>

#include <memory>

namespace core {
namespace process {

class Process;

class ProcessName
{
public:
    explicit ProcessName();
    ~ProcessName() = default;

    void refreashProcessName(Process *proc);

    QString name() const;
    QString displayName() const;

    static QString normalizeProcessName(const QString &name, const QByteArrayList &cmdline);

private:
    QString getDisplayName(Process *proc);

private:
    QString m_name {};
    QString m_displayName {};
};

inline QString ProcessName::name() const
{
    return m_name;
}

inline QString ProcessName::displayName() const
{
    return m_displayName;
}

} // namespace process
} // namespace core

#endif // PROCESS_NAME_H

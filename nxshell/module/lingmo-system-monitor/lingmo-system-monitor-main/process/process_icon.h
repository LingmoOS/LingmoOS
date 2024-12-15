// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_ICON_H
#define PROCESS_ICON_H

#include <QIcon>

#include <memory>

namespace core {
namespace process {

class Process;
struct icon_data_t;

class ProcessIcon
{
public:
    explicit ProcessIcon();
    ~ProcessIcon();

    QIcon icon() const;
    void refreashProcessIcon(Process *proc);

private:
    std::shared_ptr<struct icon_data_t> getIcon(Process *proc);
    struct icon_data_t *defaultIconData(const QString &procname) const;
    struct icon_data_t *terminalIconData(const QString &procname) const;
    QString getFileManagerString();

private:
    std::shared_ptr<struct icon_data_t> m_data;
};

} // namespace process
} // namespace core

#endif // PROCESS_ICON_H

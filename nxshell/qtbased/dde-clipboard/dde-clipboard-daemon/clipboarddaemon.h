// Copyright (C) 2020 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARD_DAEMON_H
#define CLIPBOARD_DAEMON_H

#include <QObject>

class ClipboardDaemon : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardDaemon(QObject *parent = nullptr);
};

#endif //CLIPBOARD_DAEMON_H

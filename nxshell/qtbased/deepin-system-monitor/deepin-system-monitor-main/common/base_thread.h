// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASE_THREAD_H
#define BASE_THREAD_H

#include <QThread>

namespace common {
namespace core {

class BaseThread : public QThread
{
    Q_OBJECT

public:
    enum thread_key_t {
        kUnknowThread,
        kSystemMonitorThread,
        kNetifMonitorThread,

        kUserThread = 0xff
    };

    virtual int threadKey() const { return kUnknowThread; }

    explicit BaseThread(QObject *parent = nullptr)
        : QThread(parent) {}

    virtual ~BaseThread() {}
};

} // namespace core
} // namespace common

#endif // BASE_THREAD_H

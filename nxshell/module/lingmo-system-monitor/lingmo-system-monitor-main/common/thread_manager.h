// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <QThread>
#include <QMap>

namespace common {
namespace core {

/**
 * @brief Thread Manager class to keep tracking of long running threads
 */
class BaseThread;
class ThreadManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ThreadManager)

public:
    static ThreadManager *instance();

    // TODO: no safe to return raw pointer here, since threads can quit by itself while being used in other threads
    // need a better way, eg: add a read/write lock & reference counter in base thread implemention...
    template<typename T>
    T *thread(int key);

public slots:
    void attach(BaseThread *thread);

public:
    explicit ThreadManager() {}
    virtual ~ThreadManager() {}

private:
    QMap<int, BaseThread *> m_threadDB;
};

template<typename T>
inline T *ThreadManager::thread(int key)
{
    auto *thr = m_threadDB[key];
    return static_cast<T *>(thr);
}

} // namespace core
} // namespace common

#endif // THREAD_MANAGER_H

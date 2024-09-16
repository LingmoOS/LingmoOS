// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAtomicPointer>
#include <QReadWriteLock>
#include <QMutex>
#include <QDebug>

template<class T>

class Singleton
{
public:
    static T &getInstance(void);

private:
    Singleton();
    Singleton(const Singleton<T> &);
    Singleton<T> &operator=(const Singleton<T> &);

    QReadWriteLock internalMutex;
    static QMutex mutex;
    static QAtomicPointer<T> m_pInstance;
};

template<class T>
QAtomicPointer<T> Singleton<T>::m_pInstance;

template<class T>
QMutex Singleton<T>::mutex(QMutex::Recursive);

template<class T>
T &Singleton<T>::getInstance(void)
{
    QMutexLocker locker(&mutex);
    if (m_pInstance.testAndSetOrdered(0, 0)) {
        m_pInstance.testAndSetOrdered(0, new T);
    }

    return *m_pInstance.load();
}

#define DEFENDER_MUTEX_LOCKER(mutex_ptr) QMutexLocker locker(mutex_ptr)

class CMutex : public QObject
{
    Q_OBJECT
public:
    explicit CMutex(QObject *parent = nullptr);
    QMutex m_mutex;
};
typedef Singleton<CMutex> SMutex;

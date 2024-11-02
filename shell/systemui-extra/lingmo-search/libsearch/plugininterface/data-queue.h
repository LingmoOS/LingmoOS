/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#ifndef DATAQUEUE_H
#define DATAQUEUE_H
#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include "libsearch_global.h"
namespace LingmoUISearch {
// TODO I want a unlocked queue
template <typename T>
class LIBSEARCH_EXPORT DataQueue : protected QList<T>
{
public:
    inline void enqueue(const T &t) {
        QMutexLocker locker(&m_mutex);
        QList<T>::append(t);
    }
    inline T dequeue() {
        QMutexLocker locker(&m_mutex);
        return QList<T>::takeFirst();
    }
    inline void clear() {
        QMutexLocker locker(&m_mutex);
        QList<T>::clear();
        QList<T>::reserve(QList<T>::size());
    }
    inline bool isEmpty() {
        QMutexLocker locker(&m_mutex);
        return QList<T>::isEmpty();
    }
    inline T tryDequeue() {
        QMutexLocker locker(&m_mutex);
        if(QList<T>::isEmpty()) {
            return T();
        } else {
            return QList<T>::takeFirst();
        }
    }
private:
    QMutex m_mutex;
};
}

#endif // DATAQUEUE_H

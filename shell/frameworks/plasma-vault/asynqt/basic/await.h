/*
 *   SPDX-FileCopyrightText: 2017 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_AWAIT_H
#define ASYNQT_AWAIT_H

#include <QCoreApplication>

namespace AsynQt
{
template<typename T>
T await(const QFuture<T> &future)
{
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }

    return future.result();
}

inline void await(const QFuture<void> &future)
{
    while (!future.isFinished()) {
        QCoreApplication::processEvents();
    }
}

} // namespace AsynQt

#endif // ASYNQT_AWAIT_H

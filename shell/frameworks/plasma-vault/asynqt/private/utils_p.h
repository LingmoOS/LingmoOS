/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_PRIVATE_UTILS_P_H
#define ASYNQT_PRIVATE_UTILS_P_H

#include <QFuture>
#include <QFutureWatcher>
#include <memory>

namespace AsynQt
{
namespace detail
{
#define IMPLEMENT_FUTURE_WATCHER_HANDLER(SignalName, EventName)                                                                                                \
    template<typename T, typename Function>                                                                                                                    \
    inline void EventName(const std::unique_ptr<QFutureWatcher<T>> &watcher, Function function)                                                                \
    {                                                                                                                                                          \
        QObject::connect(watcher.get(), &QFutureWatcherBase::SignalName, watcher.get(), function, Qt::QueuedConnection);                                       \
    }                                                                                                                                                          \
                                                                                                                                                               \
    template<typename T, typename Function>                                                                                                                    \
    inline void EventName(const QFutureWatcher<T> &watcher, Function function)                                                                                 \
    {                                                                                                                                                          \
        QObject::connect(&watcher, &QFutureWatcherBase::SignalName, &watcher, function, Qt::QueuedConnection);                                                 \
    }

IMPLEMENT_FUTURE_WATCHER_HANDLER(finished, onFinished)
IMPLEMENT_FUTURE_WATCHER_HANDLER(canceled, onCanceled)
IMPLEMENT_FUTURE_WATCHER_HANDLER(resultReadyAt, onResultReadyAt)

template<typename Function>
inline void onResultReadyAt(const std::unique_ptr<QFutureWatcher<void>> &watcher, Function function)
{
    Q_UNUSED(watcher)
    Q_UNUSED(function)
}

#undef IMPLEMENT_FUTURE_WATCHER_HANDLER

} // namespace detail
} // namespace AsynQt

#endif // ASYNQT_PRIVATE_UTILS_P_H

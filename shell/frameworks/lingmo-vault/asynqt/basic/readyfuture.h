/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_CONS_READY_FUTURE_H
#define ASYNQT_CONS_READY_FUTURE_H

#include <QFuture>
#include <QFutureInterface>
#include <QObject>
#include <QTimer>

#include "../private/basic/readyfuture_p.h"

namespace AsynQt
{
/**
 * Creates a future that has already been completed,
 * and that contains the specified value
 */
template<typename _Result>
QFuture<typename std::decay<_Result>::type> makeReadyFuture(_Result &&value)
{
    return detail::makeReadyFuture(std::forward<_Result>(value));
}

/**
 * Creates a void future that has already been completed.
 */
inline QFuture<void> makeReadyFuture()
{
    return detail::makeReadyFuture();
}

} // namespace AsynQt

#endif // ASYNQT_CONS_READY_FUTURE_H

/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_CONS_CANCELED_FUTURE_H
#define ASYNQT_CONS_CANCELED_FUTURE_H

#include <QFuture>
#include <QFutureInterface>
#include <QObject>
#include <QTimer>

#ifndef QT_NO_EXCEPTIONS
#include <QException>
#endif

#include "../private/basic/canceledfuture_p.h"

namespace AsynQt
{
/**
 * Creates a canceled future.
 */
template<typename _Result = void>
QFuture<_Result> makeCanceledFuture()
{
    // No need to decay the type, expecting the user not to try
    // and make a future of ref-to-type or anything that funny
    return detail::makeCanceledFuture<_Result>();
}

#ifndef QT_NO_EXCEPTIONS
/**
 * Creates a canceled future.
 */
template<typename _Result = void>
QFuture<_Result> makeCanceledFuture(const QException &exception)
{
    // No need to decay the type, expecting the user not to try
    // and make a future of ref-to-type or anything that funny
    return detail::makeCanceledFuture<_Result>(exception);
}

#ifdef ENABLE_EVIL_QFUTURE_HACKS_THAT_SHOULD_BE_IN_QT

namespace evil
{
// TODO: Remove these

template<typename T>
bool hasException(const QFuture<T> &future)
{
    return reinterpret_cast<const QFuture<AsynQt_QFuturePrivacyHack_hasException> *>(&future)->isCanceled();
}

template<typename T>
void throwPossibleException(QFuture<T> &future)
{
    reinterpret_cast<QFuture<AsynQt_QFuturePrivacyHack_throwPossibleException> *>(&future)->cancel();
}

} // namespace evil

#endif

#else
#warning "Exceptions are disabled. If you enable them, you'll open a whole new world"
#endif

} // namespace AsynQt

#endif // ASYNQT_CONS_CANCELED_FUTURE_H

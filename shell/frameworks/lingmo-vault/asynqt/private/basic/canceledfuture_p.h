/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

//
// W A R N I N G
// -------------
//
// This file is not part of the AsynQt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace AsynQt
{
namespace detail
{
template<typename _Result>
QFuture<_Result> makeCanceledFuture()
{
    QFutureInterface<_Result> interface;
    auto future = interface.future();

    interface.reportStarted();
    interface.reportCanceled();
    interface.reportFinished();

    return future;
}

#ifndef QT_NO_EXCEPTIONS
template<typename _Result>
QFuture<_Result> makeCanceledFuture(const QException &exception)
{
    QFutureInterface<_Result> interface;
    auto future = interface.future();

    interface.reportStarted();
    interface.reportException(exception);
    interface.reportFinished();

    return future;
}
#endif

} // namespace detail
} // namespace AsynQt

#ifdef ENABLE_EVIL_QFUTURE_HACKS_THAT_SHOULD_BE_IN_QT

class AsynQt_QFuturePrivacyHack_hasException;
template<>
inline bool QFuture<AsynQt_QFuturePrivacyHack_hasException>::isCanceled() const
{
    return d.exceptionStore().hasException();
}

class AsynQt_QFuturePrivacyHack_throwPossibleException;
template<>
inline void QFuture<AsynQt_QFuturePrivacyHack_throwPossibleException>::cancel()
{
    return d.exceptionStore().throwPossibleException();
}

#endif

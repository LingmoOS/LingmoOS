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
QFuture<typename std::decay<_Result>::type> makeReadyFuture(_Result &&value)
{
    QFutureInterface<_Result> interface;
    auto future = interface.future();

    interface.reportStarted();
    interface.reportResult(std::forward<_Result>(value));
    interface.reportFinished();

    return future;
}

inline QFuture<void> makeReadyFuture()
{
    QFutureInterface<void> interface;
    auto future = interface.future();

    interface.reportStarted();
    interface.reportFinished();

    return future;
}

} // namespace detail
} // namespace AsynQt

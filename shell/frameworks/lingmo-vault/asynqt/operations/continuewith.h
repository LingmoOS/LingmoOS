/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_BASE_CONTINUEWITH_H
#define ASYNQT_BASE_CONTINUEWITH_H

#include <QFuture>
#include <QFutureWatcher>

#include <memory>
#include <type_traits>

#include "flatten.h"
#include "transform.h"

namespace AsynQt
{
/**
 * This method is similar to `transform`,
 * but it takes a transformation function (in this
 * case, called a continuation) that does not
 * return a normal value, but also a new future.
 * It returns the future that will be returned
 * by the continuation.
 *
 * It is equivalent to calling `flatten` on the result
 * of the `transform` function when a continuation
 * is passed to it.
 *
 * Example:
 *
 * <code>
 *     QFuture<QString> input = getUserInput();
 *     QFuture<Status> result = continueWith(input, [] (QString message) {
 *          return server.send(message);
 *     });
 * </code>
 *
 * @arg future the future to connect the continuation to
 * @arg continuation the continuation function
 * @returns the future that the continuation will return
 */
template<typename _In, typename _Continuation>
auto continueWith(const QFuture<_In> &future, _Continuation &&continuation) -> decltype(flatten(transform(future, std::forward<_Continuation>(continuation))))
{
    return flatten(transform(future, std::forward<_Continuation>(continuation)));
}

namespace operators
{
template<typename _In, typename _Continuation>
auto operator|(const QFuture<_In> &future, _Continuation &&continuation) -> decltype(continueWith(future, continuation))
{
    return continueWith(future, continuation);
}

} // namespace operators

} // namespace AsynQt

#endif // ASYNQT_BASE_CONTINUEWITH_H

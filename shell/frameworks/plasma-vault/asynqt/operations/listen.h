/*
 *   SPDX-FileCopyrightText: 2017 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_BASE_LISTEN_H
#define ASYNQT_BASE_LISTEN_H

#include <QFuture>
#include <QFutureWatcher>

#include "../private/operations/listen_p.h"

namespace AsynQt
{
/**
 *
 */
template<typename _Result, typename _Function>
QFuture<_Result> onFinished(const QFuture<_Result> &future, _Function &&function)
{
    return detail::onFinished_impl(future, std::forward<_Function>(function));
}

/**
 *
 */
template<typename _Result, typename _Function>
QFuture<_Result> onSuccess(const QFuture<_Result> &future, _Function &&function)
{
    return onFinished(future, detail::PassResult<_Function>(function));
}

/**
 *
 */
template<typename _Result, typename _Function>
QFuture<_Result> onError(const QFuture<_Result> &future, _Function &&function)
{
    return onFinished(future, detail::PassError<_Function>(function));
}

namespace operators
{
template<typename _Function>
inline detail::operators::OnFinishedModifier<_Function> onFinished(_Function &&function)
{
    return detail::operators::OnFinishedModifier<_Function>(std::forward<_Function>(function));
}

template<typename _Function>
inline detail::operators::OnFinishedModifier<detail::PassResult<_Function>> onSuccess(_Function &&function)
{
    return detail::operators::OnFinishedModifier<detail::PassResult<_Function>>(detail::PassResult<_Function>(function));
}

template<typename _Function>
inline detail::operators::OnFinishedModifier<detail::PassError<_Function>> onError(_Function &&function)
{
    return detail::operators::OnFinishedModifier<detail::PassError<_Function>>(detail::PassError<_Function>(function));
}

} // namespace operators

} // namespace AsynQt

#endif // include guard

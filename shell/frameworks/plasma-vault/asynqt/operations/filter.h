/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_BASE_FILTER_H
#define ASYNQT_BASE_FILTER_H

#include <QFuture>
#include <QFutureWatcher>

#include <memory>
#include <type_traits>

#include "../private/operations/filter_p.h"

namespace AsynQt
{
/**
 * Takes a future of a future, and flattens it out.
 *
 * If any of the futures is canceled, the resulting future
 * will be canceled as well.
 *
 * @arg future future that contains another future of type T
 * @returns a single-level future of type T
 */
template<typename _Result, typename _Predicate>
QFuture<_Result> filter(const QFuture<_Result> &future, _Predicate &&predicate)
{
    return detail::filter_impl(future, std::forward<_Predicate>(predicate));
}

namespace operators
{
template<typename _Predicate>
inline detail::operators::FilterModifier<_Predicate> filter(_Predicate &&predicate)
{
    return detail::operators::FilterModifier<_Predicate>(std::forward<_Predicate>(predicate));
}

} // namespace operators

} // namespace AsynQt

#endif // ASYNQT_BASE_FILTER_H

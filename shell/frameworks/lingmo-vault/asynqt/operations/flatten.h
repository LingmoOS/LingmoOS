/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_BASE_FLATTEN_H
#define ASYNQT_BASE_FLATTEN_H

#include <QFuture>
#include <QFutureWatcher>

#include <memory>
#include <type_traits>

#include "../private/operations/flatten_p.h"

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
template<typename _Result>
QFuture<_Result> flatten(const QFuture<QFuture<_Result>> &future)
{
    return detail::flatten_impl(future);
}

namespace operators
{
inline detail::operators::FlattenModifier flatten()
{
    return detail::operators::FlattenModifier();
}

} // namespace operators

} // namespace AsynQt

#endif // ASYNQT_BASE_FLATTEN_H

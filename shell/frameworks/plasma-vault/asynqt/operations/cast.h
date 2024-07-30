/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_BASE_CAST_H
#define ASYNQT_BASE_CAST_H

#include <QFuture>
#include <QFutureWatcher>

#include <memory>
#include <type_traits>

#include "../private/operations/cast_p.h"
#include "transform.h"

namespace AsynQt
{
/**
 * Casts the future result into the specified type.
 *
 * <code>
 *     auto future = AsynQt::Process::getOutput("echo", { "Hello KDE" });
 *     auto castFuture = AsynQt::qfuture_cast<QString>(future);
 * </code>
 */
template<typename _Out, typename _In>
QFuture<_Out> qfuture_cast(const QFuture<_In> &future)
{
    return detail::qfuture_cast_impl<_Out>(future);
}

namespace operators
{
template<typename _Out>
detail::operators::CastModifier<_Out> cast()
{
    return detail::operators::CastModifier<_Out>();
}

} // namespace operator

} // namespace AsynQt

#endif // ASYNQT_BASE_CAST_H

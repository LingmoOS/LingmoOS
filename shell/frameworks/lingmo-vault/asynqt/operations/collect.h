/*
 *   SPDX-FileCopyrightText: 2017 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_BASE_COLLECT_H
#define ASYNQT_BASE_COLLECT_H

#include <tuple>

#include "../private/operations/collect_p.h"

namespace AsynQt
{
template<typename... _Results>
QFuture<std::tuple<_Results...>> collect(const QFuture<_Results> &...futures)
{
    return detail::collect_impl(futures...);
}

} // namespace AsynQt

#endif // ASYNQT_COLLECT_H

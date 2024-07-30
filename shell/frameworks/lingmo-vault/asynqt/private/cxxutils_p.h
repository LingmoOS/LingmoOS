/*
 *   SPDX-FileCopyrightText: 2017 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_CXXUTILS_P_H
#define ASYNQT_CXXUTILS_P_H

namespace AsynQt
{
namespace detail
{
// Some things we need to wait for... c++17

template<int... Indices>
struct index_sequence {
};

template<int N, int... Indices>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Indices...> {
};

template<int... Indices>
struct make_index_sequence<0, Indices...> : index_sequence<Indices...> {
};

} // namespace detail
} // namespace AsynQt

#endif // ASYNQT_CXXUTILS_P_H

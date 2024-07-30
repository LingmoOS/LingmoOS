/*
 *   SPDX-FileCopyrightText: 2017 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_EITHER_H
#define ASYNQT_EITHER_H

#include "expected.h"

namespace AsynQt
{
template<typename T1, typename T2>
class Either : private Expected<T1, T2>
{
public:
    template<typename... ConsParams>
    static Either left(ConsParams &&...params)
    {
        return Expected<T1, T2>::success(std::forward<ConsParams>(params)...);
    }

    template<typename... ConsParams>
    static Either right(ConsParams &&...params)
    {
        return Expected<T1, T2>::error(std::forward<ConsParams>(params)...);
    }

    Either(T1 value)
    {
        auto tmp = Either::left(value);
        this->swap(tmp);
    }

    Either(T2 value)
    {
        auto tmp = Either::right(value);
        this->swap(tmp);
    }

    T1 &left()
    {
        return Expected<T1, T2>::get();
    }

    T1 left() const
    {
        return Expected<T1, T2>::get();
    }

    T2 &right()
    {
        return Expected<T1, T2>::error();
    }

    T2 right() const
    {
        return Expected<T1, T2>::error();
    }

    Either(const Either &other) = default;
    Either(Either &&other) = default;

    Either &operator=(const Either &other) = default;
    Either &operator=(Either &&other) = default;
};

} // namespace AsynQt

#endif // ASYNQT_EITHER_H

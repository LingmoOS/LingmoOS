/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEBUG_AND_RETURN_H
#define DEBUG_AND_RETURN_H

#ifdef QT_DEBUG
#include <QDebug>
#endif

namespace kamd
{
namespace utils
{
template<typename T>
T debug_and_return(bool debug, const char *message, T &&value)
{
    if (debug) {
        qDebug().noquote() << message << " " << value;
    }

    return std::forward<T>(value);
}

} // namespace utils
} // namespace kamd

#endif // DEBUG_AND_RETURN_H

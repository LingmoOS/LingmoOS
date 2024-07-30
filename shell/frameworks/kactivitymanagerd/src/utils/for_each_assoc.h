/*
 *   SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <kactivities-features.h>

/********************************************************************
 *  Associative container's for_each (for hash, map, and similar )  *
 ********************************************************************/

namespace kamd
{
namespace utils
{
namespace details
{ //_

// Iterator Functions

template<typename Iterator, typename Function>
Function qt_for_each_assoc(Iterator start, Iterator end, Function f)
{
    for (; start != end; ++start)
        f(start.key(), start.value());

    return f;
}

template<typename Iterator, typename Function>
Function stl_for_each_assoc(Iterator start, Iterator end, Function f)
{
    for (; start != end; ++start)
        f(start->first, start->second);

    return f;
}

// Container functions

template<typename Container, typename Function>
Function _for_each_assoc_helper_container(const Container &c, Function f, decltype(&Container::constBegin) *)
{
    // STL will never have methods with camelCase :)
    return qt_for_each_assoc(c.constBegin(), c.constEnd(), f);
}

template<typename Container, typename Function, typename Default>
Function _for_each_assoc_helper_container(const Container &c, Function f, Default *)
{
    return stl_for_each_assoc(c.cbegin(), c.cend(), f);
}

} //^ namespace details

template<typename Container, typename Function>
Function for_each_assoc(const Container &c, Function f)
{
    return details::_for_each_assoc_helper_container<Container, Function>(c, f, nullptr);
}

} // namespace utils
} // namespace kamd

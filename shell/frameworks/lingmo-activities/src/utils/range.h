/*
    SPDX-FileCopyrightText: 2012 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef UTILS_RANGE_H
#define UTILS_RANGE_H

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>

/********************************************************************
 *  Syntactic sugar for converting ranges to collections            *
 ********************************************************************/

namespace kamd
{
namespace utils
{
template<typename Collection, typename Range>
__inline Collection as_collection(Range range)
{
    Collection result;

    boost::copy(range, std::back_inserter(result));

    return result;
}

template<typename Member, typename... Args>
__inline auto transformed(Member member, Args... args) -> decltype(boost::adaptors::transformed(std::bind(member, args..., std::placeholders::_1)))
{
    return boost::adaptors::transformed(std::bind(member, args..., std::placeholders::_1));
}

template<typename Member, typename... Args>
__inline auto filtered(Member member, Args... args) -> decltype(boost::adaptors::filtered(std::bind(member, args..., std::placeholders::_1)))
{
    return boost::adaptors::filtered(std::bind(member, args..., std::placeholders::_1));
}

template<typename Class, typename Member>
__inline auto filtered(Class *const self, Member member) -> decltype(boost::adaptors::filtered(std::bind(member, self, std::placeholders::_1)))
{
    return boost::adaptors::filtered(std::bind(member, self, std::placeholders::_1));
}

} // namespace utils
} // namespace kamd

#endif // UTILS_RANGE_H

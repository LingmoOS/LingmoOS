/*
    SPDX-FileCopyrightText: 2012 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef UTILS_REMOVE_IF_H
#define UTILS_REMOVE_IF_H

#include <algorithm>

/********************************************************************
 *  Syntactic sugar for the erase-remove idiom                      *
 ********************************************************************/

namespace kamd
{
namespace utils
{
template<typename Collection, typename Filter>
__inline void remove_if(Collection &collection, Filter filter)
{
    collection.erase(std::remove_if(collection.begin(), collection.end(), filter), collection.end());
}

} // namespace utils
} // namespace kamd

#endif // UTILS_REMOVE_IF_H

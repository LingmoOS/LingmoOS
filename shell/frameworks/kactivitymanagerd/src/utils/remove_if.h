/*
 *   SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <algorithm>
#include <kactivities-features.h>

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

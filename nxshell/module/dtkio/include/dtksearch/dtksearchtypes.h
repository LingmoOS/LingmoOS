// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTKSEARCHTYPES_H
#define DTKSEARCHTYPES_H

#include <functional>

#include <QFlags>
#include <QMetaType>

#include "dtksearch_global.h"

DSEARCH_BEGIN_NAMESPACE

using ResultFilterFunc = std::function<bool (const QString &)>;

//!<@~english The search flags
enum class SearchFlag : uint8_t {
    FileName = 0x01,    //!<@~english Search for the filename.
    PinYin = 0x02 | FileName,   //!<@~english Search for the pinyin of the filename, and also recall the filename results that match the search keyword.
    FullText = 0x04,    //!<@~english Search for the file contents.
    AllEntries = PinYin | FullText  //!<@~english Search for filename, pinyin of filename and file contents.
};
Q_DECLARE_FLAGS(SearchFlags, SearchFlag)

DSEARCH_END_NAMESPACE

Q_DECLARE_OPERATORS_FOR_FLAGS(DSEARCH_NAMESPACE::SearchFlags);
Q_DECLARE_METATYPE(DSEARCH_NAMESPACE::SearchFlags)
Q_DECLARE_METATYPE(DSEARCH_NAMESPACE::ResultFilterFunc)

#endif // DTKSEARCHTYPES_H

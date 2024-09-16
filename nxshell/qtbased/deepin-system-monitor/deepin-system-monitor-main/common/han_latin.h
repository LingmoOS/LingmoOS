// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HAN_LATIN_H
#define HAN_LATIN_H

class QString;

/**
* @brief namespace util::common
*/
namespace util {
namespace common {

/**
* @brief convHanToLatin Convert chinese hanzi to ascii pinyin format
* @param words Chinese hanzi unicode string
* @return Converted ascii pinyin string
*/
QString convHanToLatin(const QString &words);

} // namespace common
} // namespace util

#endif  // HAN_LATIN_H

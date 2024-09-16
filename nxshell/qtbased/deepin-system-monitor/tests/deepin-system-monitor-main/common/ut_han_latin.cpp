// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "common/han_latin.h"
#include "unicode/parseerr.h"
#include "unicode/translit.h"
#include "unicode/utypes.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <sys/time.h>
#include <QApplication>
#include <QDateTime>

using namespace util::common;
using namespace std;
using namespace icu;
#define TRANSLITERATION_HAN_LATIN "Han-Latin"
#define TRANSLITERATION_LATIN_ASCII "Latin-ASCII"

/***************************************STUB begin*********************************************/
bool stub_convHanToLatin_ufailure(int)
{
    return true;
}
/***************************************STUB end**********************************************/


TEST(UT_HanLatin, test_convHanToLatin_01)
{
    QString words;
    convHanToLatin(words);
}

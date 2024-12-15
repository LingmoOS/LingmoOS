// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fsearch_utils.h"
#include "utils/chineseletterhelper.h"

#include <QRegExp>

#include <stdio.h>

bool convert_chinese_2_pinyin(const char *in_str, char *out_first_py, char *out_full_py)
{
    QRegExp reg("[\u4e00-\u9fa5]");
    QString str(in_str);
    if (!str.contains(reg))
        return false;

    QString first_py;
    QString full_py;
    bool ret = GrandSearch::ChineseLetterHelper::instance()->convertChinese2Pinyin(str, first_py, full_py);
    if (ret) {
        int full_py_len = full_py.toLocal8Bit().size();
        int first_py_len = first_py.toLocal8Bit().size();

        memcpy(out_full_py, full_py.toLocal8Bit().data(), full_py_len > FILENAME_MAX ? FILENAME_MAX : static_cast<size_t>(full_py_len));
        memcpy(out_first_py, first_py.toLocal8Bit().data(), first_py_len > FILENAME_MAX ? FILENAME_MAX : static_cast<size_t>(first_py_len));
    }

    return ret;
}

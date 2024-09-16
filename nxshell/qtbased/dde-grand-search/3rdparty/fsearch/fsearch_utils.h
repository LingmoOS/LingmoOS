// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSEARCH_UTILS_H
#define FSEARCH_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

extern bool convert_chinese_2_pinyin(const char *in_str, char *out_first_py, char *out_full_py);

#ifdef __cplusplus
}
#endif

#endif // FSEARCH_UTILS_H

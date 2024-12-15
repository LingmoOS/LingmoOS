// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONVERTTIFF_H
#define CONVERTTIFF_H

/**
 * @brief Convert old-style compression picture to none comporession format.
 * @param src source image.
 * @param dst dest image.
 * @return 0 means successed, other failed.
 */
int convertOldStyleImage(const char *src, const char *dst);

#endif //CONVERTTIFF_H

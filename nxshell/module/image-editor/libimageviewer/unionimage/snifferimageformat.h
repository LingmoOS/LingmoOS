// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SNIFFERIMAGEFORMAT_H
#define SNIFFERIMAGEFORMAT_H

#include <QString>

// Sniff image format based on its content.
// Returns an empty string if failed.
QString DetectImageFormat(const QString &filepath);

#endif // SNIFFERIMAGEFORMAT_H

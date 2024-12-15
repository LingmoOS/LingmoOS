// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <QUrl>

bool initFFmpegVideoThumbnailer();
QImage runFFmpegVideoThumbnailer(const QUrl &url);

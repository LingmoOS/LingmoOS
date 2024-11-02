/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#pragma once

#include <QMap>
#include <QString>
#include <QDir>
namespace LingmoUISearch {

#define CONTENT_DATABASE_PATH_SLOT 1
static const int LABEL_MAX_WIDTH = 320;

static const QString HOME_PATH = QDir::homePath();
static const QString INDEX_PATH = HOME_PATH + QStringLiteral("/.config/org.lingmo/lingmo-search/index_data");
static const QString CONTENT_INDEX_PATH = HOME_PATH + QStringLiteral("/.config/org.lingmo/lingmo-search/content_index_data");
static const QString OCR_CONTENT_INDEX_PATH = HOME_PATH + QStringLiteral("/.config/org.lingmo/lingmo-search/ocr_content_index_data");

static const QString FILE_SEARCH_VALUE = QStringLiteral("0");
static const QString DIR_SEARCH_VALUE = QStringLiteral("1");
static const int OCR_MIN_SIZE = 200;

/**
 * changelog 1.0.1 修复部分中文字符在term中被截断的问题
 */
static const QString INDEX_DATABASE_VERSION = QStringLiteral("1.0.1");
/**
 * changelog 1.1.0 增加文件修改时间value
 */
static const QString CONTENT_DATABASE_VERSION = QStringLiteral("1.1.0");
static const QString OCR_CONTENT_DATABASE_VERSION = QStringLiteral("1.0.0");


static const QStringList allAppPath = {
    {HOME_PATH + "/.local/share/applications/"},
    {"/usr/share/applications/"}
};


//TODO Put things that needed to be put here here.
/**
 * @brief The DataBaseType enum
 * Basic   基础数据库
 * Content 文本内容数据库
 */
enum class DataBaseType {
    Basic        = 0,
    Content      = 1,
    OcrContent   = 2
};

/**
 * @brief The IndexType enum
 * Index type
 */
enum class IndexType {
    Basic,
    Contents,
    OCR,
    Ai
};
}

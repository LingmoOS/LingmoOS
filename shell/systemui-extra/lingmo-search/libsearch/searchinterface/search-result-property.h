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
 */
#ifndef SEARCHRESULTPROPERTY_H
#define SEARCHRESULTPROPERTY_H
#include <QMap>
#include <QVector>
#include <QVariant>
namespace LingmoUISearch {
namespace SearchProperty {

/**
 * @brief The SearchType enum 表示搜索插件种类
 */
enum class SearchType
{
    File        = 1u << 0,
    FileContent = 1u << 1,
    Application = 1u << 2,
    Setting     = 1u << 3,
    Note        = 1u << 4,
    Mail        = 1u << 5,
    Custom      = 1u << 6
};
/**
 * @brief The SearchResultProperty enum 表示搜索结果信息
 */
enum SearchResultProperty {
    Invalid = 0,
    //文件搜索
    FilePath,                 //文件路径
    FileName,                 //文件名
    FileIconName,             //文件图标名称
    ModifiedTime,             //文件修改时间
    //应用搜索
    ApplicationDesktopPath,   //应用desktop文件路径
    ApplicationLocalName,     //应用本地化名称
    ApplicationIconName,      //应用图标名称（或路径）
    ApplicationDescription,   //应用描述（针对软件商店上架应用）
    IsOnlineApplication,      //是否是未安装应用
    ApplicationPkgName        //应用包名
};
}
typedef QVector<SearchProperty::SearchResultProperty> SearchResultProperties;
typedef QMap<SearchProperty::SearchResultProperty, QVariant> SearchResultPropertyMap;
}
Q_DECLARE_METATYPE(LingmoUISearch::SearchResultProperties);
#endif // SEARCHRESULTPROPERTY_H

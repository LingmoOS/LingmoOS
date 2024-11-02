/*
 *
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
#ifndef APPLICATIONPROPERTY_H
#define APPLICATIONPROPERTY_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QVariant>

namespace LingmoUISearch {

namespace ApplicationProperty {
/**
 * @brief 表示应用基础信息
 */
enum Property {
    Invalid = 0,
    DesktopFilePath,           //desktop文件路径
    ModifiedTime,                //数据修改时间
    InsertTime,                  //数据插入时间
    LocalName,                   //应用名（本地化）
    NameEn,                      //应用英文名
    NameZh,                      //应用中文名
    PinyinName,                  //名称拼音
    FirstLetterOfPinyin,         //拼音首字母
    FirstLetterAll,              //名称首字母
    Icon,                        //应用图标
    Type,                        //种类（Application等）
    Category,                    //分类（AudioVideo等）
    Exec,                        //执行命令
    Comment,                     //简介
    Md5,                         //desktop文件的md5
    LaunchTimes,                 //启动次数
    Favorites,                   //收藏顺序（0 未收藏）
    Launched,                    //是否从开始菜单启动过
    Top,                         //置顶顺序（0 未置顶）
    Lock,                        //是否锁定（不允许显示或打开）
    DontDisplay,                 //是否不需要显示（设置了Nodisplay等字段）
    AutoStart,                    //是否自启动（位于自启动目录/etc/xdg/autostart下）
    StartUpWMClass,              //应用的className
    Keywords,                    //关键词
    LocalKeywords                //关键词（本地化）
};
} //namespace ApplicationProperty
typedef QVector<ApplicationProperty::Property> ApplicationProperties;
typedef QMap<ApplicationProperty::Property, QVariant> ApplicationPropertyMap;
typedef QMap<QString, ApplicationPropertyMap> ApplicationInfoMap; // desktopFile->ApplicationPropertyMap
}
Q_DECLARE_METATYPE(LingmoUISearch::ApplicationProperty::Property)
Q_DECLARE_METATYPE(LingmoUISearch::ApplicationPropertyMap)
Q_DECLARE_METATYPE(LingmoUISearch::ApplicationInfoMap)
Q_DECLARE_METATYPE(LingmoUISearch::ApplicationProperties)
#endif // APPLICATIONPROPERTY_H

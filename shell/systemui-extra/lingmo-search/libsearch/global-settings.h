/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangzihao <zhangzihao@kylinos.cn>
 * Modified by: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */
#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QObject>
#include <QString>
#include <QVariant>
#include "libsearch_global.h"

namespace LingmoUISearch {

const static QString TRANSPARENCY_KEY = "transparency";
const static QString STYLE_NAME_KEY = "styleName";
const static QString FONT_SIZE_KEY = "systemFontSize";
const static QString ICON_THEME_KEY = "iconThemeName";
const static QString WINDOW_RADIUS_KEY = "windowRadius";

class GlobalSettingsPrivate;
/**
 * @brief The GlobalSettings class
 * 用于同步搜索应用的全局设置
 * 不要作为文件索引或应用数据等服务设置使用
 */
class LIBSEARCH_EXPORT GlobalSettings : public QObject {
    Q_OBJECT
public:
    static GlobalSettings& getInstance();
    const QVariant getValue(const QString& key);

Q_SIGNALS:
    void valueChanged(const QString& key, QVariant value);

private:
    explicit GlobalSettings(QObject *parent = nullptr);
    GlobalSettings(const GlobalSettings&) = delete;
    GlobalSettings& operator =(const GlobalSettings&) = delete;

    GlobalSettingsPrivate *d;
};

}

#endif // GLOBALSETTINGS_H

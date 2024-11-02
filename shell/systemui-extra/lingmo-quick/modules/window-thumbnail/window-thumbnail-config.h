/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_WINDOW_THUMBNAIL_CONFIG_H
#define LINGMO_QUICK_WINDOW_THUMBNAIL_CONFIG_H
#include <QObject>

class WindowThumbnailConfig: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool pipewireThumbnailEnable READ pipewireThumbnailEnable NOTIFY pipeWireThumbnailEnableChanged)
    Q_PROPERTY(bool kywlcomWindowThumbnailEnable READ kywlcomWindowThumbnailEnable NOTIFY kywlcomWindowThumbnailEnableChanged)
    Q_PROPERTY(bool xThumbnailEnable READ xThumbnailEnable NOTIFY xThumbnailEnableChanged)
    Q_PROPERTY(bool realTimeThumbnailEnable READ realTimeThumbnailEnable NOTIFY realTimeThumbnailEnableChanged)

public:
    WindowThumbnailConfig(QObject *parent = nullptr);
    bool pipewireThumbnailEnable();
    static bool realTimeThumbnailEnable();
    bool kywlcomWindowThumbnailEnable() const;
    bool xThumbnailEnable() const;

Q_SIGNALS:
    void realTimeThumbnailEnableChanged();
    void pipeWireThumbnailEnableChanged();
    void kywlcomWindowThumbnailEnableChanged();
    void xThumbnailEnableChanged();

private:
    bool m_pipewireThumbnailEnable = false;
    bool m_kywlcomThumbnailEnable = false;
    bool m_xThumbnailEnable = false;
};


#endif //LINGMO_QUICK_WINDOW_THUMBNAIL_CONFIG_H

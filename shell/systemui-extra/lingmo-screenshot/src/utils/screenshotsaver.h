/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
 *              2020 KylinSoft Co., Ltd.
 * This file is part of Lingmo-Screenshot.
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
*/

#pragma once
#include <QGSettings/qgsettings.h>
#include "src/utils/desktopinfo.h"

class QPixmap;
class QString;

class ScreenshotSaver
{
public:
    ScreenshotSaver();

    void saveToClipboard(const QPixmap &capture);
    bool saveToFilesystem(const QPixmap &capture, const QString &path);
    bool saveToFilesystem(const QPixmap &capture, const QString &path, const QString &type);
    bool saveToFilesystemGUI(const QPixmap &capture);
private:
    QGSettings *ScreenshotGsettings;
    QString m_savePath, m_saveName, m_saveType;
    DesktopInfo m_info;

};

/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef LINGMOSTYLESETTINGS_H
#define LINGMOSTYLESETTINGS_H

#include "libqt5-lingmo-style_global.h"
#include <QGSettings>

//Fix me:after so
//#include "lingmo-style-conf-settings.h"

/*!
 * \brief The LINGMOStyleSettings class
 * \details
 * To distingust with other gsettings, I derived this class form QGSettings.
 * It just represent the specific gsettings "org.lingmo.style", and
 * there is no api difference from LINGMOStyleSettings to QGSettings.
 */
class LIBQT5LINGMOSTYLESHARED_EXPORT LINGMOStyleSettings : public QGSettings
{
    Q_OBJECT
public:
    LINGMOStyleSettings();

    static LINGMOStyleSettings *globalInstance();
};

#endif // LINGMOSTYLESETTINGS_H

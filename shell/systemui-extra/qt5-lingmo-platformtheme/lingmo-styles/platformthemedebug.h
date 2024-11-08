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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef PLATFORMTHEMEDEBUG_H
#define PLATFORMTHEMEDEBUG_H

#include <QDebug>


//#define LINGMOStyle_Debug
//#define AnimationSlider_Debug
//#define AnimationScrollbar_Debug

//#define LINGMOStyle_Warning

class Debug

{

public:

template<typename T>

Debug & operator <<(const T&){return *this;}

};
inline Debug debug(){return Debug();}


#ifdef LINGMOStyle_Debug

#    define cDebug qDebug() << "LINGMOStyle input:"

#else

#    define cDebug Debug()

#endif

#ifdef AnimationSlider_Debug

#    define aSliderDebug qDebug() << "LINGMOStyle input:"

#else

#    define aSliderDebug Debug()

#endif

#ifdef AnimationScrollbar_Debug

#    define aScrollBarDebug qDebug() << "LINGMOStyle input:"

#else

#    define aScrollBarDebug Debug()

#endif

class Warning

{

public:

template<typename T>

Warning & operator <<(const T&){return *this;}

};
inline Warning warning(){return Warning();}

#ifdef LINGMOStyle_Warning

#    define cWarning qWarning() << "LINGMOStyle input:"

#else

#    define cWarning Warning()

#endif

#endif // PLATFORMTHEMEDEBUG_H

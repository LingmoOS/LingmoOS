/*  smplayer, GUI front-end for mplayer.
    Copyright (C) 2006-2015 Ricardo Villalba <rvm@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <QString>
#include <QSettings>

#include <KF5/KWindowSystem/kwindoweffects.h>


class QSettings;
class QGSettings;

namespace Global {

// 错误类型
enum KERROR_TYPE{
    NO_ERROR = 0
};

//! Read and store application settings
extern bool isWayland;

void global_init();
void global_end();

}

#endif


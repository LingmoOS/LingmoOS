/*
 *
 * Copyright:  2020 KylinSoft Co., Ltd.
 * Authors:
 *   wenjie bi <biwenjie@cs2c.com.cn>
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */
#pragma once

#include <exception>

#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xinerama.h>

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/record.h>
#if 0
// #undef Bool
#undef Status
#undef min
#undef max
#undef CursorShape
#undef Expose
// #undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
// #undef None
#undef Unsorted
#undef XButton1
#undef XButton2
#undef Data
#undef Bool
#endif

class X11Exception : public std::exception
{
public:
    inline virtual const char *what() const throw() override
    {
        return "X11Exception";
    }
};

/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Meihong <hemeihong@kylinos.cn>
 *
 */

#ifndef PEONYQTSETWALLPAPER_GLOBAL_H
#define PEONYQTSETWALLPAPER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PEONYSETWALLPAPER_LIBRARY)
#  define PEONYQTSETWALLPAPERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PEONYQTSETWALLPAPERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PEONYQTSETWALLPAPER_GLOBAL_H

/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef PEONYCORE_GLOBAL_H
#define PEONYCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#undef signals
#undef slots
#undef emit

#if defined(PEONYCORE_LIBRARY)
#  define PEONYCORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define PEONYCORESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PEONYCORE_GLOBAL_H

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
#ifndef COMPATIBLEDEFINE_H
#define COMPATIBLEDEFINE_H
#include <QtGlobal>

#endif // COMPATIBLEDEFINE_H
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
//T QAtomicInteger::load() const
#define LOAD load()

//QMap<Key, T> &QMap::unite(const QMap<Key, T> &other)
#define INSERT(T) unite(T)
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
//T QAtomicInteger::loadRelaxed() const
#define LOAD loadRelaxed()
//void QMap::insert(const QMap<Key, T> &map)
#define INSERT(T) insert(T)
#endif

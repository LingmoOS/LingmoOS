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
 *
 */
#ifndef COMMON_H
#define COMMON_H
#include <QtCore>
#include <QtConcurrent/QtConcurrent>

#define SERVER "Everything"

#define LOG(a) \
	//qWarning() << a;

#define REHASH(a) \
	if (sl_minus_1 < (int)sizeof(int) * CHAR_BIT)       \
		hashHaystack -= (a) << sl_minus_1; \
	hashHaystack <<= 1

void* xmalloc(size_t tSize);

void* xcalloc(size_t tNmemb, size_t tSize);

void* xrealloc(void *pvArg, size_t tSize);

void* xfree(void *pvArg);


#endif // COMMON_H

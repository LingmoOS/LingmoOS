/*
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */


#ifndef LINGMOSDK_BASE_UTILS_MARCOS_H__
#define LINGMOSDK_BASE_UTILS_MARCOS_H__

/*
    GNU Attribute
*/
#define	DEPRECATED	__attribute__((deprecated))
#define	CONSTRUCTOR	__attribute__((constructor))
#define	DESTRUCTOR	__attribute__((destructor))
#define	FORMAT(type, fmtindex, argindex)	__attribute__((format(type, fmtindex, argindex)))
#define	WARNUNSEDRET	__attribute__((warn_unused_result))
#define	WEAK		__attribute__((weak))
#define ALWAYSINLINE    __attribute__((always_inline))
#define	CHECK_FMT(x,y)	__attribute__((format(printf,x,y)))
#define	NOINLINE	__attribute__((noinline))
#define	NOTNULL(...)	__attribute__((nonnull(__VA_ARGS__)))
#define	MUSTCHECKRESULT	__attribute__((warn_unused_result))

/*
    Customize
*/
#define ASSERT_NOT_NULL(x, ret)  if (!x) {return ret;}
#define SAFE_FREE(x) if (x) {free(x); x = NULL;}

#endif

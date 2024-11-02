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


#ifndef LINGMOSDK_BASE_PROCESSDAEMON_H__
#define LINGMOSDK_BASE_PROCESSDAEMON_H__

namespace LINGMOSDK_BASE
{
typedef void (*daemon_callback)(void *args);
extern int startProcDaemon(unsigned int interval, daemon_callback func, void* args);
extern void feedDog();
extern int stopProcDaemon(); 
} // namespace LINGMOSDK_BASE

#endif

/**
 * \example lingmosdk-base/src/process/test/main.cpp
 * 
 */

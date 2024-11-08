/*
 * liblingmosdk-system's Library
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "../libkyrealtimeinfo.h"
#include <iostream>

int main()
{
    RealTimeInfo info;
    std::cout << "Of speed:" << info.kdk_real_get_net_speed("enp3s0") << std::endl;
    std::cout << "If speed:" << info.kdk_real_get_if_speed("enp3s0") << std::endl;
    std::cout << "cpu Temp:" << info.kdk_real_get_cpu_temperature() << std::endl;
    std::cout << "Disk Temp:" << info.kdk_real_get_disk_temperature("/dev/sda") << std::endl;
    std::cout << "Disk rate:" << info.kdk_real_get_disk_rate("/dev/sda") << std::endl;
    return 0;
}
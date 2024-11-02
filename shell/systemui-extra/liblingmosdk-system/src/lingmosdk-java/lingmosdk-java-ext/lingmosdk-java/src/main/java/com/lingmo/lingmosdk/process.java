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

package com.lingmo.lingmosdk;

import java.util.List;
import org.freedesktop.dbus.interfaces.DBusInterface;

public interface process extends DBusInterface {
    //输入：进程号
    //输出：CPU瞬时使用率
    public double getProcInfoCpuUsage(int proc);
    //输入：进程号
    //输出：IO使用率
    public double getProcInfoIoUsage(int proc);
    //输入：进程号
    //输出：内存占用率
    public double getProcInfoMemUsage(int proc);
    //输入：进程号
    //输出：进程状态
    public String getProcInfoStatus(int proc);
    //输入：进程号
    //输出：进程使用的端口号
    public int getProcInfoPort(int proc);
    //输入：进程号
    //输出：进程的启动时间
    public String getProcInfoStartTime(int proc);
    //输入：进程号
    //输出：进程的运行时间
    public String getProcInfoRunningTime(int proc);
    //输入：进程号
    //输出：CPU时间
    public String getProcInfoCpuTime(int proc);
    //输入：进程号
    //输出：cmd
    public String getProcInfoCmd(int proc);
    //输入：进程号
    //输出：属主
    public String getProcInfoUser(int proc);
    //输入：进程名
    //输出：某进程所有信息
    public List<String> getProcInfo(String proc);
    public List<String> getAllProcInfo();

}

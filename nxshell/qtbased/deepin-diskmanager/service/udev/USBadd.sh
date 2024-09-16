#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-only

PROC_NAME='deepin-diskmanager'

# 前端进程'deepin-diskmanager'起来会设置udisks2-inhibit，只有磁盘管理器启动时才需DBus更新挂/卸载
ProcNumber=`ps aux |grep udisks2-inhibit |sed '/grep/d' |sed '/tail/d'`
if [ -n "$ProcNumber" ];then
   /usr/bin/dbus-send --system --type=method_call --dest=com.deepin.diskmanager /com/deepin/diskmanager com.deepin.diskmanager.updateUsb
else
   exit 0
fi


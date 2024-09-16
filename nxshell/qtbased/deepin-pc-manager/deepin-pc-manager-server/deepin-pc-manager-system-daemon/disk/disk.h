// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <fcntl.h>
#include <unistd.h>

// 获取磁盘设备路径列表
QList<QString> getDiskDevicePathList();

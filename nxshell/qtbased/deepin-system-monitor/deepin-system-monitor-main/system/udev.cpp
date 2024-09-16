// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "udev.h"

#include <libudev.h>

namespace core {
namespace system {

UDev::UDev()
{
    m_udev = udev_new();
}

UDev::~UDev()
{
    udev_unref(m_udev);
}

} // namespace system
} // namespace core

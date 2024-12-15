// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "service.h"

Service::Service(QObject *parent)
    : QObject(parent)
{
}

QString Service::Hello()
{
    return "World";
}

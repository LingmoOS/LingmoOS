// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dopticaldiscmanager.h"

#include <DError>

#include "dopticaldiscinfo.h"
#include "dopticaldiscoperator.h"

DBURN_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DError;
using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DUnexpected;

DExpected<DOpticalDiscInfo *> DOpticalDiscManager::createOpticalDiscInfo(const QString &dev, QObject *parent)
{
    auto info { new DOpticalDiscInfo(dev, parent) };
    if (info && info->device().isEmpty()) {
        delete info;
        return DUnexpected<> { DError { -1, QString("Invalid device: %1").arg(dev) } };
    }
    return info;
}

DExpected<DOpticalDiscOperator *> DOpticalDiscManager::createOpticalDiscOperator(const QString &dev, QObject *parent)
{
    if (dev.isEmpty())
        return DUnexpected<> { DError { -1, "Empty device" } };
    return { new DOpticalDiscOperator(dev, parent) };
}

DBURN_END_NAMESPACE

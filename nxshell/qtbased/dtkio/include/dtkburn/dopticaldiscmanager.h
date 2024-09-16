// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOPTIFCALDISCMANAGER_H
#define DOPTIFCALDISCMANAGER_H

#include <QObject>

#include <DExpected>

#include "dtkburn_global.h"

DBURN_BEGIN_NAMESPACE

class DOpticalDiscInfo;
class DOpticalDiscOperator;

namespace DOpticalDiscManager {
[[nodiscard]] DTK_CORE_NAMESPACE::DExpected<DOpticalDiscInfo *> createOpticalDiscInfo(const QString &dev, QObject *parent = nullptr);
[[nodiscard]] DTK_CORE_NAMESPACE::DExpected<DOpticalDiscOperator *> createOpticalDiscOperator(const QString &dev, QObject *parent = nullptr);
}   // namespace DOpticalDiscManager

DBURN_END_NAMESPACE

#endif   // DOPTIFCALDISCMANAGER_H

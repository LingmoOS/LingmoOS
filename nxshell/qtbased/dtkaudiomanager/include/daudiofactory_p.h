// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOFACTORY_P_H
#define DAUDIOFACTORY_P_H

#include "dtkaudiomanager_global.h"

DAUDIOMANAGER_BEGIN_NAMESPACE
class DAudioManagerPrivate;
class DAudioFactory
{
public:
    static DAudioManagerPrivate *createAudioManager();
    static DAudioManagerPrivate *createAudioManager(const QString &type);
};
DAUDIOMANAGER_END_NAMESPACE

#endif

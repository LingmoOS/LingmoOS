// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCONFIGHIOCEANNMENUSCENE_P_H
#define DCONFIGHIOCEANNMENUSCENE_P_H

#include "menuscene/dconfighioceannmenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

DFMBASE_USE_NAMESPACE
DPMENU_BEGIN_NAMESPACE

class DConfigHioceannMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class DConfigHioceannMenuScene;

public:
    explicit DConfigHioceannMenuScenePrivate(DConfigHioceannMenuScene *qq);
};

DPMENU_END_NAMESPACE

#endif   // DCONFIGHIOCEANNMENUSCENE_P_H

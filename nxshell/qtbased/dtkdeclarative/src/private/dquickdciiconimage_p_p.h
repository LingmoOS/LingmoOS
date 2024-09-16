// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKDCIICONIMAGE_P_P_H
#define DQUICKDCIICONIMAGE_P_P_H

#include "dquickiconimage_p.h"
#include "dqmlglobalobject_p.h"
#include "dquickdciiconimage_p.h"
#include "dquickiconimage_p_p.h"

#include <dobject_p.h>
#include <DDciIconPalette>

DQUICK_BEGIN_NAMESPACE
class DQuickDciIconImageItemPrivate;
class DQuickDciIconImageItemPrivate : public DQuickIconImagePrivate
{
    Q_DECLARE_PUBLIC(DQuickIconImage)

public:
    DQuickDciIconImageItemPrivate(DQuickDciIconImagePrivate *pqq);
    void maybeUpdateUrl();
    QUrlQuery getUrlQuery();

private:
    DQuickDciIconImagePrivate *parentPriv;
};

class DQuickDciIconImagePrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    Q_DECLARE_PUBLIC(DQuickDciIconImage)

public:
    DQuickDciIconImagePrivate(DQuickDciIconImage *qq);
    void layout();
    void updateImageSourceUrl();

    DDciIconPalette palette;
    DQuickIconImage *imageItem;
    DQMLGlobalObject::ControlState mode = DQMLGlobalObject::NormalState;
    DGuiApplicationHelper::ColorType theme = DGuiApplicationHelper::ColorType::LightType;
    bool fallbackToQIcon = true;
};

DQUICK_END_NAMESPACE
#endif // DQUICKDCIICONIMAGE_P_P_H

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFONTSIZEMANAGER_P_H
#define DFONTSIZEMANAGER_P_H

#include <DObjectPrivate>
#include <DObject>

#include "dfontmanager.h"

DGUI_BEGIN_NAMESPACE

class DFontManagerPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DFontManagerPrivate(DFontManager *qq);

    int fontPixelSize[DFontManager::NSizeTypes] = {40, 30, 24, 20, 17, 14, 13, 12, 11, 10};
    int baseFontSizeType = DFontManager::T6;
    // 字号的差值
    int fontPixelSizeDiff = 0;
    QFont baseFont;

private:
    D_DECLARE_PUBLIC(DFontManager)
};

DGUI_END_NAMESPACE

#endif // DFONTSIZEMANAGER_P_H

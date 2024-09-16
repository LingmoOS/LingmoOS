// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMINPUTCONTEXTHOOK_H
#define DPLATFORMINPUTCONTEXTHOOK_H

#include "global.h"

#include <QRectF>
#include <QtGlobal>

#include "im_interface.h"


QT_BEGIN_NAMESPACE
class QPlatformInputContext;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DPlatformInputContextHook
{

public:
    static void showInputPanel(QPlatformInputContext *inputContext);
    static void hideInputPanel(QPlatformInputContext *inputContext);
    static bool isInputPanelVisible(QPlatformInputContext *inputContext);
    static QRectF keyboardRect(QPlatformInputContext *inputContext);

    static ComDeepinImInterface* instance();
};

DPP_END_NAMESPACE

#endif // DPLATFORMINPUTCONTEXTHOOK_H

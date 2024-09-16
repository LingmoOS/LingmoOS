// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplatforminputcontexthook.h"

#include "global.h"
#include <qpa/qplatforminputcontext.h>

DPP_BEGIN_NAMESPACE

void DPlatformInputContextHook::showInputPanel(QPlatformInputContext *inputContext)
{
    Q_UNUSED(inputContext)
    instance()->setImActive(true);
}

void DPlatformInputContextHook::hideInputPanel(QPlatformInputContext *inputContext)
{
    Q_UNUSED(inputContext)
    instance()->setImActive(false);
}

bool DPlatformInputContextHook::isInputPanelVisible(QPlatformInputContext *inputContext)
{
    Q_UNUSED(inputContext)
    return instance()->imActive();
}

QRectF DPlatformInputContextHook::keyboardRect(QPlatformInputContext *inputContext)
{
    Q_UNUSED(inputContext)
    return instance()->geometry();
}

Q_GLOBAL_STATIC_WITH_ARGS(ComDeepinImInterface, __imInterface,
                          (QString("com.deepin.im"), QString("/com/deepin/im"), QDBusConnection::sessionBus()))

ComDeepinImInterface* DPlatformInputContextHook::instance()
{
    return __imInterface;
}

DPP_END_NAMESPACE

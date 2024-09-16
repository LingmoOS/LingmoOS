// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPLATFORMNATIVEINTERFACE_H
#define DPLATFORMNATIVEINTERFACE_H

#include <QtGlobal>

#include "global.h"

QT_BEGIN_NAMESPACE
class QPlatformNativeInterface;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DPlatformNativeInterfaceHook
{
public:
    static QFunctionPointer platformFunction(QPlatformNativeInterface *interface, const QByteArray &function);
};

DPP_END_NAMESPACE

#endif // DPLATFORMNATIVEINTERFACE_H

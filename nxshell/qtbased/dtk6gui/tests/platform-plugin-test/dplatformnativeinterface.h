// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMNATIVEINTERFACE_H
#define DPLATFORMNATIVEINTERFACE_H

#include <QObject>
#include <qpa/qplatformnativeinterface.h>

class DPlatformNativeInterface : public QPlatformNativeInterface
{
public:
    DPlatformNativeInterface();
    virtual QFunctionPointer platformFunction(const QByteArray &function) const;
};

#endif // DPLATFORMNATIVEINTERFACE_H

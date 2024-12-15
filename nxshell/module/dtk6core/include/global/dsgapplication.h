// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSGAPPLICATION_H
#define DSGAPPLICATION_H

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DSGApplication
{
public:
    static QByteArray id();
    static QByteArray getId(qint64 pid);
};

DCORE_END_NAMESPACE

#endif // DSGAPPLICATION_H

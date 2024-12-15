// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOBJECT_P_H
#define DOBJECT_P_H

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DObject;
class LIBDTKCORESHARED_EXPORT DObjectPrivate
{
public:
    virtual ~DObjectPrivate();

protected:
    DObjectPrivate(DObject *qq);

    DObject *q_ptr;

    Q_DECLARE_PUBLIC(DObject)
};

DCORE_END_NAMESPACE

#endif // DOBJECT_P_H


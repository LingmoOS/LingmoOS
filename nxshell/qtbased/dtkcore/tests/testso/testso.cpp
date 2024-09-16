// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "testso.h"

char TestClass::A::test(int v)
{
    qDebug() << Q_FUNC_INFO << this << v;

    return 'a';
}

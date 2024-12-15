// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkcore_global.h"
#include <QString>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DSecureString : public QString
{
public:
    using QString::QString;
    DSecureString(const QString &other) noexcept;
    ~DSecureString();
};

DCORE_END_NAMESPACE

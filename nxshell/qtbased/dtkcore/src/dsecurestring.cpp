// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsecurestring.h"
#include "dutil.h"
#include <cstring>

DCORE_BEGIN_NAMESPACE

DSecureString::DSecureString(const QString &other) noexcept
    : QString(other)
{
}

DSecureString::~DSecureString()
{
    DUtil::SecureErase(*this);
}

DCORE_END_NAMESPACE

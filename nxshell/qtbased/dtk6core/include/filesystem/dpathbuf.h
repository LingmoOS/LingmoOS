// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDir>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DPathBuf
{
public:
    DPathBuf();
    DPathBuf(const QString &path);

    DPathBuf operator/(const QString &p) const
    {
        return DPathBuf(m_path + "/" + p);
    }

    DPathBuf &operator/=(const QString &p)
    {
        return join(p);
    }

    DPathBuf operator/(const char *p) const
    {
        return operator /(QString(p));
    }

    DPathBuf &operator/=(const char *p)
    {
        return operator /=(QString(p));
    }

    DPathBuf &join(const QString &p)
    {
        m_path += "/" + p;
        m_path = QDir(m_path).absolutePath();
        return *this;
    }

    QString toString() const
    {
        return QDir::toNativeSeparators(m_path);
    }

private:
    QString m_path;
};

DCORE_END_NAMESPACE

/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "commandresult.h"

namespace PlasmaVault
{
Error::Error(Code code, const QString &message, const QString &out, const QString &err)
    : m_code(code)
    , m_message(message)
    , m_out(out)
    , m_err(err)
{
}

Error::Code Error::code() const
{
    return m_code;
}

QString Error::message() const
{
    return m_message;
}

QString Error::out() const
{
    return m_out;
}

QString Error::err() const
{
    return m_err;
}

} // namespace PlasmaVault

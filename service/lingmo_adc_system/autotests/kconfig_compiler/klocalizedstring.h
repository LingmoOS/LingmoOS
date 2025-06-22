/*
    SPDX-FileCopyrightText: 2015 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: MIT
*/

/* This file is needed for test_translation_kde.
 * It provides fake i18n* in order to avoid dependency on Ki18n framework.
 */

#ifndef KLOCALIZEDSTRING_H
#define KLOCALIZEDSTRING_H

inline QString i18n(const char *msgid)
{
    return QString::fromUtf8(msgid);
}

inline QString i18nc(const char *msgctxt, const char *msgid)
{
    Q_UNUSED(msgctxt);
    return QString::fromUtf8(msgid);
}

inline QString i18nd(const char *domain, const char *msgid)
{
    Q_UNUSED(domain);
    return QString::fromUtf8(msgid);
}

inline QString i18ndc(const char *domain, const char *msgctxt, const char *msgid)
{
    Q_UNUSED(domain);
    Q_UNUSED(msgctxt);
    return QString::fromUtf8(msgid);
}

#endif

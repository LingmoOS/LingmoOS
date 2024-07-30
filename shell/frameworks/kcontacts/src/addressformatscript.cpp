/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "address.h"
#include "addressformatscript_p.h"

using namespace KContacts;

AddressFormatScript::ScriptType AddressFormatScript::detect(const QString &s)
{
    for (auto c : s) {
        switch (c.script()) {
        case QChar::Script_Arabic:
            return AddressFormatScript::ArabicLikeScript;
        case QChar::Script_Han:
            return AddressFormatScript::HanLikeScript;
        case QChar::Script_Hangul:
        case QChar::Script_Thai:
            return AddressFormatScript::HangulLikeScript;
        default:
            break;
        }
    }
    return AddressFormatScript::LatinLikeScript;
}

AddressFormatScript::ScriptType AddressFormatScript::detect(const Address &addr)
{
    return std::max(detect(addr.street()), detect(addr.locality()));
}

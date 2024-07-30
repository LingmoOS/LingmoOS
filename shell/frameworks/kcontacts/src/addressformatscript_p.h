/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_ADDRESSFORMATSCRIPT_P_H
#define KCONTACTS_ADDRESSFORMATSCRIPT_P_H

#include "addressformat.h"

namespace KContacts
{

/**
 * Classify the script used in an address.
 * This is used for two things:
 * - determining the line style separator
 * - decide whether to use local or latin script format alternatives when available
 *
 * @see address_formatter.cc in libaddressinput
 * @internal
 */
namespace AddressFormatScript
{
enum ScriptType {
    LatinLikeScript,
    ArabicLikeScript,
    HanLikeScript,
    HangulLikeScript,
};

ScriptType detect(const QString &s);
ScriptType detect(const Address &addr);
}
}

#endif // KCONTACTS_ADDRESSFORMATSCRIPT_P_H

/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "isocodes_p.h"

#include <limits>

// "unit tests" for the low-level encoding functions
static_assert(IsoCodes::mapToUpper(QLatin1Char('a')) == 'A');
static_assert(IsoCodes::mapToAlphaNumKey(QLatin1Char('A')) == 11);
static_assert(IsoCodes::mapToAlphaNumKey('A') == IsoCodes::mapToAlphaNumKey('a'));
static_assert(IsoCodes::mapToAlphaNumKey(QLatin1Char('z')) < IsoCodes::AlphaNumKeyFactor);
static_assert(IsoCodes::mapToAlphaNumKey(QLatin1Char('0')) == 1);
static_assert(IsoCodes::mapToAlphaNumKey(QLatin1Char('9')) == 10);
static_assert(IsoCodes::AlphaNumKeyFactor * IsoCodes::AlphaNumKeyFactor * IsoCodes::AlphaNumKeyFactor < std::numeric_limits<uint16_t>::max());

static_assert(IsoCodes::mapFromAlphaNumKey(IsoCodes::mapToAlphaNumKey('0')) == '0');
static_assert(IsoCodes::mapFromAlphaNumKey(IsoCodes::mapToAlphaNumKey('9')) == '9');
static_assert(IsoCodes::mapFromAlphaNumKey(IsoCodes::mapToAlphaNumKey('a')) == 'A');
static_assert(IsoCodes::mapFromAlphaNumKey(IsoCodes::mapToAlphaNumKey('Z')) == 'Z');

static_assert(IsoCodes::alpha2CodeToKey("AZ") == 0x415a);
static_assert(IsoCodes::alpha2CodeToKey("az") == 0x415a);
static_assert(IsoCodes::alpha2CodeToKey("Az") == 0x415a);
static_assert(IsoCodes::alpha2CodeToKey("ZA") == 0x5a41);
static_assert(IsoCodes::alpha2CodeToKey("NZ") == IsoCodes::alpha2CodeToKey(u"NZ"));

static_assert(IsoCodes::alpha2CodeToKey("") == 0);
static_assert(IsoCodes::alpha2CodeToKey("12") == 0);
static_assert(IsoCodes::alpha2CodeToKey("A") == 0);
static_assert(IsoCodes::alpha2CodeToKey("ABC") == 0);
static_assert(IsoCodes::alpha2CodeToKey("A@") == 0);

static_assert(IsoCodes::alpha2CodeToKey("AA") < IsoCodes::alpha2CodeToKey("AB"));
static_assert(IsoCodes::alpha2CodeToKey("AZ") < IsoCodes::alpha2CodeToKey("BA"));

static_assert(IsoCodes::alpha3CodeToKey("ZZZ") < std::numeric_limits<uint16_t>::max());
static_assert(IsoCodes::alpha3CodeToKey("AAA") > 0);
static_assert(IsoCodes::alpha3CodeToKey("AAA") < IsoCodes::alpha3CodeToKey("AAB"));
static_assert(IsoCodes::alpha3CodeToKey("AAB") < IsoCodes::alpha3CodeToKey("BAA"));
static_assert(IsoCodes::alpha3CodeToKey("NZL") == IsoCodes::alpha3CodeToKey(u"NZL"));

static_assert(IsoCodes::alpha3CodeToKey("") == 0);
static_assert(IsoCodes::alpha3CodeToKey("AA") == 0);
static_assert(IsoCodes::alpha3CodeToKey("ABCD") == 0);
static_assert(IsoCodes::alpha3CodeToKey("AB1") == 0);
static_assert(IsoCodes::alpha3CodeToKey("A@C") == 0);

static_assert(IsoCodes::subdivisionCodeToKey("AA-AAA") > 0);
static_assert(IsoCodes::subdivisionCodeToKey("ZZ-ZZZ") > 0);
static_assert(IsoCodes::subdivisionCodeToKey("ZZ-999") < std::numeric_limits<uint32_t>::max());
static_assert(IsoCodes::subdivisionCodeToKey("AA-A") < IsoCodes::subdivisionCodeToKey("AA-AA"));
static_assert(IsoCodes::subdivisionCodeToKey("AA-AAA") < IsoCodes::subdivisionCodeToKey("AA-AAB"));
static_assert(IsoCodes::subdivisionCodeToKey("AA-AAA") < IsoCodes::subdivisionCodeToKey("AA-AB"));
static_assert(IsoCodes::subdivisionCodeToKey("AA-AAB") < IsoCodes::subdivisionCodeToKey("AA-BAA"));
static_assert(IsoCodes::subdivisionCodeToKey("AA-AA1") < IsoCodes::subdivisionCodeToKey("AA-AAZ"));
static_assert(IsoCodes::subdivisionCodeToKey("FR-99") < IsoCodes::subdivisionCodeToKey("FR-RE"));
static_assert(IsoCodes::subdivisionCodeToKey("AB-cd1") == IsoCodes::subdivisionCodeToKey("AB-CD1"));

static_assert(IsoCodes::subdivisionCodeToKey("") == 0);
static_assert(IsoCodes::subdivisionCodeToKey("AA-") == 0);
static_assert(IsoCodes::subdivisionCodeToKey("12-ABC") == 0);
static_assert(IsoCodes::subdivisionCodeToKey("AA-@") == 0);
static_assert(IsoCodes::subdivisionCodeToKey("AB") == 0);
static_assert(IsoCodes::subdivisionCodeToKey("ABC") == 0);

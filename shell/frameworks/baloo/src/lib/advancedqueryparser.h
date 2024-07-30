/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_ADVANCEDQUERYPARSER_H
#define BALOO_ADVANCEDQUERYPARSER_H

#include "query.h"
#include "term.h"

namespace Baloo {

class AdvancedQueryParser
{
public:
    AdvancedQueryParser();

    Baloo::Term parse(const QString& text);
};
}

#endif // BALOO_ADVANCEDQUERYPARSER_H

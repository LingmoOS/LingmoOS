/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KHOLIDAYS_HOLIDAYSCANNERPLAN_P_H
#define KHOLIDAYS_HOLIDAYSCANNERPLAN_P_H

// Flex expects the signature of yylex to be defined in the macro YY_DECL, and
// the C++ parser expects it to be declared. We can factor both as follows.

#ifndef YY_DECL

#define YY_DECL                                                                                                                                                \
    KHolidays::HolidayParserPlan::token_type KHolidays::HolidayScannerPlan::lex(KHolidays::HolidayParserPlan::semantic_type *yylval,                           \
                                                                                KHolidays::HolidayParserPlan::location_type *yylloc)
#endif

#ifndef __FLEX_LEXER_H
#define yyFlexLexer HolidayScannerFlexLexer
#include "FlexLexer.h"
#undef yyFlexLexer
#endif

#include "holidayparserplan.hpp"

namespace KHolidays
{
/**
 * HolidayScannerPlan implementation class
 *
 * Implements the flex scanner for scanning Plan holiday files as
 * implemented in KDE SC 4.5 onwards.  This includes extensions to the file
 * format to support non-Gregorian calendar systems, metadata, and other new
 * features.
 *
 * Implemented using Flex C++ mode, based on the example at
 * https://panthema.net/2007/flex-bison-cpp-example/ which is released to Public Domain.
 *
 * @internal Private, for internal use only
 */

class HolidayScannerPlan : public HolidayScannerFlexLexer
{
public:
    explicit HolidayScannerPlan(std::istream *arg_yyin = nullptr, std::ostream *arg_yyout = nullptr);

    ~HolidayScannerPlan() override;

    virtual HolidayParserPlan::token_type lex(HolidayParserPlan::semantic_type *yylval, HolidayParserPlan::location_type *yylloc);

    void set_debug(bool b);
};

} // namespace KHolidays

#endif // KHOLIDAYS_HOLIDAYSCANNERPLAN_P_H

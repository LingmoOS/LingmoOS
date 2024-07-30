
/* A Bison parser, made by GNU Bison 2.4.1.  */

/*  Skeleton implementation for Bison LALR(1) parsers in C++

    SPDX-FileCopyrightText: 2002, 2003, 2004, 2005, 2006, 2007, 2008 Free Software Foundation, Inc.

    SPDX-License-Identifier: GPL-3.0-or-later WITH Bison-exception-2.2
*/

// Take the name prefix into account.
#define yylex KHolidayslex

/* First part of user declarations.  */

/* Line 311 of lalr1.cc  */
#line 30 "holidayparserplan.ypp"
/*** C/C++ Declarations ***/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include <QString>

// clang-format off

/* Line 311 of lalr1.cc  */
#line 60 "holidayparserplan.cpp"

#include "holidayparserplan.hpp"

/* User implementation prologue.  */

/* Line 317 of lalr1.cc  */
#line 111 "holidayparserplan.ypp"

#include "holidayparserdriverplan_p.h"
#include "holidayscannerplan_p.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.m_scanner->lex

/* Line 317 of lalr1.cc  */
#line 83 "holidayparserplan.cpp"

#ifndef YY_
# if defined(YYENABLE_NLS) && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)  \
    do {                            \
        if (yydebug_)                     \
        {                           \
            *yycdebug_ << Title << ' ';           \
            yy_symbol_print_ ((Type), (Value), (Location));   \
            *yycdebug_ << std::endl;              \
        }                           \
    } while (false)

# define YY_REDUCE_PRINT(Rule)      \
    do {                    \
        if (yydebug_)             \
            yy_reduce_print_ (Rule);        \
    } while (false)

# define YY_STACK_PRINT()       \
    do {                    \
        if (yydebug_)             \
            yystack_print_ ();          \
    } while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_REDUCE_PRINT(Rule)
# define YY_STACK_PRINT()

#endif /* !YYDEBUG */

#define yyerrok     (yyerrstatus_ = 0)
#define yyclearin   (yychar = yyempty_)

#define YYACCEPT    goto yyacceptlab
#define YYABORT     goto yyabortlab
#define YYERROR     goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

/* Line 380 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

namespace KHolidays
{

/* Line 380 of lalr1.cc  */
#line 152 "holidayparserplan.cpp"
#if YYERROR_VERBOSE

/* Return YYSTR after stripping away unnecessary quotes and
   backslashes, so that it's suitable for yyerror.  The heuristic is
   that double-quoting is unnecessary unless the string contains an
   apostrophe, a comma, or backslash (other than backslash-backslash).
   YYSTR is taken from yytname.  */
std::string
HolidayParserPlan::yytnamerr_(const char *yystr)
{
    if (*yystr == '"') {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
            switch (*++yyp) {
            case '\'':
            case ',':
                goto do_not_strip_quotes;

            case '\\':
                if (*++yyp != '\\') {
                    goto do_not_strip_quotes;
                }
            /* Fall through.  */
            default:
                yyr += *yyp;
                break;

            case '"':
                return yyr;
            }
    do_not_strip_quotes:;
    }

    return yystr;
}

#endif

/// Build a parser object.
HolidayParserPlan::HolidayParserPlan(class HolidayParserDriverPlan &driver_yyarg)
    :
#if YYDEBUG
    yydebug_(false),
    yycdebug_(&std::cerr),
#endif
    driver(driver_yyarg)
{
}

HolidayParserPlan::~HolidayParserPlan()
{
}

#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

inline void
HolidayParserPlan::yy_symbol_value_print_(int yytype,
        const semantic_type *yyvaluep, const location_type *yylocationp)
{
    YYUSE(yylocationp);
    YYUSE(yyvaluep);
    switch (yytype) {
    default:
        break;
    }
}

void
HolidayParserPlan::yy_symbol_print_(int yytype,
                                    const semantic_type *yyvaluep, const location_type *yylocationp)
{
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
               << ' ' << yytname_[yytype] << " ("
               << *yylocationp << ": ";
    yy_symbol_value_print_(yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
}
#endif

void
HolidayParserPlan::yydestruct_(const char *yymsg,
                               int yytype, semantic_type *yyvaluep, location_type *yylocationp)
{
    YYUSE(yylocationp);
    YYUSE(yymsg);
    YYUSE(yyvaluep);

    YY_SYMBOL_PRINT(yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype) {
    case 6: /* "STRING" */

        /* Line 480 of lalr1.cc  */
#line 96 "holidayparserplan.ypp"
        {
            free((yyvaluep->sval));
        };

        /* Line 480 of lalr1.cc  */
#line 259 "holidayparserplan.cpp"
        break;

    default:
        break;
    }
}

void
HolidayParserPlan::yypop_(unsigned int n)
{
    yystate_stack_.pop(n);
    yysemantic_stack_.pop(n);
    yylocation_stack_.pop(n);
}

#if YYDEBUG
std::ostream &
HolidayParserPlan::debug_stream() const
{
    return *yycdebug_;
}

void
HolidayParserPlan::set_debug_stream(std::ostream &o)
{
    yycdebug_ = &o;
}

HolidayParserPlan::debug_level_type
HolidayParserPlan::debug_level() const
{
    return yydebug_;
}

void
HolidayParserPlan::set_debug_level(debug_level_type l)
{
    yydebug_ = l;
}
#endif

int
HolidayParserPlan::parse()
{
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    /* State.  */
    int yyn;
    int yylen = 0;
    int yystate = 0;

    /* Error handling.  */
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    semantic_type yylval;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[2];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    YYCDEBUG << "Starting parse" << std::endl;

    /* User initialization code.  */

    /* Line 553 of lalr1.cc  */
#line 70 "holidayparserplan.ypp"
    {
        // initialize the initial location object
        yylloc.begin.filename = driver.fileToParse();
        yylloc.end.filename = yylloc.begin.filename;
    }

    /* Line 553 of lalr1.cc  */
#line 346 "holidayparserplan.cpp"

    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type(0);
    yysemantic_stack_ = semantic_stack_type(0);
    yylocation_stack_ = location_stack_type(0);
    yysemantic_stack_.push(yylval);
    yylocation_stack_.push(yylloc);

    /* New state.  */
yynewstate:
    yystate_stack_.push(yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_) {
        goto yyacceptlab;
    }

    goto yybackup;

    /* Backup.  */
yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yyn == yypact_ninf_) {
        goto yydefault;
    }

    /* Read a lookahead token.  */
    if (yychar == yyempty_) {
        YYCDEBUG << "Reading a token: ";
        yychar = yylex(&yylval, &yylloc);
    }

    /* Convert token to internal form.  */
    if (yychar <= yyeof_) {
        yychar = yytoken = yyeof_;
        YYCDEBUG << "Now at end of input." << std::endl;
    } else {
        yytoken = yytranslate_(yychar);
        YY_SYMBOL_PRINT("Next token is", yytoken, &yylval, &yylloc);
    }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken) {
        goto yydefault;
    }

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0) {
        if (yyn == 0 || yyn == yytable_ninf_) {
            goto yyerrlab;
        }
        yyn = -yyn;
        goto yyreduce;
    }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push(yylval);
    yylocation_stack_.push(yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_) {
        --yyerrstatus_;
    }

    yystate = yyn;
    goto yynewstate;

    /*-----------------------------------------------------------.
    | yydefault -- do the default action for the current state.  |
    `-----------------------------------------------------------*/
yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0) {
        goto yyerrlab;
    }
    goto yyreduce;

    /*-----------------------------.
    | yyreduce -- Do a reduction.  |
    `-----------------------------*/
yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen) {
        yyval = yysemantic_stack_[yylen - 1];
    } else {
        yyval = yysemantic_stack_[0];
    }

    {
        slice<location_type, location_stack_type> slice(yylocation_stack_, yylen);
        YYLLOC_DEFAULT(yyloc, slice, yylen);
    }
    YY_REDUCE_PRINT(yyn);
    switch (yyn) {
    case 4:

        /* Line 678 of lalr1.cc  */
#line 132 "holidayparserplan.ypp"
        {
            driver.setFileCountryCode(QString());
        }
        break;

    case 5:

        /* Line 678 of lalr1.cc  */
#line 133 "holidayparserplan.ypp"
        {
            char *s = (yysemantic_stack_[(2) - (2)].sval);
            driver.setFileCountryCode(QString::fromUtf8(s));
            free(s);
            (yysemantic_stack_[(2) - (2)].sval) = nullptr;
        }
        break;

    case 6:

        /* Line 678 of lalr1.cc  */
#line 136 "holidayparserplan.ypp"
        {
            driver.setFileLanguageCode(QString());
        }
        break;

    case 7:

        /* Line 678 of lalr1.cc  */
#line 137 "holidayparserplan.ypp"
        {
            char *s = (yysemantic_stack_[(2) - (2)].sval);
            driver.setFileLanguageCode(QString::fromUtf8(s));
            free(s);
            (yysemantic_stack_[(2) - (2)].sval) = nullptr;
        }
        break;

    case 8:

        /* Line 678 of lalr1.cc  */
#line 140 "holidayparserplan.ypp"
        {
            driver.setFileName(QString());
        }
        break;

    case 9:

        /* Line 678 of lalr1.cc  */
#line 141 "holidayparserplan.ypp"
        {
            char *s = (yysemantic_stack_[(2) - (2)].sval);
            driver.setFileName(QString::fromUtf8(s));
            free(s);
            (yysemantic_stack_[(2) - (2)].sval) = nullptr;
        }
        break;

    case 10:

        /* Line 678 of lalr1.cc  */
#line 144 "holidayparserplan.ypp"
        {
            driver.setFileDescription(QString());
        }
        break;

    case 11:

        /* Line 678 of lalr1.cc  */
#line 145 "holidayparserplan.ypp"
        {
            char *s = (yysemantic_stack_[(2) - (2)].sval);
            driver.setFileDescription(QString::fromUtf8(s));
            free(s);
            (yysemantic_stack_[(2) - (2)].sval) = nullptr;
        }
        break;

    case 14:

        /* Line 678 of lalr1.cc  */
#line 152 "holidayparserplan.ypp"
        {
            char *s = (yysemantic_stack_[(1) - (1)].sval);
            driver.setEventName(QString::fromUtf8(s));
            free(s);
            (yysemantic_stack_[(1) - (1)].sval) = nullptr;
        }
        break;

    case 15:

        /* Line 678 of lalr1.cc  */
#line 155 "holidayparserplan.ypp"
        {
            driver.setEventCategory((yysemantic_stack_[(1) - (1)].sval));
        }
        break;

    case 16:

        /* Line 678 of lalr1.cc  */
#line 156 "holidayparserplan.ypp"
        {
            driver.setEventCategory((yysemantic_stack_[(2) - (2)].sval));
        }
        break;

    case 17:

        /* Line 678 of lalr1.cc  */
#line 159 "holidayparserplan.ypp"
        {
            driver.setEventCalendarType(QStringLiteral("gregorian"));
        }
        break;

    case 18:

        /* Line 678 of lalr1.cc  */
#line 160 "holidayparserplan.ypp"
        {
            driver.setEventCalendarType(QString::fromUtf8((yysemantic_stack_[(1) - (1)].sval)));
        }
        break;

    case 19:

        /* Line 678 of lalr1.cc  */
#line 163 "holidayparserplan.ypp"
        {
            driver.setFromEaster((yysemantic_stack_[(3) - (2)].ival), (yysemantic_stack_[(3) - (3)].ival));
        }
        break;

    case 20:

        /* Line 678 of lalr1.cc  */
#line 164 "holidayparserplan.ypp"
        {
            driver.setFromPascha((yysemantic_stack_[(3) - (2)].ival), (yysemantic_stack_[(3) - (3)].ival));
        }
        break;

    case 21:

        /* Line 678 of lalr1.cc  */
#line 165 "holidayparserplan.ypp"
        {
            driver.setFromDate((yysemantic_stack_[(4) - (2)].ival), (yysemantic_stack_[(4) - (3)].ival), (yysemantic_stack_[(4) - (4)].ival));
        }
        break;

    case 22:

        /* Line 678 of lalr1.cc  */
#line 166 "holidayparserplan.ypp"
        {
            driver.setFromWeekdayInMonth(1, (yysemantic_stack_[(3) - (1)].ival), 1, (yysemantic_stack_[(3) - (2)].ival), (yysemantic_stack_[(3) - (3)].ival));
        }
        break;

    case 23:

        /* Line 678 of lalr1.cc  */
#line 167 "holidayparserplan.ypp"
        {
            driver.setFromWeekdayInMonth((yysemantic_stack_[(4) - (1)].ival), (yysemantic_stack_[(4) - (2)].ival), 1, (yysemantic_stack_[(4) - (3)].ival), (yysemantic_stack_[(4) - (4)].ival));
        }
        break;

    case 24:

        /* Line 678 of lalr1.cc  */
#line 168 "holidayparserplan.ypp"
        {
            driver.setFromWeekdayInMonth((yysemantic_stack_[(6) - (1)].ival), (yysemantic_stack_[(6) - (2)].ival), (yysemantic_stack_[(6) - (4)].ival), (yysemantic_stack_[(6) - (5)].ival), (yysemantic_stack_[(6) - (6)].ival));
        }
        break;

    case 25:

        /* Line 678 of lalr1.cc  */
#line 169 "holidayparserplan.ypp"
        {
            driver.setFromRelativeWeekday((yysemantic_stack_[(5) - (2)].ival), (yysemantic_stack_[(5) - (1)].ival), (yysemantic_stack_[(5) - (4)].ival), (yysemantic_stack_[(5) - (5)].ival));
        }
        break;

    case 26:

        /* Line 678 of lalr1.cc  */
#line 172 "holidayparserplan.ypp"
        {
            (yyval.ival) =   0;
        }
        break;

    case 27:

        /* Line 678 of lalr1.cc  */
#line 173 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(2) - (2)].ival);
        }
        break;

    case 28:

        /* Line 678 of lalr1.cc  */
#line 174 "holidayparserplan.ypp"
        {
            (yyval.ival) = -(yysemantic_stack_[(2) - (2)].ival);
        }
        break;

    case 29:

        /* Line 678 of lalr1.cc  */
#line 177 "holidayparserplan.ypp"
        {
            (yyval.ival) = 0;
        }
        break;

    case 30:

        /* Line 678 of lalr1.cc  */
#line 178 "holidayparserplan.ypp"
        {
            (yyval.ival) = ((yysemantic_stack_[(4) - (2)].ival) << 8) | (yysemantic_stack_[(4) - (4)].ival);
        }
        break;

    case 31:

        /* Line 678 of lalr1.cc  */
#line 181 "holidayparserplan.ypp"
        {
            (yyval.ival) = 0;
        }
        break;

    case 32:

        /* Line 678 of lalr1.cc  */
#line 182 "holidayparserplan.ypp"
        {
            (yyval.ival) = (1 << (yysemantic_stack_[(1) - (1)].ival));
        }
        break;

    case 33:

        /* Line 678 of lalr1.cc  */
#line 183 "holidayparserplan.ypp"
        {
            (yyval.ival) = (1 << (yysemantic_stack_[(3) - (1)].ival)) | (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 34:

        /* Line 678 of lalr1.cc  */
#line 186 "holidayparserplan.ypp"
        {
            (yyval.ival) =  1;
        }
        break;

    case 35:

        /* Line 678 of lalr1.cc  */
#line 187 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(2) - (2)].ival);
        }
        break;

    case 36:

        /* Line 678 of lalr1.cc  */
#line 190 "holidayparserplan.ypp"
        {
            driver.setEventDate(-99999, (yysemantic_stack_[(3) - (3)].ival), (yysemantic_stack_[(3) - (1)].ival));
        }
        break;

    case 37:

        /* Line 678 of lalr1.cc  */
#line 191 "holidayparserplan.ypp"
        {
            driver.setEventDate(-99999, (yysemantic_stack_[(4) - (3)].ival), (yysemantic_stack_[(4) - (1)].ival));
        }
        break;

    case 38:

        /* Line 678 of lalr1.cc  */
#line 192 "holidayparserplan.ypp"
        {
            driver.setEventDate((yysemantic_stack_[(5) - (5)].ival), (yysemantic_stack_[(5) - (3)].ival), (yysemantic_stack_[(5) - (1)].ival));
        }
        break;

    case 39:

        /* Line 678 of lalr1.cc  */
#line 193 "holidayparserplan.ypp"
        {
            driver.setEventDate(-99999, (yysemantic_stack_[(3) - (1)].ival), (yysemantic_stack_[(3) - (3)].ival));
        }
        break;

    case 40:

        /* Line 678 of lalr1.cc  */
#line 194 "holidayparserplan.ypp"
        {
            driver.setEventDate((yysemantic_stack_[(5) - (5)].ival), (yysemantic_stack_[(5) - (1)].ival), (yysemantic_stack_[(5) - (3)].ival));
        }
        break;

    case 41:

        /* Line 678 of lalr1.cc  */
#line 195 "holidayparserplan.ypp"
        {
            driver.setEventDate(-99999, (yysemantic_stack_[(2) - (1)].ival), (yysemantic_stack_[(2) - (2)].ival));
        }
        break;

    case 42:

        /* Line 678 of lalr1.cc  */
#line 196 "holidayparserplan.ypp"
        {
            driver.setEventDate((yysemantic_stack_[(3) - (3)].ival), (yysemantic_stack_[(3) - (1)].ival), (yysemantic_stack_[(3) - (2)].ival));
        }
        break;

    case 43:

        /* Line 678 of lalr1.cc  */
#line 197 "holidayparserplan.ypp"
        {
            driver.setEventDate(-99999, (yysemantic_stack_[(2) - (2)].ival), (yysemantic_stack_[(2) - (1)].ival));
        }
        break;

    case 44:

        /* Line 678 of lalr1.cc  */
#line 198 "holidayparserplan.ypp"
        {
            driver.setEventDate((yysemantic_stack_[(3) - (3)].ival), (yysemantic_stack_[(3) - (2)].ival), (yysemantic_stack_[(3) - (1)].ival));
        }
        break;

    case 45:

        /* Line 678 of lalr1.cc  */
#line 199 "holidayparserplan.ypp"
        {
            driver.setEventDate((yysemantic_stack_[(4) - (4)].ival), (yysemantic_stack_[(4) - (3)].ival), (yysemantic_stack_[(4) - (1)].ival));
        }
        break;

    case 46:

        /* Line 678 of lalr1.cc  */
#line 200 "holidayparserplan.ypp"
        {
            driver.setEventDate((yysemantic_stack_[(1) - (1)].ival));
        }
        break;

    case 47:

        /* Line 678 of lalr1.cc  */
#line 203 "holidayparserplan.ypp"
        {
            char *s = (yysemantic_stack_[(1) - (1)].sval);
            (yyval.ival) = driver.julianDayFromEventName(s);
            free(s);
            (yysemantic_stack_[(1) - (1)].sval) = nullptr;
        }
        break;

    case 48:

        /* Line 678 of lalr1.cc  */
#line 204 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.julianDayFromEaster();
        }
        break;

    case 49:

        /* Line 678 of lalr1.cc  */
#line 205 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.julianDayFromPascha();
        }
        break;

    case 50:

        /* Line 678 of lalr1.cc  */
#line 206 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.julianDayFromMonthDay((yysemantic_stack_[(3) - (3)].ival), (yysemantic_stack_[(3) - (1)].ival));
        }
        break;

    case 51:

        /* Line 678 of lalr1.cc  */
#line 207 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.julianDayFromMonthDay((yysemantic_stack_[(4) - (3)].ival), (yysemantic_stack_[(4) - (1)].ival));
        }
        break;

    case 52:

        /* Line 678 of lalr1.cc  */
#line 208 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.julianDayFromMonthDay((yysemantic_stack_[(3) - (1)].ival), (yysemantic_stack_[(3) - (3)].ival));
        }
        break;

    case 53:

        /* Line 678 of lalr1.cc  */
#line 209 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.julianDayFromMonthDay((yysemantic_stack_[(2) - (2)].ival), (yysemantic_stack_[(2) - (1)].ival));
        }
        break;

    case 54:

        /* Line 678 of lalr1.cc  */
#line 210 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.julianDayFromMonthDay((yysemantic_stack_[(2) - (1)].ival), (yysemantic_stack_[(2) - (2)].ival));
        }
        break;

    case 55:

        /* Line 678 of lalr1.cc  */
#line 211 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.julianDayFromRelativeWeekday((yysemantic_stack_[(3) - (2)].ival), (yysemantic_stack_[(3) - (1)].ival), (yysemantic_stack_[(3) - (3)].ival));
        }
        break;

    case 56:

        /* Line 678 of lalr1.cc  */
#line 212 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.julianDayFromWeekdayInMonth((yysemantic_stack_[(4) - (1)].ival), (yysemantic_stack_[(4) - (2)].ival), (yysemantic_stack_[(4) - (4)].ival));
        }
        break;

    case 58:

        /* Line 678 of lalr1.cc  */
#line 216 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.adjustedMonthNumber((yysemantic_stack_[(1) - (1)].ival));
        }
        break;

    case 59:

        /* Line 678 of lalr1.cc  */
#line 219 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.adjustedMonthNumber((yysemantic_stack_[(1) - (1)].ival));
        }
        break;

    case 60:

        /* Line 678 of lalr1.cc  */
#line 222 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
        }
        break;

    case 61:

        /* Line 678 of lalr1.cc  */
#line 223 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) || (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 62:

        /* Line 678 of lalr1.cc  */
#line 224 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) && (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 63:

        /* Line 678 of lalr1.cc  */
#line 225 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) == (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 64:

        /* Line 678 of lalr1.cc  */
#line 226 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) != (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 65:

        /* Line 678 of lalr1.cc  */
#line 227 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) <= (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 66:

        /* Line 678 of lalr1.cc  */
#line 228 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) >= (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 67:

        /* Line 678 of lalr1.cc  */
#line 229 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) < (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 68:

        /* Line 678 of lalr1.cc  */
#line 230 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) > (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 69:

        /* Line 678 of lalr1.cc  */
#line 231 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) + (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 70:

        /* Line 678 of lalr1.cc  */
#line 232 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) - (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 71:

        /* Line 678 of lalr1.cc  */
#line 233 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) * (yysemantic_stack_[(3) - (3)].ival);
        }
        break;

    case 72:

        /* Line 678 of lalr1.cc  */
#line 234 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (3)].ival) ? (yysemantic_stack_[(3) - (1)].ival) / (yysemantic_stack_[(3) - (3)].ival) : 0;
        }
        break;

    case 73:

        /* Line 678 of lalr1.cc  */
#line 235 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (3)].ival) ? (yysemantic_stack_[(3) - (1)].ival) % (yysemantic_stack_[(3) - (3)].ival) : 0;
        }
        break;

    case 74:

        /* Line 678 of lalr1.cc  */
#line 236 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(5) - (1)].ival) ? (yysemantic_stack_[(5) - (3)].ival) : (yysemantic_stack_[(5) - (5)].ival);
        }
        break;

    case 75:

        /* Line 678 of lalr1.cc  */
#line 237 "holidayparserplan.ypp"
        {
            (yyval.ival) = !(yysemantic_stack_[(2) - (2)].ival);
        }
        break;

    case 76:

        /* Line 678 of lalr1.cc  */
#line 238 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (2)].ival);
        }
        break;

    case 77:

        /* Line 678 of lalr1.cc  */
#line 241 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(3) - (2)].ival);
        }
        break;

    case 78:

        /* Line 678 of lalr1.cc  */
#line 242 "holidayparserplan.ypp"
        {
            (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
        }
        break;

    case 80:

        /* Line 678 of lalr1.cc  */
#line 246 "holidayparserplan.ypp"
        {
            (yyval.ival) = -(yysemantic_stack_[(2) - (2)].ival);
        }
        break;

    case 81:

        /* Line 678 of lalr1.cc  */
#line 247 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.parseYear();
        }
        break;

    case 82:

        /* Line 678 of lalr1.cc  */
#line 248 "holidayparserplan.ypp"
        {
            (yyval.ival) = driver.isLeapYear((yysemantic_stack_[(2) - (2)].ival));
        }
        break;

        /* Line 678 of lalr1.cc  */
#line 990 "holidayparserplan.cpp"
    default:
        break;
    }
    YY_SYMBOL_PRINT("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_(yylen);
    yylen = 0;
    YY_STACK_PRINT();

    yysemantic_stack_.push(yyval);
    yylocation_stack_.push(yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
            && yycheck_[yystate] == yystate_stack_[0]) {
        yystate = yytable_[yystate];
    } else {
        yystate = yydefgoto_[yyn - yyntokens_];
    }
    goto yynewstate;

    /*------------------------------------.
    | yyerrlab -- here on detecting error |
    `------------------------------------*/
yyerrlab:
    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_) {
        ++yynerrs_;
        error(yylloc, yysyntax_error_(yylloc.begin.filename, yystate, yytoken));
    }

    yyerror_range[0] = yylloc;
    if (yyerrstatus_ == 3) {
        /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

        if (yychar <= yyeof_) {
            /* Return failure if at end of input.  */
            if (yychar == yyeof_) {
                YYABORT;
            }
        } else {
            yydestruct_("Error: discarding", yytoken, &yylval, &yylloc);
            yychar = yyempty_;
        }
    }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;

    /*---------------------------------------------------.
    | yyerrorlab -- error raised explicitly by YYERROR.  |
    `---------------------------------------------------*/
yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false) {
        goto yyerrorlab;
    }

    yyerror_range[0] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_(yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

    /*-------------------------------------------------------------.
    | yyerrlab1 -- common code for both syntax error and YYERROR.  |
    `-------------------------------------------------------------*/
yyerrlab1:
    yyerrstatus_ = 3;   /* Each real token shifted decrements this.  */

    for (;;) {
        yyn = yypact_[yystate];
        if (yyn != yypact_ninf_) {
            yyn += yyterror_;
            if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_) {
                yyn = yytable_[yyn];
                if (0 < yyn) {
                    break;
                }
            }
        }

        /* Pop the current state because it cannot handle the error token.  */
        if (yystate_stack_.height() == 1) {
            YYABORT;
        }

        yyerror_range[0] = yylocation_stack_[0];
        yydestruct_("Error: popping",
                    yystos_[yystate],
                    &yysemantic_stack_[0], &yylocation_stack_[0]);
        yypop_();
        yystate = yystate_stack_[0];
        YY_STACK_PRINT();
    }

    yyerror_range[1] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT(yyloc, (yyerror_range - 1), 2);
    yysemantic_stack_.push(yylval);
    yylocation_stack_.push(yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT("Shifting", yystos_[yyn],
                    &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
yyabortlab:
    yyresult = 1;
    goto yyreturn;

yyreturn:
    if (yychar != yyempty_) {
        yydestruct_("Cleanup: discarding lookahead", yytoken, &yylval, &yylloc);
    }

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_(yylen);
    while (yystate_stack_.height() != 1) {
        yydestruct_("Cleanup: popping",
                    yystos_[yystate_stack_[0]],
                    &yysemantic_stack_[0],
                    &yylocation_stack_[0]);
        yypop_();
    }

    return yyresult;
}

// Generate an error message.
std::string
HolidayParserPlan::yysyntax_error_(std::string *filename, int yystate, int tok)
{
    std::string res;
    YYUSE(yystate);
#if YYERROR_VERBOSE
    int yyn = yypact_[yystate];
    if (yypact_ninf_ < yyn && yyn <= yylast_) {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  */
        int yyxbegin = yyn < 0 ? -yyn : 0;

        /* Stay within bounds of both yycheck and yytname.  */
        int yychecklim = yylast_ - yyn + 1;
        int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
        int count = 0;
        for (int x = yyxbegin; x < yyxend; ++x)
            if (yycheck_[x + yyn] == x && x != yyterror_) {
                ++count;
            }

        // FIXME: This method of building the message is not compatible
        // with internationalization.  It should work like yacc.c does it.
        // That is, first build a string that looks like this:
        // "syntax error, unexpected %s or %s or %s"
        // Then, invoke YY_ on this string.
        // Finally, use the string as a format to output
        // yytname_[tok], etc.
        // Until this gets fixed, this message appears in English only.
        res = "syntax error, unexpected ";
        res += yytnamerr_(yytname_[tok]);
        if (count < 5) {
            count = 0;
            for (int x = yyxbegin; x < yyxend; ++x)
                if (yycheck_[x + yyn] == x && x != yyterror_) {
                    res += (!count++) ? ", expecting " : " or ";
                    res += yytnamerr_(yytname_[x]);
                    res += ". Filepath : " + *filename;
                }
        }
    } else
#endif
        res = YY_("syntax error");
    return res;
}

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
const signed char HolidayParserPlan::yypact_ninf_ = -70;
const short int
HolidayParserPlan::yypact_[] = {
    -24,    -1,    25,   -70,     1,   -70,   -70,    39,    42,    38,
    -70,    65,   -70,    64,    51,   -70,     5,   -70,    75,   -70,
    -70,   -70,   114,   -70,   -70,   -70,   129,   -70,     6,    30,
    30,    84,   134,   -70,    30,    58,     6,    -2,   -70,   134,
    134,    77,    63,   -70,    77,    77,   -70,   134,    85,   160,
    -70,    81,     6,     6,    12,   131,     6,   209,   209,   134,
    -70,    30,    43,   -70,   -70,    62,     6,   -70,   -70,   -70,
    54,    67,     6,     3,   134,   134,   134,   134,   134,   134,
    134,   134,   134,   134,   134,   134,   134,   134,   -70,    98,
    77,    71,   -70,    63,    77,     6,    70,   -70,   -70,   -70,
    209,    77,     6,   -70,     6,   -70,   -70,   102,    63,   251,
    251,   251,   251,   251,   251,   230,   251,   116,   116,    62,
    62,    62,   187,    86,    99,   -70,     6,    30,   -70,   -70,
    134,   -70,   -70,   -70,    63,    80,   134,    98,    98,   -70,
    77,   209,   -70,   -70,   -70,   -70,   -70,   -70
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error.  */
const unsigned char
HolidayParserPlan::yydefact_[] = {
    4,     0,     0,    12,     6,     5,     1,     2,     0,     8,
    14,     0,     7,     0,    10,    15,    17,     9,     0,     3,
    16,    18,     0,    11,    79,    59,    26,    81,     0,    26,
    26,     0,     0,    13,    26,     0,    57,    46,    78,     0,
    0,    34,     0,    82,    34,    34,    80,     0,     0,     0,
    60,    29,     0,    41,    26,     0,    43,    27,    28,     0,
    22,    26,    46,    19,    20,    75,     0,    47,    48,    49,
    0,     0,    57,    58,     0,     0,     0,     0,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,    77,    31,
    34,    39,    42,     0,    34,    59,    36,    57,    58,    44,
    35,    34,     0,    76,     0,    54,    53,     0,     0,    63,
    64,    65,    66,    67,    68,    61,    62,    70,    69,    71,
    72,    73,     0,    32,     0,    21,     0,    26,    23,    45,
    37,    25,    55,    52,     0,    50,     0,    31,    31,    40,
    34,    38,    56,    51,    74,    33,    30,    24
};

/* YYPGOTO[NTERM-NUM].  */
const signed char
HolidayParserPlan::yypgoto_[] = {
    -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,
    -70,   -70,   -19,   -70,   -69,   -16,    79,   -70,    -9,   111,
    -23,   -22,   -70
};

/* YYDEFGOTO[NTERM-NUM].  */
const signed char
HolidayParserPlan::yydefgoto_[] = {
    -1,     2,     3,     4,     9,    14,    19,     7,    11,    16,
    22,    33,    41,    90,   124,    60,    34,    70,    35,    97,
    49,    50,    38
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.  */
const signed char HolidayParserPlan::yytable_ninf_ = -59;
const short int
HolidayParserPlan::yytable_[] = {
    37,     1,    25,    54,    42,     5,    43,   106,   107,    24,
    44,    45,    20,    21,    53,    51,    57,    58,    27,    28,
    62,    93,    39,    40,    65,     6,    73,     8,    63,    64,
    91,    92,   -58,    98,    99,    94,   100,    31,    55,    71,
    39,    40,   101,   108,   102,    10,    96,    25,    12,    32,
    105,   109,   110,   111,   112,   113,   114,   115,   116,   117,
    118,   119,   120,   121,   122,    13,    24,    25,   145,   146,
    17,    98,    15,   129,   125,    27,    28,   -58,   128,    18,
    132,    23,   133,    55,   127,   131,    98,    46,    24,    25,
    66,    67,    52,    59,    31,    89,   103,    27,    28,   135,
    87,   104,    68,   123,   139,   126,    32,   141,   140,    69,
    130,   134,    98,   144,   138,   137,    31,    24,    25,    26,
    143,    61,     0,     0,   147,   142,    27,    28,    32,     0,
    0,    29,    24,    36,    24,    95,     0,    24,    30,    39,
    40,    27,    28,    27,    28,    31,    27,    28,    56,    84,
    85,    86,     0,    36,    87,     0,     0,    32,     0,    72,
    31,     0,    31,     0,     0,    31,     0,     0,     0,     0,
    47,     0,    32,    56,    32,    48,     0,    32,    74,    75,
    76,    77,    78,    79,     0,     0,     0,     0,     0,    80,
    81,    82,    83,    84,    85,    86,     0,     0,    87,     0,
    0,     0,     0,     0,    88,    74,    75,    76,    77,    78,
    79,     0,     0,     0,     0,     0,    80,    81,    82,    83,
    84,    85,    86,     0,     0,    87,   136,    74,    75,    76,
    77,    78,    79,     0,     0,     0,     0,     0,    80,    81,
    82,    83,    84,    85,    86,     0,     0,    87,    74,    75,
    76,    77,    78,    79,     0,     0,     0,     0,     0,     0,
    81,    82,    83,    84,    85,    86,     0,     0,    87,    74,
    75,    76,    77,    78,    79,     0,     0,     0,     0,     0,
    0,     0,    82,    83,    84,    85,    86,     0,     0,    87
};

/* YYCHECK.  */
const short int
HolidayParserPlan::yycheck_[] = {
    22,    25,     4,     5,    26,     6,    28,     4,     5,     3,
    29,    30,     7,     8,    36,    34,    39,    40,    12,    13,
    42,     9,    10,    11,    47,     0,    48,    26,    44,    45,
    52,    53,    34,    55,    56,    54,    59,    31,    40,    48,
    10,    11,    61,    40,    66,     6,    55,     4,     6,    43,
    72,    74,    75,    76,    77,    78,    79,    80,    81,    82,
    83,    84,    85,    86,    87,    27,     3,     4,   137,   138,
    6,    93,     7,    95,    90,    12,    13,    34,    94,    28,
    102,     6,   104,    40,    93,   101,   108,     3,     3,     4,
    5,     6,    34,    16,    31,    14,    42,    12,    13,   108,
    38,    34,    17,     5,   126,    34,    43,   130,   127,    24,
    40,     9,   134,   136,    15,    29,    31,     3,     4,     5,
    40,    42,    -1,    -1,   140,   134,    12,    13,    43,    -1,
    -1,    17,     3,    22,     3,     4,    -1,     3,    24,    10,
    11,    12,    13,    12,    13,    31,    12,    13,    37,    33,
    34,    35,    -1,    42,    38,    -1,    -1,    43,    -1,    48,
    31,    -1,    31,    -1,    -1,    31,    -1,    -1,    -1,    -1,
    36,    -1,    43,    62,    43,    41,    -1,    43,    18,    19,
    20,    21,    22,    23,    -1,    -1,    -1,    -1,    -1,    29,
    30,    31,    32,    33,    34,    35,    -1,    -1,    38,    -1,
    -1,    -1,    -1,    -1,    44,    18,    19,    20,    21,    22,
    23,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,
    33,    34,    35,    -1,    -1,    38,    39,    18,    19,    20,
    21,    22,    23,    -1,    -1,    -1,    -1,    -1,    29,    30,
    31,    32,    33,    34,    35,    -1,    -1,    38,    18,    19,
    20,    21,    22,    23,    -1,    -1,    -1,    -1,    -1,    -1,
    30,    31,    32,    33,    34,    35,    -1,    -1,    38,    18,
    19,    20,    21,    22,    23,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    31,    32,    33,    34,    35,    -1,    -1,    38
};

/* STOS_[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
const unsigned char
HolidayParserPlan::yystos_[] = {
    0,    25,    46,    47,    48,     6,     0,    52,    26,    49,
    6,    53,     6,    27,    50,     7,    54,     6,    28,    51,
    7,     8,    55,     6,     3,     4,     5,    12,    13,    17,
    24,    31,    43,    56,    61,    63,    64,    66,    67,    10,
    11,    57,    66,    66,    57,    57,     3,    36,    41,    65,
    66,    57,    34,    66,     5,    40,    64,    65,    65,    16,
    60,    61,    66,    60,    60,    65,     5,     6,    17,    24,
    62,    63,    64,    66,    18,    19,    20,    21,    22,    23,
    29,    30,    31,    32,    33,    34,    35,    38,    44,    14,
    58,    66,    66,     9,    57,     4,    63,    64,    66,    66,
    65,    57,    66,    42,    34,    66,     4,     5,    40,    65,
    65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
    65,    65,    65,     5,    59,    60,    34,    63,    60,    66,
    40,    60,    66,    66,     9,    63,    39,    29,    15,    66,
    57,    65,    63,    40,    65,    59,    59,    60
};

#if YYDEBUG
/* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
   to YYLEX-NUM.  */
const unsigned short int
HolidayParserPlan::yytoken_number_[] = {
    0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
    265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
    275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
    285,    45,    43,    42,    47,    37,    33,   286,    63,    58,
    46,    91,    93,    40,    41
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
const unsigned char
HolidayParserPlan::yyr1_[] = {
    0,    45,    46,    47,    48,    48,    49,    49,    50,    50,
    51,    51,    52,    52,    53,    54,    54,    55,    55,    56,
    56,    56,    56,    56,    56,    56,    57,    57,    57,    58,
    58,    59,    59,    59,    60,    60,    61,    61,    61,    61,
    61,    61,    61,    61,    61,    61,    61,    62,    62,    62,
    62,    62,    62,    62,    62,    62,    62,    63,    63,    64,
    65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
    65,    65,    65,    65,    65,    65,    65,    66,    66,    67,
    67,    67,    67
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
const unsigned char
HolidayParserPlan::yyr2_[] = {
    0,     2,     2,     4,     0,     2,     0,     2,     0,     2,
    0,     2,     0,     5,     1,     1,     2,     0,     1,     3,
    3,     4,     3,     4,     6,     5,     0,     2,     2,     0,
    4,     0,     1,     3,     0,     2,     3,     4,     5,     3,
    5,     2,     3,     2,     3,     4,     1,     1,     1,     1,
    3,     4,     3,     2,     2,     3,     4,     1,     1,     1,
    1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
    3,     3,     3,     3,     5,     2,     3,     3,     1,     1,
    2,     1,     2
};

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
const char *
const HolidayParserPlan::yytname_[] = {
    "END", "error", "$undefined", "NUMBER", "MONTH", "WDAY", "STRING",
    "CATEGORY", "CALENDAR", "INOP", "PLUS", "MINUS", "YEAR", "LEAPYEAR",
    "SHIFT", "IF", "LENGTH", "EASTER", "EQ", "NE", "LE", "GE", "LT", "GT",
    "PASCHA", "COUNTRY", "LANGUAGE", "NAME", "DESCRIPTION", "OR", "AND",
    "'-'", "'+'", "'*'", "'/'", "'%'", "'!'", "UMINUS", "'?'", "':'", "'.'",
    "'['", "']'", "'('", "')'", "$accept", "planfile", "metadata",
    "countrycode", "languagecode", "name", "description", "list",
    "eventname", "categories", "calendar", "eventrule", "offset",
    "conditionaloffset", "wdaycondition", "length", "date", "reldate",
    "month", "monthnumber", "expr", "pexpr", "number", nullptr
};
#endif

#if YYDEBUG
/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
const HolidayParserPlan::rhs_number_type
HolidayParserPlan::yyrhs_[] = {
    46,     0,    -1,    47,    52,    -1,    48,    49,    50,    51,
    -1,    -1,    25,     6,    -1,    -1,    26,     6,    -1,    -1,
    27,     6,    -1,    -1,    28,     6,    -1,    -1,    52,    53,
    54,    55,    56,    -1,     6,    -1,     7,    -1,    54,     7,
    -1,    -1,     8,    -1,    17,    57,    60,    -1,    24,    57,
    60,    -1,    61,    57,    58,    60,    -1,     5,    57,    60,
    -1,    66,     5,    57,    60,    -1,    66,     5,     9,    63,
    57,    60,    -1,     5,    66,    61,    57,    60,    -1,    -1,
    10,    65,    -1,    11,    65,    -1,    -1,    14,    59,    15,
    59,    -1,    -1,     5,    -1,     5,    29,    59,    -1,    -1,
    16,    65,    -1,    66,    40,    63,    -1,    66,    40,    63,
    40,    -1,    66,    40,    63,    40,    65,    -1,    63,    34,
    66,    -1,    63,    34,    66,    34,    66,    -1,    64,    66,
    -1,    64,    66,    66,    -1,    66,    64,    -1,    66,    64,
    66,    -1,    66,    40,     4,    66,    -1,    66,    -1,     6,
    -1,    17,    -1,    24,    -1,    66,    40,    63,    -1,    66,
    40,    63,    40,    -1,    63,    34,    66,    -1,    66,     4,
    -1,    64,    66,    -1,     5,    66,    66,    -1,    66,     5,
    9,    63,    -1,    64,    -1,    66,    -1,     4,    -1,    66,
    -1,    65,    29,    65,    -1,    65,    30,    65,    -1,    65,
    18,    65,    -1,    65,    19,    65,    -1,    65,    20,    65,
    -1,    65,    21,    65,    -1,    65,    22,    65,    -1,    65,
    23,    65,    -1,    65,    32,    65,    -1,    65,    31,    65,
    -1,    65,    33,    65,    -1,    65,    34,    65,    -1,    65,
    35,    65,    -1,    65,    38,    65,    39,    65,    -1,    36,
    65,    -1,    41,    62,    42,    -1,    43,    65,    44,    -1,
    67,    -1,     3,    -1,    31,     3,    -1,    12,    -1,    13,
    66,    -1
};

/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
const unsigned short int
HolidayParserPlan::yyprhs_[] = {
    0,     0,     3,     6,    11,    12,    15,    16,    19,    20,
    23,    24,    27,    28,    34,    36,    38,    41,    42,    44,
    48,    52,    57,    61,    66,    73,    79,    80,    83,    86,
    87,    92,    93,    95,    99,   100,   103,   107,   112,   118,
    122,   128,   131,   135,   138,   142,   147,   149,   151,   153,
    155,   159,   164,   168,   171,   174,   178,   183,   185,   187,
    189,   191,   195,   199,   203,   207,   211,   215,   219,   223,
    227,   231,   235,   239,   243,   249,   252,   256,   260,   262,
    264,   267,   269
};

/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
const unsigned char
HolidayParserPlan::yyrline_[] = {
    0,   126,   126,   129,   132,   133,   136,   137,   140,   141,
    144,   145,   148,   149,   152,   155,   156,   159,   160,   163,
    164,   165,   166,   167,   168,   169,   172,   173,   174,   177,
    178,   181,   182,   183,   186,   187,   190,   191,   192,   193,
    194,   195,   196,   197,   198,   199,   200,   203,   204,   205,
    206,   207,   208,   209,   210,   211,   212,   215,   216,   219,
    222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
    232,   233,   234,   235,   236,   237,   238,   241,   242,   245,
    246,   247,   248
};

// Print the state stack on the debug stream.
void
HolidayParserPlan::yystack_print_()
{
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin();
            i != yystate_stack_.end(); ++i) {
        *yycdebug_ << ' ' << *i;
    }
    *yycdebug_ << std::endl;
}

// Report on the debug stream that the rule \a yyrule is going to be reduced.
void
HolidayParserPlan::yy_reduce_print_(int yyrule)
{
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
        YY_SYMBOL_PRINT("   $" << yyi + 1 << " =",
                        yyrhs_[yyprhs_[yyrule] + yyi],
                        &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
                        &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
}
#endif // YYDEBUG

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
HolidayParserPlan::token_number_type
HolidayParserPlan::yytranslate_(int t)
{
    static
    const token_number_type
    translate_table[] = {
        0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,    36,     2,     2,     2,    35,     2,     2,
        43,    44,    33,    32,     2,    31,    40,    34,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,    39,     2,
        2,     2,     2,    38,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,    41,     2,    42,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
        2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
        5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
        15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
        25,    26,    27,    28,    29,    30,    37
    };
    if ((unsigned int) t <= yyuser_token_number_max_) {
        return translate_table[t];
    } else {
        return yyundef_token_;
    }
}

const int HolidayParserPlan::yyeof_ = 0;
const int HolidayParserPlan::yylast_ = 289;
const int HolidayParserPlan::yynnts_ = 23;
const int HolidayParserPlan::yyempty_ = -2;
const int HolidayParserPlan::yyfinal_ = 6;
const int HolidayParserPlan::yyterror_ = 1;
const int HolidayParserPlan::yyerrcode_ = 256;
const int HolidayParserPlan::yyntokens_ = 45;

const unsigned int HolidayParserPlan::yyuser_token_number_max_ = 286;
const HolidayParserPlan::token_number_type HolidayParserPlan::yyundef_token_ = 2;

/* Line 1054 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

} // KHolidays

/* Line 1054 of lalr1.cc  */
#line 1573 "holidayparserplan.cpp"

/* Line 1056 of lalr1.cc  */
#line 251 "holidayparserplan.ypp"

/*** Private Yacc callbacks and helper functions ***/

void KHolidays::HolidayParserPlan::error(const KHolidays::HolidayParserPlan::location_type &errorLocation, const std::string &errorMessage)
{
    driver.error(errorLocation, errorMessage.c_str());
}


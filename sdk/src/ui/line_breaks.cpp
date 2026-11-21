// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/line_breaks.h"
#include "ui/line_break_unicode.h"

#include <algorithm>
#include <array>
#include <iterator>

#include <unicode/uchar.h>
#include <unicode/uvernum.h>

#include "modules/skunicode/include/SkUnicode.h"
#include "src/core/SkUTF.h"

namespace Lingmo::ui {

namespace {

// Line break classes: the pair table's classes first, then those resolved
// outside it.
enum Class : uint8_t {
    OP, CL, CPc, QU, QU_Pi, QU_Pf, GL, NS, EX, SY, IS, PR, PO, NU, AL, HL, ID, IN, HY, BA, BB, B2, ZW, CM, WJ,
    H2, H3, JL, JV, JT, RI, CB, EB, EM, ZWJ, SA, SG, SP, CR, LF, BK,
};

struct Properties
{
    Class cls;
    int8_t category;
    int8_t eastAsianWidth;
    bool pictographic;
};

// LB25: no break inside (PR|PO)?(OP|HY)?NU(NU|SY|IS)*(CL|CP)?(PR|PO)?.
namespace Number {

enum Action { None, Start, Continue, Break };
enum Kind { XX, PRPO, OPHY, NU, SYIS, CLCP };

} // namespace Number

// The pair table, after UAX #14 revision 37, with LB25 and LB21a
// tailored.
enum Action : uint8_t {
    PB, // prohibited
    DB, // direct
    IB, // indirect, after spaces only
    CI, // combining indirect
    CP, // combining prohibited
    HH, // prohibited after Hebrew and a hyphen
    IN_, // indirect unless the next is wide (LB30)
};

} // namespace

// ICU's (UAX #14's) classes resolved so: NL to BK, CJ to NS, AI, XX and the
// Aksara classes to AL, VI and VF to CM.
static Class classOf(UChar32 c)
{
    switch (u_getIntPropertyValue(c, UCHAR_LINE_BREAK)) {
    case U_LB_OPEN_PUNCTUATION: return OP;
    case U_LB_CLOSE_PUNCTUATION: return CL;
    case U_LB_CLOSE_PARENTHESIS: return CPc;
    case U_LB_QUOTATION: return QU;
    case U_LB_GLUE: return GL;
    case U_LB_NONSTARTER: return NS;
    case U_LB_CONDITIONAL_JAPANESE_STARTER: return NS;
    case U_LB_EXCLAMATION: return EX;
    case U_LB_BREAK_SYMBOLS: return SY;
    case U_LB_INFIX_NUMERIC: return IS;
    case U_LB_PREFIX_NUMERIC: return PR;
    case U_LB_POSTFIX_NUMERIC: return PO;
    case U_LB_NUMERIC: return NU;
    case U_LB_HEBREW_LETTER: return HL;
    case U_LB_IDEOGRAPHIC: return ID;
    case U_LB_INSEPARABLE: return IN;
    case U_LB_HYPHEN: return HY;
    case U_LB_BREAK_AFTER: return BA;
    case U_LB_BREAK_BEFORE: return BB;
    case U_LB_BREAK_BOTH: return B2;
    case U_LB_ZWSPACE: return ZW;
    case U_LB_COMBINING_MARK: return CM;
    case U_LB_WORD_JOINER: return WJ;
    case U_LB_H2: return H2;
    case U_LB_H3: return H3;
    case U_LB_JL: return JL;
    case U_LB_JV: return JV;
    case U_LB_JT: return JT;
    case U_LB_REGIONAL_INDICATOR: return RI;
    case U_LB_CONTINGENT_BREAK: return CB;
    case U_LB_E_BASE: return EB;
    case U_LB_E_MODIFIER: return EM;
    case U_LB_ZWJ: return ZWJ;
    case U_LB_COMPLEX_CONTEXT: return SA;
    case U_LB_SURROGATE: return SG;
    case U_LB_SPACE: return SP;
    case U_LB_CARRIAGE_RETURN: return CR;
    case U_LB_LINE_FEED: return LF;
    case U_LB_MANDATORY_BREAK: return BK;
    case U_LB_NEXT_LINE: return BK;
#if U_ICU_VERSION_MAJOR_NUM >= 74
    case U_LB_VIRAMA_FINAL: return CM;
    case U_LB_VIRAMA: return CM;
#endif
    default: return AL;
    }
}

static Properties lookUpProperties(UChar32 c)
{
    return {classOf(c), int8_t(u_charType(c)), int8_t(u_getIntPropertyValue(c, UCHAR_EAST_ASIAN_WIDTH)),
            bool(u_hasBinaryProperty(c, UCHAR_EXTENDED_PICTOGRAPHIC))};
}

static Properties propertiesOf(UChar32 c)
{
    // ASCII from a table: four ICU lookups a character add up in long lines.
    static const std::array<Properties, 128> ascii = [] {
        std::array<Properties, 128> table;
        for (UChar32 k = 0; k < 128; ++k)
            table[size_t(k)] = lookUpProperties(k);
        return table;
    }();
    return c >= 0 && c < 128 ? ascii[size_t(c)] : lookUpProperties(c);
}

// LB25's actions, by the number kinds before and after.
static const uint8_t numberActionTable[Number::CLCP + 1][Number::CLCP + 1] = {
//    XX                PRPO              OPHY              NU                SYIS              CLCP
    { Number::None    , Number::Start   , Number::Start   , Number::Start   , Number::None    , Number::None     }, // XX
    { Number::None    , Number::Start   , Number::Continue, Number::Continue, Number::None    , Number::None     }, // PRPO
    { Number::None    , Number::Start   , Number::Start   , Number::Continue, Number::None    , Number::None     }, // OPHY
    { Number::Break   , Number::Break   , Number::Break   , Number::Continue, Number::Continue, Number::Continue }, // NU
    { Number::Break   , Number::Break   , Number::Break   , Number::Continue, Number::Continue, Number::Continue }, // SYIS
    { Number::Break   , Number::Continue, Number::Break   , Number::Break   , Number::Break   , Number::Break    }, // CLCP
};

static Number::Kind numberKindOf(Class cls, int8_t category)
{
    switch (cls) {
    case AL:
        // AI math symbols in a number are IS.
        return category == U_MATH_SYMBOL ? Number::SYIS : Number::XX;
    case PR: case PO: return Number::PRPO;
    case OP: case HY: return Number::OPHY;
    case NU: return Number::NU;
    case SY: case IS: return Number::SYIS;
    case CL: case CPc: return Number::CLCP;
    default: return Number::XX;
    }
}

/* 1↓ 2→   OP  CL  CP  QU  +Pi +Pf GL  NS  EX  SY  IS  PR  PO  NU  AL  HL  ID  IN  HY  BA  BB  B2  ZW  CM  WJ  H2  H3  JL  JV  JT  RI  CB  EB  EM*/
static const uint8_t breakTable[ZWJ][ZWJ] = {
    /* OP */ { PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, CP, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB },
    /* CL */ { DB, PB, PB, IB, IB, PB, IB, PB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* CP */ { DB, PB, PB, IB, IB, PB, IB, PB, PB, PB, PB, DB, DB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* QU */ { IB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB },
    /* +Pi*/ { PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, CP, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB },
    /* +Pf*/ { IB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB },
    /* GL */ { IB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB },
    /* NS */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* EX */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* SY */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, DB, DB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* IS */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, DB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* PR */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, IB, IB, IB, IB, IB, IB, IB, DB, DB, PB, CI, PB, IB, IB, IB, IB, IB, DB, DB, IB, IB },
    /* PO */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* NU */ { IN_, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* AL */ { IN_, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* HL */ { IN_, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, DB, IB, CI, CI, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* ID */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* IN */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* HY */ { HH, PB, PB, IB, IB, PB, HH, IB, PB, PB, PB, HH, HH, IB, HH, HH, HH, IB, IB, IB, HH, HH, PB, CI, PB, HH, HH, HH, HH, HH, HH, DB, DB, DB },
    /* BA */ { HH, PB, PB, IB, IB, PB, HH, IB, PB, PB, PB, HH, HH, HH, HH, HH, HH, IB, IB, IB, HH, HH, PB, CI, PB, HH, HH, HH, HH, HH, HH, DB, DB, DB },
    /* BB */ { IB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB, IB, DB, IB, IB },
    /* B2 */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, IB, DB, PB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* ZW */ { DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* CM */ { IB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* WJ */ { IB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB },
    /* H2 */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, IB, IB, DB, DB, DB, DB },
    /* H3 */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, IB, DB, DB, DB, DB },
    /* JL */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, IB, IB, IB, IB, DB, DB, DB, DB, DB },
    /* JV */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, IB, IB, DB, DB, DB, DB },
    /* JT */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, IB, DB, DB, DB, DB },
    /* RI */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, IB, DB, DB, DB },
    /* CB */ { DB, PB, PB, IB, IB, PB, IB, DB, PB, PB, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
    /* EB */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, IB },
    /* EM */ { DB, PB, PB, IB, IB, PB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
};


static std::vector<uint8_t> lineBreakAttributes(std::u16string_view text)
{
    const size_t length = text.size();
    std::vector<uint8_t> breaks(length + 1, 0);
    if (length == 0)
        return breaks;

    size_t numberStart = 0;
    Number::Kind numberLast = Number::XX;
    Class lcls = LF; // LB10 at the start
    Class cls = lcls;
    Properties lastProperties = propertiesOf(U'\n');

    for (size_t i = 0; i != length; ++i) {
        const size_t pos = i;
        UChar32 c = text[i];
        if (U16_IS_LEAD(c) && i + 1 != length && U16_IS_TRAIL(text[i + 1])) {
            c = U16_GET_SUPPLEMENTARY(c, text[i + 1]);
            ++i;
        }
        const Properties properties = propertiesOf(c);
        Class ncls = properties.cls;
        const auto isMark = [&properties] {
            return properties.category == U_NON_SPACING_MARK || properties.category == U_COMBINING_SPACING_MARK;
        };

        // Hangul tailoring, always on: Korean breaks at spaces (LB27).
        if ((ncls >= H2 && ncls <= JT) || (c >= 0x3130 && c <= 0x318F && ncls == ID)) {
            ncls = AL;
        } else {
            if (ncls == SA && isMark())
                ncls = CM;
            if (ncls == CM && (lcls == ZW || lcls >= SP))
                ncls = AL;
        }
        // LB1
        if (ncls == SA && isMark())
            ncls = CM;

        if (ncls == QU) {
            if (properties.category == U_INITIAL_PUNCTUATION) {
                // LB15a
                constexpr Class lb15a[] = {BK, CR, LF, OP, QU, QU_Pi, QU_Pf, GL, SP, ZW};
                if (std::find(std::begin(lb15a), std::end(lb15a), lcls) != std::end(lb15a))
                    ncls = QU_Pi;
            } else if (properties.category == U_FINAL_PUNCTUATION) {
                // LB15b
                Class nncls = LF;
                if (i + 1 < length) {
                    UChar32 next = text[i + 1];
                    if (U16_IS_LEAD(next) && i + 2 != length && U16_IS_TRAIL(text[i + 2]))
                        next = U16_GET_SUPPLEMENTARY(next, text[i + 2]);
                    nncls = classOf(next);
                }
                constexpr Class lb15b[] = {SP, GL, WJ, CL, QU, QU_Pi, QU_Pf, CPc, EX, IS, SY, BK, CR, LF, ZW};
                if (std::find(std::begin(lb15b), std::end(lb15b), nncls) != std::end(lb15b))
                    ncls = QU_Pf;
            }
        }

        bool updateClass = true;
        if (lcls >= CR) {
            // LB4, LB5
            if (lcls > CR || ncls != LF)
                breaks[pos] = 1;
            if (ncls == CM || ncls == ZWJ) {
                cls = AL;
                updateClass = false;
            }
        } else if (ncls >= SP) {
            // LB6: × (BK|CR|LF|NL); LB7: × SP
            updateClass = ncls > SP;
        } else if ((ncls == CM || ncls == ZWJ) && lcls != ZW && lcls < SP) {
            // LB9
            updateClass = false;
        } else if (lcls == ZWJ) {
            // LB8a
        } else {
            // LB25
            const Number::Kind current = numberKindOf(ncls, properties.category);
            switch (numberActionTable[numberLast][current]) {
            case Number::Break:
                for (size_t j = numberStart + 1; j < pos; ++j)
                    breaks[j] = 0;
                [[fallthrough]];
            case Number::None:
                numberLast = Number::XX;
                break;
            case Number::Start:
                numberStart = i;
                [[fallthrough]];
            default:
                numberLast = current;
                break;
            }

            if (ncls == RI && lcls == RI) {
                // LB30a
                ncls = SP;
            } else if (ncls == EM && lastProperties.category == U_UNASSIGNED && lastProperties.pictographic) {
                // LB30b
            } else {
                if (cls >= SA)
                    cls = AL;
                Class tcls = cls;
                if (tcls == CM || tcls == ZWJ)
                    tcls = AL; // LB10
                switch (breakTable[tcls][ncls < ZWJ ? ncls : AL]) {
                case DB:
                    breaks[pos] = 1;
                    break;
                case IB:
                    if (lcls == SP)
                        breaks[pos] = 1;
                    break;
                case CI:
                    if (lcls != SP)
                        updateClass = false;
                    else
                        breaks[pos] = 1;
                    break;
                case CP:
                    if (lcls != SP)
                        updateClass = false;
                    break;
                case HH:
                    if (lcls != HL)
                        breaks[pos] = 1;
                    break;
                case IN_:
                    if (properties.eastAsianWidth == U_EA_FULLWIDTH || properties.eastAsianWidth == U_EA_WIDE
                        || properties.eastAsianWidth == U_EA_HALFWIDTH || lcls == SP)
                        breaks[pos] = 1;
                    break;
                default:
                    break;
                }
            }
        }
        if (updateClass) {
            cls = ncls;
            lastProperties = properties;
        }
        lcls = ncls;
    }

    if (numberActionTable[numberLast][Number::XX] == Number::Break) {
        for (size_t j = numberStart + 1; j < length; ++j)
            breaks[j] = 0;
    }
    breaks[0] = 0; // LB2
    breaks[length] = 1; // LB3
    return breaks;
}

// White space but no-break space: the white space a line wraps at.
static bool isBreakableSpace(char16_t c)
{
    if (c < 0x80)
        return c == 0x20 || (c >= 0x09 && c <= 0x0d);
    const bool space = c == 0x20 || (c >= 0x09 && c <= 0x0d) || c == 0x85 || u_charType(c) == U_SPACE_SEPARATOR
            || u_charType(c) == U_LINE_SEPARATOR || u_charType(c) == U_PARAGRAPH_SEPARATOR;
    return space && u_getIntPropertyValue(c, UCHAR_DECOMPOSITION_TYPE) != U_DT_NOBREAK;
}

BreakAttributes breakAttributes(std::u16string_view text)
{
    BreakAttributes attributes;
    attributes.lineBreak = lineBreakAttributes(text);
    attributes.breakableSpace.resize(text.size() + 1, 0);
    for (size_t i = 0; i < text.size(); ++i)
        attributes.breakableSpace[i] = isBreakableSpace(text[i]);
    return attributes;
}

std::vector<uint8_t> lineBreaks(std::u16string_view text)
{
    BreakAttributes attributes = breakAttributes(text);
    // A line also ends after any white space but a no-break one, whatever
    // UAX #14 says: "a WireGuard .conf" may break before ".conf".
    for (size_t i = 1; i < text.size(); ++i) {
        if (attributes.breakableSpace[i - 1] && !attributes.breakableSpace[i])
            attributes.lineBreak[i] = 1;
    }
    return attributes.lineBreak;
}

namespace {

class LineBreakUnicode : public SkUnicode
{
public:
    explicit LineBreakUnicode(sk_sp<SkUnicode> icu)
        : m_icu(std::move(icu))
    {
    }

    SkString toUpper(const SkString& text) override { return m_icu->toUpper(text); }
    SkString toUpper(const SkString& text, const char* locale) override { return m_icu->toUpper(text, locale); }
    bool isControl(SkUnichar c) override { return m_icu->isControl(c); }
    bool isWhitespace(SkUnichar c) override { return m_icu->isWhitespace(c); }
    bool isSpace(SkUnichar c) override { return m_icu->isSpace(c); }
    bool isTabulation(SkUnichar c) override { return m_icu->isTabulation(c); }
    bool isHardBreak(SkUnichar c) override { return m_icu->isHardBreak(c); }
    bool isEmoji(SkUnichar c) override { return m_icu->isEmoji(c); }
    bool isEmojiComponent(SkUnichar c) override { return m_icu->isEmojiComponent(c); }
    bool isEmojiModifierBase(SkUnichar c) override { return m_icu->isEmojiModifierBase(c); }
    bool isEmojiModifier(SkUnichar c) override { return m_icu->isEmojiModifier(c); }
    bool isRegionalIndicator(SkUnichar c) override { return m_icu->isRegionalIndicator(c); }
    bool isIdeographic(SkUnichar c) override { return m_icu->isIdeographic(c); }

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction direction) override
    {
        return m_icu->makeBidiIterator(text, count, direction);
    }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[], int count,
                                                     SkBidiIterator::Direction direction) override
    {
        return m_icu->makeBidiIterator(text, count, direction);
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[], BreakType type) override
    {
        return m_icu->makeBreakIterator(locale, type);
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType type) override
    {
        return m_icu->makeBreakIterator(type);
    }

    bool getBidiRegions(const char utf8[], int utf8Units, TextDirection direction,
                        std::vector<BidiRegion>* results) override
    {
        return m_icu->getBidiRegions(utf8, utf8Units, direction, results);
    }
    bool getWords(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override
    {
        return m_icu->getWords(utf8, utf8Units, locale, results);
    }
    bool getUtf8Words(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override
    {
        return m_icu->getUtf8Words(utf8, utf8Units, locale, results);
    }
    bool getSentences(const char utf8[], int utf8Units, const char* locale, std::vector<Position>* results) override
    {
        return m_icu->getSentences(utf8, utf8Units, locale, results);
    }

    bool computeCodeUnitFlags(char utf8[], int utf8Units, bool replaceTabs,
                              skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override
    {
        if (!m_icu->computeCodeUnitFlags(utf8, utf8Units, replaceTabs, results))
            return false;
        // The UTF-8 offset of every UTF-16 position.
        std::u16string utf16;
        std::vector<int> offsets;
        const char* current = utf8;
        const char* end = utf8 + utf8Units;
        while (current < end) {
            const int offset = int(current - utf8);
            SkUnichar c = SkUTF::NextUTF8(&current, end);
            if (c < 0)
                c = 0xFFFD;
            uint16_t units[2];
            const size_t count = SkUTF::ToUTF16(c, units);
            for (size_t i = 0; i < count; ++i) {
                utf16.push_back(char16_t(units[i]));
                offsets.push_back(offset);
            }
        }
        offsets.push_back(utf8Units);
        applyBreaks(lineBreaks(utf16), offsets, results);
        return true;
    }

    bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                              skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results) override
    {
        if (!m_icu->computeCodeUnitFlags(utf16, utf16Units, replaceTabs, results))
            return false;
        std::vector<int> offsets(size_t(utf16Units) + 1);
        for (int i = 0; i <= utf16Units; ++i)
            offsets[size_t(i)] = i;
        applyBreaks(lineBreaks(std::u16string_view(utf16, size_t(utf16Units))), offsets, results);
        return true;
    }

    void reorderVisual(const BidiLevel runLevels[], int levelsCount, int32_t logicalFromVisual[]) override
    {
        m_icu->reorderVisual(runLevels, levelsCount, logicalFromVisual);
    }

private:
    // Soft breaks inside the text from the pair table; the ends and the hard
    // breaks stay ICU's.
    static void applyBreaks(const std::vector<uint8_t>& breaks, const std::vector<int>& offsets,
                            skia_private::TArray<SkUnicode::CodeUnitFlags, true>* results)
    {
        const int last = offsets.back();
        for (int i = 1; i < last && i < results->size(); ++i)
            (*results)[i] = SkUnicode::CodeUnitFlags((*results)[i] & ~SkUnicode::kSoftLineBreakBefore);
        for (size_t i = 1; i + 1 < breaks.size(); ++i) {
            const int offset = offsets[i];
            // A break inside a surrogate pair has no UTF-8 position of its own.
            if (!breaks[i] || offset == offsets[i - 1] || offset >= results->size())
                continue;
            if (!((*results)[offset] & SkUnicode::kHardLineBreakBefore))
                (*results)[offset] = SkUnicode::CodeUnitFlags((*results)[offset] | SkUnicode::kSoftLineBreakBefore);
        }
    }

    sk_sp<SkUnicode> m_icu;
};

} // namespace

sk_sp<SkUnicode> makeLineBreakUnicode(sk_sp<SkUnicode> icu)
{
    return sk_make_sp<LineBreakUnicode>(std::move(icu));
}

} // namespace Lingmo::ui

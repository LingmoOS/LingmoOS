/******************************************************************

              Copyright 1993 by SunSoft, Inc.
              Copyright 1999-2000 by Bruno Haible

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the names of SunSoft, Inc. and
Bruno Haible not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.  SunSoft, Inc. and Bruno Haible make no representations
about the suitability of this software for any purpose.  It is
provided "as is" without express or implied warranty.

SunSoft Inc. AND Bruno Haible DISCLAIM ALL WARRANTIES WITH REGARD
TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL SunSoft, Inc. OR Bruno Haible BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/*
 * This file contains:
 *
 * I. Conversion routines CompoundText/CharSet <--> Unicode/UTF-8.
 *
 *    Used for three purposes:
 *      1. The UTF-8 locales, see below.
 *      2. Unicode aware applications for which the use of 8-bit character
 *         sets is an anachronism.
 *      3. For conversion from keysym to locale encoding.
 *
 * II. Conversion files for an UTF-8 locale loader.
 *     Supports: all locales with codeset UTF-8.
 *     How: Provides converters for UTF-8.
 *     Platforms: all systems.
 *
 * The loader itself is located in lcUTF8.c.
 */

/*
 * The conversion from UTF-8 to CompoundText is realized in a very
 * conservative way. Recall that CompoundText data is used for inter-client
 * communication purposes. We distinguish three classes of clients:
 * - Clients which accept only those pieces of CompoundText which belong to
 *   the character set understood by the current locale.
 *   (Example: clients which are linked to an older X11 library.)
 * - Clients which accept CompoundText with multiple character sets and parse
 *   it themselves.
 *   (Example: emacs, xemacs.)
 * - Clients which rely entirely on the X{mb,wc}TextPropertyToTextList
 *   functions for the conversion of CompoundText to their current locale's
 *   multi-byte/wide-character format.
 * For best interoperation, the UTF-8 to CompoundText conversion proceeds as
 * follows. For every character, it first tests whether the character is
 * representable in the current locale's original (non-UTF-8) character set.
 * If not, it goes through the list of predefined character sets for
 * CompoundText and tests if the character is representable in that character
 * set. If so, it encodes the character using its code within that character
 * set. If not, it uses an UTF-8-in-CompoundText encapsulation. Since
 * clients of the first and second kind ignore such encapsulated text,
 * this encapsulation is kept to a minimum and terminated as early as possible.
 *
 * In a distant future, when clients of the first and second kind will have
 * disappeared, we will be able to stuff UTF-8 data directly in CompoundText
 * without first going through the list of predefined character sets.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "XlcPubI.h"

/* Replacement character for invalid multibyte sequence or wide character. */
#define BAD_WCHAR ((ucs4_t) 0xfffd)
#define BAD_CHAR '?'

/***************************************************************************/
/* Part I: Conversion routines CompoundText/CharSet <--> Unicode/UTF-8.
 *
 * Note that this code works in any locale. We store Unicode values in
 * `ucs4_t' variables, but don't pass them to the user.
 *
 * This code has to support all character sets that are used for CompoundText,
 * nothing more, nothing less. See the table in lcCT.c.
 * Since the conversion _to_ CompoundText is likely to need the tables for all
 * character sets at once, we don't use dynamic loading (of tables or shared
 * libraries through iconv()). Use a fixed set of tables instead.
 *
 * We use statically computed tables, not dynamically allocated arrays,
 * because it's more memory efficient: Different processes using the same
 * libX11 shared library share the "text" and read-only "data" sections.
 */

typedef unsigned int ucs4_t;
#define conv_t XlcConv

typedef struct _Utf8ConvRec {
    const char *name;
    int (* cstowc) (ucs4_t *, unsigned char const *, int);
    int (* wctocs) (unsigned char *, ucs4_t, int);
} Utf8ConvRec, *Utf8Conv;

/*
 * int xxx_cstowc (XlcConv conv, ucs4_t *pwc, unsigned char const *s, int n)
 * converts the byte sequence starting at s to a wide character. Up to n bytes
 * are available at s. n is >= 1.
 * Result is number of bytes consumed (if a wide character was read),
 * or 0 if invalid, or -1 if n too small.
 *
 * int xxx_wctocs (XlcConv conv, unsigned char *r, ucs4_t wc, int n)
 * converts the wide character wc to the character set xxx, and stores the
 * result beginning at r. Up to n bytes may be written at r. n is >= 1.
 * Result is number of bytes written, or 0 if invalid, or -1 if n too small.
 */

/* Return code if invalid. (xxx_mbtowc, xxx_wctomb) */
#define RET_ILSEQ      0
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n)  (-1-(n))
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
#define RET_TOOSMALL   -1

/*
 * The tables below are bijective. It would be possible to extend the
 * xxx_wctocs tables to do some transliteration (e.g. U+201C,U+201D -> 0x22)
 * but *only* with characters not contained in any other table, and *only*
 * when the current locale is not an UTF-8 locale.
 */

#include "lcUniConv/utf8.h"
#include "lcUniConv/ucs2be.h"
#ifdef notused
#include "lcUniConv/ascii.h"
#endif
#include "lcUniConv/iso8859_1.h"
#include "lcUniConv/iso8859_2.h"
#include "lcUniConv/iso8859_3.h"
#include "lcUniConv/iso8859_4.h"
#include "lcUniConv/iso8859_5.h"
#include "lcUniConv/iso8859_6.h"
#include "lcUniConv/iso8859_7.h"
#include "lcUniConv/iso8859_8.h"
#include "lcUniConv/iso8859_9.h"
#include "lcUniConv/iso8859_10.h"
#include "lcUniConv/iso8859_11.h"
#include "lcUniConv/iso8859_13.h"
#include "lcUniConv/iso8859_14.h"
#include "lcUniConv/iso8859_15.h"
#include "lcUniConv/iso8859_16.h"
#include "lcUniConv/iso8859_9e.h"
#include "lcUniConv/jisx0201.h"
#include "lcUniConv/tis620.h"
#include "lcUniConv/koi8_r.h"
#include "lcUniConv/koi8_u.h"
#include "lcUniConv/koi8_c.h"
#include "lcUniConv/armscii_8.h"
#include "lcUniConv/cp1133.h"
#include "lcUniConv/mulelao.h"
#include "lcUniConv/viscii.h"
#include "lcUniConv/tcvn.h"
#include "lcUniConv/georgian_academy.h"
#include "lcUniConv/georgian_ps.h"
#include "lcUniConv/cp1251.h"
#include "lcUniConv/cp1255.h"
#include "lcUniConv/cp1256.h"
#include "lcUniConv/tatar_cyr.h"

typedef struct {
    unsigned short indx; /* index into big table */
    unsigned short used; /* bitmask of used entries */
} Summary16;

#include "lcUniConv/gb2312.h"
#include "lcUniConv/jisx0208.h"
#include "lcUniConv/jisx0212.h"
#include "lcUniConv/ksc5601.h"
#include "lcUniConv/big5.h"
#include "lcUniConv/big5_emacs.h"
#include "lcUniConv/big5hkscs.h"
#include "lcUniConv/gbk.h"

static Utf8ConvRec all_charsets[] = {
    /* The ISO10646-1/UTF-8 entry occurs twice, once at the beginning
       (for lookup speed), once at the end (as a fallback).  */
    {   "ISO10646-1",
        utf8_mbtowc, utf8_wctomb
    },

    {   "ISO8859-1",
        iso8859_1_mbtowc, iso8859_1_wctomb
    },
    {   "ISO8859-2",
        iso8859_2_mbtowc, iso8859_2_wctomb
    },
    {   "ISO8859-3",
        iso8859_3_mbtowc, iso8859_3_wctomb
    },
    {   "ISO8859-4",
        iso8859_4_mbtowc, iso8859_4_wctomb
    },
    {   "ISO8859-5",
        iso8859_5_mbtowc, iso8859_5_wctomb
    },
    {   "ISO8859-6",
        iso8859_6_mbtowc, iso8859_6_wctomb
    },
    {   "ISO8859-7",
        iso8859_7_mbtowc, iso8859_7_wctomb
    },
    {   "ISO8859-8",
        iso8859_8_mbtowc, iso8859_8_wctomb
    },
    {   "ISO8859-9",
        iso8859_9_mbtowc, iso8859_9_wctomb
    },
    {   "ISO8859-10",
        iso8859_10_mbtowc, iso8859_10_wctomb
    },
    {   "ISO8859-11",
        iso8859_11_mbtowc, iso8859_11_wctomb
    },
    {   "ISO8859-13",
        iso8859_13_mbtowc, iso8859_13_wctomb
    },
    {   "ISO8859-14",
        iso8859_14_mbtowc, iso8859_14_wctomb
    },
    {   "ISO8859-15",
        iso8859_15_mbtowc, iso8859_15_wctomb
    },
    {   "ISO8859-16",
        iso8859_16_mbtowc, iso8859_16_wctomb
    },
    {   "JISX0201.1976-0",
        jisx0201_mbtowc, jisx0201_wctomb
    },
    {   "TIS620-0",
        tis620_mbtowc, tis620_wctomb
    },
    {   "GB2312.1980-0",
        gb2312_mbtowc, gb2312_wctomb
    },
    {   "JISX0208.1983-0",
        jisx0208_mbtowc, jisx0208_wctomb
    },
    {   "JISX0208.1990-0",
        jisx0208_mbtowc, jisx0208_wctomb
    },
    {   "JISX0212.1990-0",
        jisx0212_mbtowc, jisx0212_wctomb
    },
    {   "KSC5601.1987-0",
        ksc5601_mbtowc, ksc5601_wctomb
    },
    {   "KOI8-R",
        koi8_r_mbtowc, koi8_r_wctomb
    },
    {   "KOI8-U",
        koi8_u_mbtowc, koi8_u_wctomb
    },
    {   "KOI8-C",
        koi8_c_mbtowc, koi8_c_wctomb
    },
    {   "TATAR-CYR",
        tatar_cyr_mbtowc, tatar_cyr_wctomb
    },
    {   "ARMSCII-8",
        armscii_8_mbtowc, armscii_8_wctomb
    },
    {   "IBM-CP1133",
        cp1133_mbtowc, cp1133_wctomb
    },
    {   "MULELAO-1",
        mulelao_mbtowc, mulelao_wctomb
    },
    {   "VISCII1.1-1",
        viscii_mbtowc, viscii_wctomb
    },
    {   "TCVN-5712",
        tcvn_mbtowc, tcvn_wctomb
    },
    {   "GEORGIAN-ACADEMY",
        georgian_academy_mbtowc, georgian_academy_wctomb
    },
    {   "GEORGIAN-PS",
        georgian_ps_mbtowc, georgian_ps_wctomb
    },
    {   "ISO8859-9E",
        iso8859_9e_mbtowc, iso8859_9e_wctomb
    },
    {   "MICROSOFT-CP1251",
        cp1251_mbtowc, cp1251_wctomb
    },
    {   "MICROSOFT-CP1255",
        cp1255_mbtowc, cp1255_wctomb
    },
    {   "MICROSOFT-CP1256",
        cp1256_mbtowc, cp1256_wctomb
    },
    {   "BIG5-0",
        big5_mbtowc, big5_wctomb
    },
    {   "BIG5-E0",
        big5_0_mbtowc, big5_0_wctomb
    },
    {   "BIG5-E1",
        big5_1_mbtowc, big5_1_wctomb
    },
    {   "GBK-0",
        gbk_mbtowc, gbk_wctomb
    },
    {   "BIG5HKSCS-0",
        big5hkscs_mbtowc, big5hkscs_wctomb
    },

    /* The ISO10646-1/UTF-8 entry occurs twice, once at the beginning
       (for lookup speed), once at the end (as a fallback).  */
    {   "ISO10646-1",
        utf8_mbtowc, utf8_wctomb
    },

    /* Encoding ISO10646-1 for fonts means UCS2-like encoding
       so for conversion to FontCharSet we need this record */
    {   "ISO10646-1",
        ucs2be_mbtowc, ucs2be_wctomb
    }
};

#define charsets_table_size (sizeof(all_charsets)/sizeof(all_charsets[0]))
#define all_charsets_count  (charsets_table_size - 1)
#define ucs2_conv_index     (charsets_table_size - 1)

/* from XlcNCharSet to XlcNUtf8String */

int
cstoutf8(
    State state,
    void * *from,
    size_t *from_left,
    void * *to,
    size_t *to_left,
    XlcCharSet charset)
{
    const char *name;
    Utf8Conv convptr;
    int i;
    unsigned char const *src;
    unsigned char const *srcend;
    unsigned char *dst;
    unsigned char *dstend;
    int unconv_num;

    if (from == NULL || *from == NULL)
        return 0;

    name = charset->encoding_name;
    /* not charset->name because the latter has a ":GL"/":GR" suffix */

    for (convptr = all_charsets, i = all_charsets_count-1; i > 0; convptr++, i--)
        if (!strcmp(convptr->name, name))
            break;
    if (i == 0)
        return -1;

    src = (unsigned char const *) *from;
    srcend = src + *from_left;
    dst = (unsigned char *) *to;
    dstend = dst + *to_left;
    unconv_num = 0;

    while (src < srcend) {
        ucs4_t wc;
        int consumed;
        int count;

        consumed = convptr->cstowc(&wc, src, srcend-src);
        if (consumed == RET_ILSEQ)
            return -1;
        if (consumed == RET_TOOFEW(0))
            break;

        count = utf8_wctomb(dst, wc, dstend-dst);
        if (count == RET_TOOSMALL)
            break;
        if (count == RET_ILSEQ) {
            count = utf8_wctomb(dst, BAD_WCHAR, dstend-dst);
            if (count == RET_TOOSMALL)
                break;
            unconv_num++;
        }
        src += consumed;
        dst += count;
    }

    *from = (void *) src;
    *from_left = srcend - src;
    *to = (void *) dst;
    *to_left = dstend - dst;

    return unconv_num;
}

/*
 * Converts a Unicode character to an appropriate character set. The NULL
 * terminated array of preferred character sets is passed as first argument.
 * If successful, *charsetp is set to the character set that was used, and
 * *sidep is set to the character set side (XlcGL or XlcGR).
 */
static int
charset_wctocs(
    Utf8Conv *charsetp,
    XlcSide *sidep,
    unsigned char *r,
    ucs4_t wc,
    int n)
{
    int count;
    Utf8Conv convptr;
    int i;

    // First one is utf8. We are not able to fetch XLC_FONTSET like libX11, so
    // we just hard code utf8 as our preferred charset.
    Utf8Conv preferred_charset[] = {all_charsets, NULL};
    Utf8Conv* preferred = preferred_charset;
    for (; *preferred != (Utf8Conv) NULL; preferred++) {
        convptr = *preferred;
        count = convptr->wctocs(r, wc, n);
        if (count == RET_TOOSMALL)
            return RET_TOOSMALL;
        if (count != RET_ILSEQ) {
            *charsetp = convptr;
            *sidep = (*r < 0x80 ? XlcGL : XlcGR);
            return count;
        }
    }

    for (convptr = all_charsets+1, i = all_charsets_count-1; i > 0; convptr++, i--) {
        count = convptr->wctocs(r, wc, n);
        if (count == RET_TOOSMALL)
            return RET_TOOSMALL;
        if (count != RET_ILSEQ) {
            *charsetp = convptr;
            *sidep = (*r < 0x80 ? XlcGL : XlcGR);
            return count;
        }
    }
    return RET_ILSEQ;
}

int
utf8tocs(
    State state,
    void **from,
    size_t *from_left,
    void **to,
    size_t *to_left,
    XlcCharSet* p_charset)
{
    XlcCharSet last_charset = NULL;
    unsigned char const *src;
    unsigned char const *srcend;
    unsigned char *dst;
    unsigned char *dstend;
    int unconv_num;

    if (from == NULL || *from == NULL)
        return 0;

    src = (unsigned char const *) *from;
    srcend = src + *from_left;
    dst = (unsigned char *) *to;
    dstend = dst + *to_left;
    unconv_num = 0;

    while (src < srcend && dst < dstend) {
        Utf8Conv chosen_charset = NULL;
        XlcSide chosen_side = XlcNONE;
        ucs4_t wc;
        int consumed;
        int count;

        consumed = utf8_mbtowc(&wc, src, srcend-src);
        if (consumed == RET_TOOFEW(0))
            break;
        if (consumed == RET_ILSEQ) {
            src++;
            unconv_num++;
            continue;
        }

        count = charset_wctocs(&chosen_charset, &chosen_side, dst, wc, dstend-dst);
        if (count == RET_TOOSMALL)
            break;
        if (count == RET_ILSEQ) {
            src += consumed;
            unconv_num++;
            continue;
        }

        if (last_charset == NULL) {
            last_charset =
                _XlcGetCharSetWithSide(chosen_charset->name, chosen_side);
            if (last_charset == NULL) {
                src += consumed;
                unconv_num++;
                continue;
            }
        } else {
            if (!(strcmp(last_charset->encoding_name, chosen_charset->name) == 0
                    && (last_charset->side == XlcGLGR
                        || last_charset->side == chosen_side)))
                break;
        }
        src += consumed;
        dst += count;
    }

    if (last_charset == NULL)
        return -1;

    *from = (void *) src;
    *from_left = srcend - src;
    *to = (void *) dst;
    *to_left = dstend - dst;

    if (p_charset)
        *p_charset = last_charset;

    return unconv_num;
}

/*
 * Copyright 1992, 1993 by TOSHIBA Corp.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of TOSHIBA not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. TOSHIBA make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * TOSHIBA DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * TOSHIBA BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Katsuhisa Yano	TOSHIBA Corp.
 *			   	mopi@osa.ilab.toshiba.co.jp
 */
/*
 * Copyright 1995 by FUJITSU LIMITED
 * This is source code modified by FUJITSU LIMITED under the Joint
 * Development Agreement for the CDE/Motif PST.
 *
 * Modifier: Takanori Tateno   FUJITSU LIMITED
 *
 */
/*
 * Most of this API is documented in i18n/Framework.PS
 */

#ifndef _XLCPUBLIC_H_
#define _XLCPUBLIC_H_

#include <stdbool.h>


/*
 * Character sets.
 */

/* Every character set has a "side". It denotes the range of byte values for
   which the character set is responsible. This means that the character
   set's encoded characters will only assumes bytes within the range, and
   that the character set can be used simultaneously with another character
   set responsible for a disjoint range. */
typedef enum {
    XlcUnknown,
    XlcC0,		/* responsible for values 0x00..0x1F */
    XlcGL,		/* responsible for values 0x00..0x7F or 0x20..0x7F */
    XlcC1,		/* responsible for values 0x80..0x9F */
    XlcGR,		/* responsible for values 0x80..0xFF or 0xA0..0xFF */
    XlcGLGR,		/* responsible for values 0x00..0xFF */
    XlcOther,		/* unused */
    XlcNONE
} XlcSide;

/* Where the character set comes from. */
typedef enum {
    CSsrcUndef,		/* unused */
    CSsrcStd,		/* defined in libX11 */
    CSsrcXLC		/* defined in an XLC_LOCALE file */
} CSSrc;

/* This is the structure of an XlcCharSet.
   Once allocated, they are never freed. */
typedef struct _XlcCharSetRec {
    /* Character set name, including side suffix */
    const char 		*name;

    /* XLFD encoding name, no side suffix */
    const char 		*encoding_name;

    /* Range for which the charset is responsible: XlcGL, XlcGR or XlcGLGR */
    XlcSide 		side;

    /* Number of bytes per character. 0 means a varying number (e.g. UTF-8) */
    int 		char_size;
    /* Classification of the character set according to ISO-2022 */
    int 		set_size;	/* e.g. 94 or 96 */
    const char 		*ct_sequence;	/* control sequence of CT */
					/* (normally at most 4 bytes) */

    /* Description source */
    CSSrc		source;
} XlcCharSetRec, *XlcCharSet;

/* Returns the charset with the given name (including side suffix).
   Returns NULL if not found. */
extern XlcCharSet _XlcGetCharSet(
    const char*		name
);

/* Returns the charset with the given encoding (no side suffix) and
   responsible for at least the given side (XlcGL or XlcGR).
   Returns NULL if not found. */
extern XlcCharSet _XlcGetCharSetWithSide(
    const char*		encoding_name,
    XlcSide		side
);

/* Registers an XlcCharSet in the list of character sets.
   Returns True if successful. */
extern bool _XlcAddCharSet(
    XlcCharSet		charset
);

/*
 * Structure representing the parse state of a Compound Text string.
 */
typedef struct _StateRec {
    XlcCharSet charset;     /* The charset of the current segment */
    XlcCharSet GL_charset;  /* The charset responsible for 0x00..0x7F */
    XlcCharSet GR_charset;  /* The charset responsible for 0x80..0xFF */
    XlcCharSet Other_charset;   /* != NULL if currently in an other segment */
    int ext_seg_left;       /* > 0 if currently in an extended segment */
} StateRec, *State;

#endif  /* _XLCPUBLIC_H_ */

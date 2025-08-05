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

#ifndef _XLCPUBLICI_H_
#define _XLCPUBLICI_H_

#include "XlcPublic.h" // IWYU pragma: export

/* Fills into a freshly created XlcCharSet the fields that can be inferred
   from the ESC sequence. These are side, char_size, set_size. */
extern bool _XlcParseCharSet(
    XlcCharSet		charset
);

/* Creates a new XlcCharSet, given its name (including side suffix) and
   Compound Text ESC sequence (normally at most 4 bytes). */
extern XlcCharSet _XlcCreateDefaultCharSet(
    const char*		name,
    const char*		ct_sequence
);

extern XlcCharSet _XlcAddCT(
    const char*		name,
    const char*		ct_sequence
);

extern bool _XlcInitCTInfo (void);


extern void
init_state(State state);

extern int
utf8tocs(
    State state,
    void **from,
    size_t *from_left,
    void **to,
    size_t *to_left,
    XlcCharSet* p_charset);

extern int
cstoct(
    State state,
    void * *from,
    size_t *from_left,
    void * *to,
    size_t *to_left,
    XlcCharSet charset);

extern int
cttocs(
    State state,
    void * *from,
    size_t *from_left,
    void * *to,
    size_t *to_left,
    XlcCharSet *p_charset);

extern int
cstoutf8(
    State state,
    void * *from,
    size_t *from_left,
    void * *to,
    size_t *to_left,
    XlcCharSet charset);

#endif  /* _XLCPUBLICI_H_ */

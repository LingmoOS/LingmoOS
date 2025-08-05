/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#ifndef _XCB_IMDKIT_ENCODING_H_
#define _XCB_IMDKIT_ENCODING_H_

#include "ximcommon.h"
#include <stdlib.h>

XCBIMDKIT_DECL_BEGIN

XCBIMDKIT_EXPORT void xcb_compound_text_init();
XCBIMDKIT_EXPORT char *
xcb_utf8_to_compound_text(const char *utf8, size_t length, size_t *lenghtOut);
XCBIMDKIT_EXPORT char *xcb_compound_text_to_utf8(const char *compound_text,
                                                 size_t length,
                                                 size_t *lenghtOut);

XCBIMDKIT_DECL_END

#endif // _XCB_IMDKIT_ENCODING_H_

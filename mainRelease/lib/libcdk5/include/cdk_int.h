/*
 * $Id: cdk_int.h,v 1.29 2021/08/25 23:34:40 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDK_INT_H
#define CDK_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cdk.h>

/*
 * Copyright 2003-2016,2021 Thomas E. Dickey
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, distribute with
 * modifications, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#define typeCallocN(type,n)     (type*)calloc((size_t)(n), sizeof(type))
#define typeCalloc(type)        typeCallocN(type,1)

#define typeReallocN(type,p,n)  (type*)realloc(p, (size_t)(n) * sizeof(type))

#define typeMallocN(type,n)     (type*)malloc((size_t)(n) * sizeof(type))
#define typeMalloc(type)        typeMallocN(type,1)

#define freeChecked(p)          if ((p) != 0) free (p)
#define freeAndNull(p)          if ((p) != 0) { free (p); p = 0; }

#define isChar(c)               ((int)(c) >= 0 && (int)(c) < KEY_MIN)
#define CharOf(c)               ((unsigned char)(c))

#define SIZEOF(v)               (sizeof(v)/sizeof((v)[0]))

#define MAX_COLORS		8

/*
 * Macros to check if caller is attempting to make the widget as high (or wide)
 * as the screen.
 */
#define isFullWidth(n)		((n) == FULL || (COLS != 0 && ((n) >= COLS)))
#define isFullHeight(n)		((n) == FULL || (LINES != 0 && ((n) >= LINES)))

/*
 * Hide details of modifying widget->exitType
 */
#define storeExitType(d)	ObjOf(d)->exitType = (d)->exitType
#define initExitType(d)		storeExitType(d) = vNEVER_ACTIVATED
#define setExitType(w,c)	setCdkExitType(ObjOf(w), &((w)->exitType), c)
#define copyExitType(d,s)	storeExitType(d) = ExitTypeOf(s)

/*
 * Use this if checkCDKObjectBind() returns true, use this function to
 * decide if the exitType should be set as a side-effect.
 */
#define checkEarlyExit(w)	if (EarlyExitOf(w) != vNEVER_ACTIVATED) \
				    storeExitType(w) = EarlyExitOf(w)

/*
 * Position within the data area of a widget, accounting for border and title.
 */
#define SCREEN_XPOS(w,n) ((n) + BorderOf(w))
#define SCREEN_YPOS(w,n) ((n) + BorderOf(w) + TitleLinesOf(w))

/*
 * Miscellaneous definitions.
 */
#define CDK_PATHMAX		256

#if !(defined(__hpux) && !defined(NCURSES_VERSION))
#define CDK_FILLED_BOX		ACS_CKBOARD
#else
#define CDK_FILLED_BOX		WACS_CKBOARD
#endif

extern char *GPasteBuffer;

#ifdef __cplusplus
}
#endif

#endif /* CDK_INT_H */
#endif /* CDKINCLUDES */

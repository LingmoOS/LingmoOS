/*
 * $Id: traverse.h,v 1.10 2005/12/30 01:09:09 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKTRAVERSE_H
#define CDKTRAVERSE_H 1

#include "cdk.h"

#ifndef CDK_H
#define CDKINCLUDES
#include <cdk.h>
#undef CDKINCLUDES
#include <binding.h>
#include <cdkscreen.h>
#include <cdk_objs.h>
#endif

/*
 * Copyright 1999-2004,2005 Thomas E. Dickey
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

typedef boolean (*CHECK_KEYCODE)(int /* keyCode */, int /* functionKey */);

extern CDKOBJS *getCDKFocusCurrent (CDKSCREEN * /* screen */);
extern CDKOBJS *setCDKFocusCurrent (CDKSCREEN * /*screen */, CDKOBJS * /* obj */);
extern CDKOBJS *setCDKFocusNext (CDKSCREEN * /* screen */);
extern CDKOBJS *setCDKFocusPrevious (CDKSCREEN * /* screen */);
extern CDKOBJS *setCDKFocusFirst (CDKSCREEN * /* screen */);
extern CDKOBJS *setCDKFocusLast (CDKSCREEN * /* screen */);

extern int  traverseCDKScreen (CDKSCREEN * /* screen */);

extern void exitCancelCDKScreen (CDKSCREEN * /* screen */);
extern void exitCancelCDKScreenOf (CDKOBJS * /* obj */);
extern void exitOKCDKScreen (CDKSCREEN * /* screen */);
extern void exitOKCDKScreenOf (CDKOBJS * /* obj */);
extern void resetCDKScreen (CDKSCREEN * /* screen */);
extern void resetCDKScreenOf (CDKOBJS * /* obj */);
extern void traverseCDKOnce (CDKSCREEN * /*screen */, CDKOBJS * /*curobj */, int /* keyCode */, boolean /* functionKey */, CHECK_KEYCODE /*funcMenuKey */);

#endif /* CDKTRAVERSE_H */
#endif /* CDKINCLUDES */

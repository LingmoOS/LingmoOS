/*
 * $Id: cdk_compat.h,v 1.5 2021/01/09 22:47:16 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDK_COMPAT_H
#define CDK_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Copyright 2004-2005,2021 Thomas E. Dickey
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

/*
 * Renaming of struct members was done for consistency.  Making old/new
 * versions compatible with renaming is awkward (there may be inadvertent
 * naming collisions), but this is mainly for demonstration purposes.
 */
#define info      list
#define infoSize  listSize
#define itemCount listSize
#define boxMatrix box

/*
 * Include the Cdk header file, which includes everything except for this file.
 */
#include <cdk.h>

/*
 * This is defined via the new header file:
 */
#ifdef CDK_VERSION_PATCH

/*
 * If we included the new Cdk header file, provide definitions for things
 * which have been made obsolete.
 */

#define MAX_LINES	5000

#define WIN_WIDTH(a)	getmaxx(a)
#define WIN_HEIGHT(a)	getmaxy(a)
#define WIN_XPOS(a)	getbegx(a)
#define WIN_YPOS(a)	getbegy(a)

extern int getDirectoryContents (char *directory, char **list, int maxListSize);
extern int readFile (char *filename, char **info, int maxlines);
extern int splitString (char *string, char **items, char splitChar);

#else

/*
 * Provide definitions to allow the applications using the old header to
 * compile using the new header's macros:
 */
#define ObjOf(a)	(a)
#define ScreenOf(a)	(a)

#endif	/* CDK_VERSION_PATCH */

#ifdef __cplusplus
}
#endif

#endif /* CDK_COMPAT_H */
#endif /* CDKINCLUDES */

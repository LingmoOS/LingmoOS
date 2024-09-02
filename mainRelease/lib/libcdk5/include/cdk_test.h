/*
 * $Id: cdk_test.h,v 1.7 2021/03/24 22:32:00 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDK_TEST_H
#define CDK_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cdk.h>

/*
 * Copyright 2005-2008,2021 Thomas E. Dickey
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
 * The whole point of this header is to define ExitProgram(), which is used for
 * leak-checking when ncurses's _nc_free_and_exit() function is available. 
 * Invoking that rather than 'exit()' tells ncurses to free all of the
 * "permanent" memory leaks, making analysis much simpler.
 */
#ifdef HAVE_NC_ALLOC_H

#ifndef HAVE_LIBDBMALLOC
#define HAVE_LIBDBMALLOC 0
#endif

#ifndef HAVE_LIBDMALLOC
#define HAVE_LIBDMALLOC 0
#endif

#ifndef HAVE_LIBMPATROL
#define HAVE_LIBMPATROL 0
#endif

#include <nc_alloc.h>

#else

#if defined(NCURSES_VERSION)
#ifdef HAVE_EXIT_CURSES
#define ExitProgram(code) exit_curses(code)
#elif defined(HAVE__NC_FREE_AND_EXIT)
/* nc_alloc.h normally not installed */
extern GCC_NORETURN void _nc_free_and_exit(int);
#define ExitProgram(code) _nc_free_and_exit(code)
#endif
#endif /* NCURSES_VERSION */

#endif /* HAVE_NC_ALLOC_H */

#ifndef ExitProgram
#define ExitProgram(code) exit(code)
#endif

#ifdef __cplusplus
}
#endif

#endif /* CDK_TEST_H */
#endif /* CDKINCLUDES */

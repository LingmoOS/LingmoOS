/*
 * $Id: cdkscreen.h,v 1.19 2016/11/20 14:42:21 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKSCREEN_H
#define CDKSCREEN_H	1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CDK_H
#define CDKINCLUDES
#include <cdk.h>
#undef CDKINCLUDES
#endif

/*
 * Changes 1999-2015,2016 copyright Thomas E. Dickey
 * Copyright 1999, Mike Glover
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
 * Declare and definitions needed for the widget.
 */
#define MAX_OBJECTS	1000	/* not used by widgets */

struct CDKOBJS;

typedef enum {
      CDKSCREEN_NOEXIT = 0
      , CDKSCREEN_EXITOK
      , CDKSCREEN_EXITCANCEL
} EExitStatus;

/*
 * Define the CDK screen structure.
 */
struct SScreen {
   WINDOW *		window;
   struct CDKOBJS **	object;
   int			objectCount;	/* last-used index in object[] */
   int			objectLimit;	/* sizeof(object[]) */
   EExitStatus		exitStatus;
   int			objectFocus;	/* focus index in object[] */
};
typedef struct SScreen CDKSCREEN;

/*
 * This function creates a CDKSCREEN pointer.
 */
CDKSCREEN *initCDKScreen (
		WINDOW *	/* window */);

/*
 * This sets which CDKSCREEN pointer will be the default screen
 * in the list of managed screen.
 */
CDKSCREEN *setDefaultCDKScreen (
		int		/* screenNumber */);

/*
 * This function registers a CDK widget with a given screen.
 */
void registerCDKObject (
		CDKSCREEN *	/* screen */,
		EObjectType	/* cdktype */,
		void *		/* object */);

/*
 * This function registers a CDK widget with it's former screen.
 */
void reRegisterCDKObject (
		EObjectType	/* cdktype */,
		void *		/* object */);

/*
 * This unregisters a CDK widget from a screen.
 */
void unregisterCDKObject (
		EObjectType	/* cdktype */,
		void *		/* object */);

/*
 * This function raises a widget on a screen to the top of the widget
 * stack of the screen the widget is associated with. The side effect
 * of doing this is the widget will be on 'top' of all the other
 * widgets on their screens.
 */
void raiseCDKObject (
		EObjectType	/* cdktype */,
		void *		/* object */);

/*
 * This function lowers a widget on a screen to the bottom of the widget
 * stack of the screen the widget is associated with. The side effect
 * of doing this is that all the other widgets will be on 'top' of the
 * widget on their screens.
 */
void lowerCDKObject (
		EObjectType	/* cdktype */,
		void *		/* object */);

/*
 * This redraws a window, forcing it to the top of the stack.
 */
void refreshCDKWindow (
		WINDOW *	/* win */);

/*
 * This redraws all the widgets associated with the given screen.
 */
void refreshCDKScreen (
		CDKSCREEN *	/* screen */);

/*
 * This calls refreshCDKScreen. (it is here to try to be consistent
 * with the drawCDKXXX functions associated with the widgets)
 */
void drawCDKScreen (
		CDKSCREEN *	/* screen */);

/*
 * This removes all the widgets from the screen.
 */
void eraseCDKScreen (
		CDKSCREEN *	/* screen */);

/*
 * Destroy all of the widgets on a screen
 */

void destroyCDKScreenObjects (CDKSCREEN *cdkscreen);

/*
 * This frees up any memory the CDKSCREEN pointer used.
 */
void destroyCDKScreen (
		CDKSCREEN *	/* screen */);

/*
 * This shuts down curses and everything else needed to
 * exit cleanly.
 */
void endCDK(void);

/*
 * This creates all the color pairs.
 */
void initCDKColor(void);

#ifdef __cplusplus
}
#endif

#endif /* CDKSCREEN_H */
#endif /* CDKINCLUDES */

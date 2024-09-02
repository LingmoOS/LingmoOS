/*
 * $Id: binding.h,v 1.15 2005/03/08 23:44:25 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKBINDING_H
#define CDKBINDING_H	1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CDK_H
#define CDKINCLUDES
#include <cdk.h>
#undef CDKINCLUDES
#endif

/*
 * Copyright 1999-2004,2005 Thomas E. Dickey
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
 * Create definitions for the key bindings.
 */

/*
 * This is the key binding prototype, typed for use with Perl.
 */
#define BINDFN_PROTO(func)  \
	int (func) ( \
		EObjectType	/* cdktype */, \
		void *		/* object */, \
		void *		/* clientData */, \
		chtype		/* input */)

typedef BINDFN_PROTO(*BINDFN);

/*
 * Bind to this function to simply translate keys without doing anything else,
 * in getcCDKObject().
 */
extern BINDFN_PROTO(getcCDKBind);

/*
 * This is the prototype for the process callback functions.
 */
typedef int (*PROCESSFN) (
		EObjectType	/* cdktype */,
		void *		/* object */,
		void *		/* clientData */,
		chtype 		/* input */);

/*
 * This binds the key to the event.
 */
void bindCDKObject (
		EObjectType	/* cdktype */,
		void *		/* object */,
		chtype		/* key */,
		BINDFN		/* function */,
		void *		/* data */);

/*
 * This unbinds the key from the event.
 */
void unbindCDKObject (
		EObjectType	/* cdktype */,
		void *		/* object */,
		chtype		/* key */);

/*
 * This checks if the given key has an event 'attached' to it, executes the
 * bound function if so.
 */
int checkCDKObjectBind (
		EObjectType	/* cdktype */,
		void *		/* object */,
		chtype		/* key */);

/*
 * This checks if the given key has an event 'attached' to it.
 */
bool isCDKObjectBind (
		EObjectType	/* cdktype */,
		void *		/* object */,
		chtype		/* key */);

/*
 * This cleans out all of the key bindings.
 */
void cleanCDKObjectBindings (
		EObjectType	/* cdktype */,
		void *		/* object */);

#ifdef __cplusplus
}
#endif

#endif /* CDKBINDING_H */
#endif /* CDKINCLUDES */

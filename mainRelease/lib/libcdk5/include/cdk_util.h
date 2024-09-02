/*
 * $Id: cdk_util.h,v 1.36 2014/01/19 01:58:31 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKUTIL_H
#define CDKUTIL_H	1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CDK_H
#define CDKINCLUDES
#include <cdk.h>
#undef CDKINCLUDES
#include <binding.h>
#include <cdkscreen.h>
#endif

/*
 * Changes 1999-2012,2014 copyright Thomas E. Dickey
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
 * This beeps at the user. The standard curses beep() does not
 * flush the stream, so it will only beep until a force is made.
 * This forces a flush after issuing the beep command.
 */
void Beep(void);

/*
 * This aligns a 'box' on the given window with the height and
 * width given.
 */
void alignxy (
		WINDOW *	/* window */,
		int *		/* xpos */,
		int *		/* ypos */,
		int		/* boxWidth */,
		int		/* boxHeight */);

/*
 * This takes a string, a field width and a justification type and returns the
 * justification adjustment to make, to fill the justification requirement.
 */
int justifyString (
		int		/* fieldWidth */,
		int		/* mesglen */,
		int		/* justify */);

/*
 * This is a quick little popup label widget.
 */
void popupLabel (
		CDKSCREEN *	/* win */,
		CDK_CSTRING2	/* mesg */,
		int		/* count */);

/*
 * This is a quick little popup label widget.
 */
void popupLabelAttrib (
		CDKSCREEN *	/* win */,
		CDK_CSTRING2	/* mesg */,
		int		/* count */,
		chtype		/* attribute */);

/*
 * This is a quick little popup dialog box.
 */
int popupDialog (
		CDKSCREEN *	/* screen */,
		CDK_CSTRING2	/* mesg */,
		int		/* mesgCount */,
		CDK_CSTRING2	/* buttons */,
		int		/* buttonCount */);

/*
 * This pops up a scrolling list and returns the value selected.
 */
int getListIndex (
		CDKSCREEN *	/* screen */,
		const char *	/* title */,
		CDK_CSTRING2	/* list */,
		int		/* listSize */,
		boolean		/* numbers */);

/*
 * This gets a simple string value from a user.
 */
char *getString (
		CDKSCREEN *	/* screen */,
		const char *	/* title */,
		const char *	/* label */,
		const char *	/* init */);

/*
 * This allows a user to view a file.
 */
int viewFile (
		CDKSCREEN *	/* screen */,
		const char *	/* title */,
		const char *	/* filename */,
		CDK_CSTRING2	/* buttons */,
		int		/* buttonCount */);

/*
 * This allows a person to select a file.
 */
char *selectFile (
		CDKSCREEN *	/* screen */,
		const char *	/* title */);

/*
 * This allows people to view information in an array.
 */
int viewInfo (
		CDKSCREEN *	/* screen */,
		const char *	/* title */,
		CDK_CSTRING2	/* info */,
		int		/* size */,
		CDK_CSTRING2	/* buttons */,
		int		/* buttonCount */,
		boolean		/* interpret */);

/*
 * This is a wrapper for qsort.
 */
void sortList (
		CDK_CSTRING *	/* list */,
		int		/* length */);

/*
 * This reads a file, loads the contents into info and
 * returns the number of lines read.
 */
int CDKreadFile (
		const char *	/* filename */,
		char ***	/* info */);

/*
 * This strips white space from the front/back of the given
 * string. The stripType is one of: vFRONT, vBACK, vBOTH.
 */
void stripWhiteSpace (
		EStripType	/* stripType */,
		char *		/* string */);

/*
 * These functions are used to manage a string which is split into parts, e.g.,
 * a file which is read into memory.
 */
char **CDKsplitString(
		const char *	/* string */,
		int		/* separator */);

unsigned CDKallocStrings(
		char ***	/* list */,
		char *		/* item */,
		unsigned	/* length */,
		unsigned	/* used */);

unsigned CDKcountStrings(
		CDK_CSTRING2	/* list */);

void CDKfreeStrings(
		char **		/* list */);

/*
 * Free a list of chtype-strings, terminated by a null pointer.
 */
void CDKfreeChtypes(
		chtype **	/* list */);

/*
 * This returns the length of an integer.
 */
int intlen (
		int		/* value */);

/*
 * This opens the given directory and reads in the contents. It stores
 * the results in 'list' and returns the number of elements found.
 */
int CDKgetDirectoryContents (
		const char *	/* directory */,
		char ***	/* list */);

/*
 * This looks for the given pattern in the given list.
 */
int searchList (
		CDK_CSTRING2	/* list */,
		int		/* listSize */,
		const char *	/* pattern */);

/*
 * This returns the basename of a file.
 */
char *baseName (
		char *		/* filename */);

/*
 * This returns the directory name of a file.
 */
char *dirName (
		char *		/* filename */);

/*
 * This frees the memory used by the given string.
 */
void freeChar (
		char *		/* string */);

/*
 * This frees the memory used by the given string.
 */
void freeChtype (
		chtype *	/* string */);

/*
 * This frees the memory used by the given list of strings.
 */
void freeCharList (
		char **		/* list */,
		unsigned	/* size */);

/*
 * This frees the memory used by the given list of strings.
 */
void freeChtypeList (
		chtype **	/* list */,
		unsigned	/* size */);

/*
 * This sets the elements of the given string to 'character'
 */
void cleanChar (
		char *		/* string */,
		int		/* length */,
		char		/* character */);

/*
 * This sets the elements of the given string to 'character'
 */
void cleanChtype (
		chtype *	/* string */,
		int		/* length */,
		chtype		/* character */);

/*
 * This takes a chtype pointer and returns a char pointer.
 */
char *chtype2Char (
		const chtype *	/* string */);

/*
 * This takes a chtype pointer and returns a char pointer with embedded
 * formatting information.
 */
char *chtype2String (
		const chtype *	/* string */);

/*
 * This takes a char pointer and returns a chtype pointer.
 */
chtype *char2Chtype (
		const char *	/* string */,
		int *		/* length */,
		int *		/* align */);

/*
 * Compare a char string to a chtype string
 */
int cmpStrChstr(const char *	/* str */,
	        const chtype *	/* chstr */);

/*
 * Copy from a chtype string to a char string
 */

void chstrncpy(char *		/* dest */,
		const chtype *	/* src */,
		int		/* maxcount */);

/*
 * Given a character pointer, returns the equivalent display type.
 */
EDisplayType char2DisplayType (
		const char *	/* string */);

/*
 * Tell if a display type is "hidden"
 */
boolean isHiddenDisplayType (
		EDisplayType	/* type */);

/*
 * Filter an input character according to the display type.
 */
int filterByDisplayType(
		EDisplayType	/* type */,
		chtype		/* input */);

/*
 * Copy the given string.
 */
chtype *copyChtype (
		const chtype *	/* string */);

/*
 * Copy the given string.
 */
char *copyChar (
		const char *	/* string */);

/*
 * Copy the given list.
 */
char **copyCharList (
		const char **	/* list */);

/*
 * Copy the given list.
 */
chtype **copyChtypeList (
		const chtype **	/* list */);

/*
 * Return the length of the given string.
 */
int chlen (
		const chtype *	/* string */);

/*
 * Return the length of the given list.
 */
int lenCharList (
		const char **	/* list */);

int lenChtypeList (
		const chtype **	/* list */);

/*
 * This takes a file mode and returns the first character of the file
 * permissions string.
 */
int mode2Filetype (
		mode_t		/* fileMode */);

/*
 * This takes a file mode and stores the character representation
 * of the mode in 'string'. This also returns the octal value
 * of the file mode.
 */
int mode2Char (
		char *		/* string */,
		mode_t		/* fileMode */);

/*
 * This looks for a link. (used by the </L> pattern)
 */
int checkForLink (
		const char *	/* line */,
		char *		/* filename */);

/*
 * This function help set the height/width values of a widget.
 */
int setWidgetDimension (
		int		/* parentDim */,
		int		/* proposedDim */,
		int		/* adjustment */);

/*
 * This safely erases a given window.
 */
void eraseCursesWindow (
		WINDOW *	/* window */);

/*
 * This safely deletes a given window.
 */
void deleteCursesWindow (
		WINDOW *	/* window */);

/*
 * This moves a given window
 */
void moveCursesWindow (
		WINDOW *	/* window */,
		int		/* xdiff */,
		int		/* ydiff */);

/*
 * Return an integer like 'floor()', which returns a double.
 */
int floorCDK(double);

/*
 * Return an integer like 'ceil()', which returns a double.
 */
int ceilCDK(double);

#ifdef __cplusplus
}
#endif

#endif /* CDKUTIL_H */
#endif /* CDKINCLUDES */

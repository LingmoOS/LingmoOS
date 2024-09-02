/*
 * $Id: template.h,v 1.25 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKTEMPLATE_H
#define CDKTEMPLATE_H	1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CDK_H
#define CDKINCLUDES
#include <cdk.h>
#undef CDKINCLUDES
#include <binding.h>
#include <cdkscreen.h>
#include <cdk_objs.h>
#endif

/*
 * Copyright 1999-2004,2012 Thomas E. Dickey
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
 * Define the CDK cdktemplate widget structure.
 */
typedef struct STemplate CDKTEMPLATE;
typedef void (*TEMPLATECB) (CDKTEMPLATE *cdktemplate, chtype input);

struct STemplate {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   WINDOW *	labelWin;
   WINDOW *	fieldWin;
   int		titleAdj;
   chtype *	label;
   chtype *	overlay;
   chtype	overlayAttr;
   char *	plate;
   char *	info;
   int		labelLen;
   int		overlayLen;
   int		labelPos;
   int		fieldWidth;
   int		boxWidth;
   int		boxHeight;
   int		platePos;
   int		plateLen;
   int		screenPos;
   int		infoPos;
   int		min;
   chtype	labelAttr;
   chtype	fieldAttr;
   EExitType	exitType;
   boolean	shadow;
   TEMPLATECB	callbackfn;
};

/*
 * This creates a pointer to a new CDK cdktemplate widget.
 */
CDKTEMPLATE *newCDKTemplate (
		CDKSCREEN *	/* cdkscreen */,
		int 		/* xpos */,
		int 		/* ypos */,
		const char *	/* title */,
		const char *	/* label */,
		const char *	/* plate */,
		const char *	/* overlay */,
		boolean 	/* Box */,
		boolean 	/* shadow */);

/*
 * This activates the cdktemplate widget.
 */
char *activateCDKTemplate (
		CDKTEMPLATE *	/* cdktemplate */,
		chtype *	/* actions */);

/*
 * This injects a single character into the widget.
 */
#define injectCDKTemplate(obj,input) injectCDKObject(obj,input,String)

/*
 * This sets various attributes of the widget.
 */
void setCDKTemplate (
		CDKTEMPLATE *	/* cdktemplate */,
		const char *	/* value */,
		boolean 	/* Box */);

/*
 * This sets the value in the cdktemplate widget.
 */
void setCDKTemplateValue (
		CDKTEMPLATE *	/* cdktemplate */,
		const char *	/* value */);

char *getCDKTemplateValue (
		CDKTEMPLATE *	/* cdktemplate */);

/*
 * This sets the minimum number of characters to enter.
 */
void setCDKTemplateMin (
		CDKTEMPLATE *	/* cdktemplate */,
		int 		/* min */);

int getCDKTemplateMin (
		CDKTEMPLATE *	/* cdktemplate */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKTemplateBox (
		CDKTEMPLATE *	/* cdktemplate */,
		boolean 	/* Box */);

boolean getCDKTemplateBox (
		CDKTEMPLATE *	/* cdktemplate */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKTemplateULChar(w,c)          setULCharOf(w,c)
#define setCDKTemplateURChar(w,c)          setURCharOf(w,c)
#define setCDKTemplateLLChar(w,c)          setLLCharOf(w,c)
#define setCDKTemplateLRChar(w,c)          setLRCharOf(w,c)
#define setCDKTemplateVerticalChar(w,c)    setVTCharOf(w,c)
#define setCDKTemplateHorizontalChar(w,c)  setHZCharOf(w,c)
#define setCDKTemplateBoxAttribute(w,c)    setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKTemplateBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKTemplateBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the cdktemplate on the screen.
 */
#define drawCDKTemplate(obj,Box) drawCDKObject(obj,Box)

/*
 * This erases the widget from the screen.
 */
#define eraseCDKTemplate(obj) eraseCDKObject(obj)

/*
 * This erases the cdktemplates contents.
 */
void cleanCDKTemplate (
		CDKTEMPLATE *	/* cdktemplate */);

/*
 * This moves the widget to the given location on the screen.
 */
#define moveCDKTemplate(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This interactively positions the widget on the screen.
 */
#define positionCDKTemplate(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and all associated memory.
 */
#define destroyCDKTemplate(obj) destroyCDKObject(obj)

/*
 * This sets the main callback function.
 */
void setCDKTemplateCB (
		CDKTEMPLATE *	/* cdktemplate */,
		TEMPLATECB 	/* callback */);

/*
 * This returns a character pointer to the contents of the cdktemplate
 * mixed with the plate.
 */
char *mixCDKTemplate (
		CDKTEMPLATE *	/* cdktemplate */);

/*
 * This returns a character pointer to the cdktemplate with the plate
 * stripped out.
 */
char *unmixCDKTemplate (
		CDKTEMPLATE *	/* cdktemplate */,
		const char *	/* string */);

/*
 * These set the pre/post callback functions.
 */
#define setCDKTemplatePreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKTemplatePostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKTEMPLATE_H */
#endif /* CDKINCLUDES */

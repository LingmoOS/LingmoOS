/*
 * $Id: calendar.h,v 1.32 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKCALENDAR_H
#define CDKCALENDAR_H	1

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
 * Copyright 2000-2011,2012 Thomas E. Dickey
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
 * Declare some definitions needed for this widget.
 */
#define MAX_DAYS	32	/* maximum number of days in any month */
#define MAX_MONTHS	13	/* month[0] is unused */
#define MAX_YEARS	140	/* years [1901..2140] */

#define CALENDAR_LIMIT          (MAX_DAYS * MAX_MONTHS * MAX_YEARS)

#define CALENDAR_INDEX(d,m,y)   ((((y) * MAX_MONTHS) + (m)) * MAX_DAYS + (d))
#define CALENDAR_CELL(c,d,m,y)  ((c)->marker[CALENDAR_INDEX(d,m,y)])

/*
 * Define the CDK calendar widget structure.
 */
struct SCalendar {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	labelWin;
   WINDOW *	fieldWin;
   WINDOW *	shadowWin;
   int		titleAdj;
   int		xpos;
   int		ypos;
   int		height;
   int		width;
   int		fieldWidth;
   int		labelLen;
   chtype	yearAttrib;
   chtype	monthAttrib;
   chtype	dayAttrib;
   chtype	highlight;
   chtype *	marker;
   int		day;
   int		month;
   int		year;
   int		weekDay;
   int		boxWidth;
   int		boxHeight;
   int		xOffset;
   EExitType	exitType;
   boolean	shadow;
   char *	DayName;
   char *	MonthName[MAX_MONTHS];
   int		weekBase;	/* starting day of week (Sunday=0, Monday=1) */
};
typedef struct SCalendar CDKCALENDAR;

/*
 * This creates a pointer to a new calendar widget.
 */
CDKCALENDAR *newCDKCalendar (
		CDKSCREEN *	/* screen */,
		int		/* xPos */,
		int		/* yPos */,
		const char *	/* title */,
		int		/* day */,
		int		/* month */,
		int		/* year */,
		chtype		/* dayAttrib */,
		chtype		/* monthAttrib */,
		chtype		/* yearAttrib */,
		chtype		/* highlight */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the calendar widget.
 */
time_t activateCDKCalendar (
		CDKCALENDAR *	/* calendar */,
		chtype *	/* actions */);

/*
 * This injects a single character into the widget.
 */
#define injectCDKCalendar(obj,input) injectCDKObject(obj,input,Int)

/*
 * This sets multiple attributes of the widget.
 */
void setCDKCalendar (
		CDKCALENDAR *	/* calendar */,
		int		/* day */,
		int		/* month */,
		int		/* year */,
		chtype		/* dayAttrib */,
		chtype		/* monthAttrib */,
		chtype		/* yearAttrib */,
		chtype		/* highlight */,
		boolean		/* Box */);

/*
 * This sets the date of the calendar.
 */
void setCDKCalendarDate (
		CDKCALENDAR *	/* calendar */,
		int		/* day */,
		int		/* month */,
		int		/* year */);

void getCDKCalendarDate (
		CDKCALENDAR *	/* calendar */,
		int *		/* day */,
		int *		/* month */,
		int *		/* year */);

/*
 * This sets the attribute of the days in the calendar.
 */
void setCDKCalendarDayAttribute (
		CDKCALENDAR *	/* calendar */,
		chtype		/* attribute */);

chtype getCDKCalendarDayAttribute (
		CDKCALENDAR *	/* calendar */);

/*
 * This sets the attribute of the month names in the calendar.
 */
void setCDKCalendarMonthAttribute (
		CDKCALENDAR *	/* calendar */,
		chtype		/* attribute */);

chtype getCDKCalendarMonthAttribute (
		CDKCALENDAR *	/* calendar */);

/*
 * This sets the attribute of the year in the calendar.
 */
void setCDKCalendarYearAttribute (
		CDKCALENDAR *	/* calendar */,
		chtype		/* attribute */);

chtype getCDKCalendarYearAttribute (
		CDKCALENDAR *	/* calendar */);

/*
 * This sets the attribute of the highlight bar.
 */
void setCDKCalendarHighlight (
		CDKCALENDAR *	/* calendar */,
		chtype		/* highlight */);

chtype getCDKCalendarHighlight (
		CDKCALENDAR *	/* calendar */);

/*
  * This sets the box attribute of the widget.
 */
void setCDKCalendarBox (
		CDKCALENDAR *	/* calendar */,
		boolean		/* Box */);

boolean getCDKCalendarBox (
		CDKCALENDAR *	/* calendar */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKCalendarULChar(w,c)          setULCharOf(w,c)
#define setCDKCalendarURChar(w,c)          setURCharOf(w,c)
#define setCDKCalendarLLChar(w,c)          setLLCharOf(w,c)
#define setCDKCalendarLRChar(w,c)          setLRCharOf(w,c)
#define setCDKCalendarVerticalChar(w,c)    setVTCharOf(w,c)
#define setCDKCalendarHorizontalChar(w,c)  setHZCharOf(w,c)
#define setCDKCalendarBoxAttribute(w,c)    setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKCalendarBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKCalendarBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This sets a marker on the calendar.
 */
void setCDKCalendarMarker (
		CDKCALENDAR *	/* calendar */,
		int		/* day */,
		int		/* month */,
		int		/* year */,
		chtype		/* markerChar */);
/*
 * Return the marker set on the calendar.
 */
chtype getCDKCalendarMarker (
		CDKCALENDAR *	/* calendar */,
		int		/* day */,
		int		/* month */,
		int		/* year */);

/*
 * This removes a marker from the calendar.
 */
void removeCDKCalendarMarker (
		CDKCALENDAR *	/* calendar */,
		int		/* day */,
		int		/* month */,
		int		/* year */);

/*
 * This draws the widget on the screen.
 */
#define drawCDKCalendar(obj,box) drawCDKObject(obj,box)

/*
 * This removes the widget from the screen.
 */
#define eraseCDKCalendar(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given location.
 */
#define moveCDKCalendar(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This is an interactive method of moving the widget.
 */
#define positionCDKCalendar(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the calendar widget and all associated memory.
 */
#define destroyCDKCalendar(obj) destroyCDKObject(obj)

/*
 * This sets the pre and post process functions.
 */
#define setCDKCalendarPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKCalendarPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

/*
 * This sets days and months names
 */
void setCDKCalendarMonthsNames (
		CDKCALENDAR *	/* calendar */,
		CDK_CSTRING2	/* months */);

void setCDKCalendarDaysNames (
		CDKCALENDAR *	/* calendar */,
		const char *	/* days - 1st is Sunday */);

#ifdef __cplusplus
}
#endif

#endif /* CDKCALENDAR_H */
#endif /* CDKINCLUDES */

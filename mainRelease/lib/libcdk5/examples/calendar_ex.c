/* $Id: calendar_ex.c,v 1.17 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "calendar_ex";
#endif

static BINDFN_PROTO (createCalendarMarkCB);
static BINDFN_PROTO (removeCalendarMarkCB);

/*
 * This program demonstrates the Cdk calendar widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen		= 0;
   CDKCALENDAR *calendar	= 0;
   const char *mesg[5];
   char temp[256];
   struct tm *dateInfo;
   time_t clck, retVal;

   CDK_PARAMS params;
   char *title;
   int day;
   int month;
   int year;

   /*
    * Get the current dates and set the default values for
    * the day/month/year values for the calendar.
    */
   time (&clck);
   dateInfo	= gmtime (&clck);

   /* *INDENT-EQLS* */
   CDKparseParams (argc, argv, &params, "d:m:y:t:w:" CDK_MIN_PARAMS);
   day   = CDKparamNumber2 (&params, 'd', dateInfo->tm_mday);
   month = CDKparamNumber2 (&params, 'm', dateInfo->tm_mon + 1);
   year  = CDKparamNumber2 (&params, 'y', dateInfo->tm_year + 1900);
   title = CDKparamString2 (&params, 't', "<C></U>CDK Calendar Widget\n<C>Demo");

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Create the calendar widget. */
   calendar = newCDKCalendar (cdkscreen,
			      CDKparamValue (&params, 'X', CENTER),
			      CDKparamValue (&params, 'Y', CENTER),
			      title, day, month, year,
			      COLOR_PAIR (16) | A_BOLD,
			      COLOR_PAIR (24) | A_BOLD,
			      COLOR_PAIR (32) | A_BOLD,
			      COLOR_PAIR (40) | A_REVERSE,
			      CDKparamValue (&params, 'N', TRUE),
			      CDKparamValue (&params, 'S', FALSE));

   /* Is the widget null? */
   if (calendar == 0)
   {
      /* Clean up the memory. */
      destroyCDKScreen (cdkscreen);

      /* End curses... */
      endCDK ();

      printf ("Cannot create the calendar. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Create a key binding to mark days on the calendar. */
   bindCDKObject (vCALENDAR, calendar, 'm', createCalendarMarkCB, calendar);
   bindCDKObject (vCALENDAR, calendar, 'M', createCalendarMarkCB, calendar);
   bindCDKObject (vCALENDAR, calendar, 'r', removeCalendarMarkCB, calendar);
   bindCDKObject (vCALENDAR, calendar, 'R', removeCalendarMarkCB, calendar);

   calendar->weekBase = CDKparamNumber (&params, 'w');

   /* Draw the calendar widget. */
   drawCDKCalendar (calendar, ObjOf (calendar)->box);

   /* Let the user play with the widget. */
   retVal = activateCDKCalendar (calendar, 0);

   /* Check which day they selected. */
   if (calendar->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No date selected.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }
   else if (calendar->exitType == vNORMAL)
   {
      mesg[0] = "You selected the following date";
      sprintf (temp, "<C></B/16>%02d/%02d/%d (dd/mm/yyyy)",
	       calendar->day,
	       calendar->month,
	       calendar->year);
      mesg[1] = temp;
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }

   /* Clean up and exit. */
   destroyCDKCalendar (calendar);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   fflush (stdout);
   printf ("Selected Time: %s\n", ctime (&retVal));
   ExitProgram (EXIT_SUCCESS);
}

/*
 * This adds a marker to the calendar.
 */
static int createCalendarMarkCB (EObjectType objectType GCC_UNUSED,
				 void *object,
				 void *clientData GCC_UNUSED,
				 chtype key GCC_UNUSED)
{
   CDKCALENDAR *calendar = (CDKCALENDAR *)object;

   setCDKCalendarMarker (calendar,
			 calendar->day,
			 calendar->month,
			 calendar->year,
			 COLOR_PAIR (5) | A_REVERSE);

   drawCDKCalendar (calendar, ObjOf (calendar)->box);
   return (FALSE);
}

/*
 * This removes a marker from the calendar.
 */
static int removeCalendarMarkCB (EObjectType objectType GCC_UNUSED,
				 void *object,
				 void *clientData GCC_UNUSED,
				 chtype key GCC_UNUSED)
{
   CDKCALENDAR *calendar = (CDKCALENDAR *)object;

   removeCDKCalendarMarker (calendar,
			    calendar->day,
			    calendar->month,
			    calendar->year);

   drawCDKCalendar (calendar, ObjOf (calendar)->box);
   return (FALSE);
}

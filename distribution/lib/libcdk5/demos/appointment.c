/* $Id: appointment.c,v 1.30 2021/01/09 22:42:07 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "appointmentBook";
#endif

/*
 * Create definitions.
 */
#define MAX_MARKERS	2000

/*
 *
 */
static chtype GPAppointmentAttributes[] =
{
   A_BLINK,
   A_BOLD,
   A_REVERSE,
   A_UNDERLINE
};

/*
 *
 */
typedef enum
{
   vBirthday, vAnniversary, vAppointment, vOther
}
EAppointmentType;

/*
 *
 */
struct AppointmentMarker
{
   EAppointmentType type;
   char *description;
   int day;
   int month;
   int year;
};

/*
 *
 */
struct AppointmentInfo
{
   struct AppointmentMarker appointment[MAX_MARKERS];
   int appointmentCount;
};

/*
 * Declare local function prototypes.
 */
static BINDFN_PROTO (createCalendarMarkCB);
static BINDFN_PROTO (removeCalendarMarkCB);
static BINDFN_PROTO (displayCalendarMarkCB);
static BINDFN_PROTO (accelerateToDateCB);
void readAppointmentFile (char *filename, struct AppointmentInfo *appInfo);
void saveAppointmentFile (char *filename, struct AppointmentInfo *appInfo);

/*
 * This program demonstrates the Cdk calendar widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen         = 0;
   CDKCALENDAR *calendar        = 0;
   const char *title            = "<C></U>CDK Appointment Book\n<C><#HL(30)>\n";
   char *filename               = 0;
   struct tm *dateInfo          = 0;
   time_t clck                  = 0;
   struct AppointmentInfo appointmentInfo;
   int day, month, year, x;

   /*
    * Get the current dates and set the default values for
    * the day/month/year values for the calendar.
    */
   /* *INDENT-EQLS* */
   time (&clck);
   dateInfo    = gmtime (&clck);
   day         = dateInfo->tm_mday;
   month       = dateInfo->tm_mon + 1;
   year        = dateInfo->tm_year + 1900;

   /* Check the command line for options. */
   while (1)
   {
      int ret;

      /* Are there any more command line options to parse. */
      if ((ret = getopt (argc, argv, "d:m:y:t:f:")) == -1)
      {
	 break;
      }

      switch (ret)
      {
      case 'd':
	 day = atoi (optarg);
	 break;

      case 'm':
	 month = atoi (optarg);
	 break;

      case 'y':
	 year = atoi (optarg);
	 break;

      case 't':
	 title = copyChar (optarg);
	 break;

      case 'f':
	 filename = copyChar (optarg);
	 break;
      }
   }

   /* Create the appointment book filename. */
   if (filename == 0)
   {
      char temp[1000];
      char *home = getenv ("HOME");
      if (home != 0)
      {
	 sprintf (temp, "%.*s/.appointment", (int)sizeof (temp) - 20, home);
      }
      else
      {
	 strcpy (temp, ".appointment");
      }
      filename = copyChar (temp);
   }

   /* Read the appointment book information. */
   readAppointmentFile (filename, &appointmentInfo);

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Create the calendar widget. */
   calendar = newCDKCalendar (cdkscreen, CENTER, CENTER,
			      title, day, month, year,
			      A_NORMAL, A_NORMAL,
			      A_NORMAL, A_REVERSE,
			      TRUE, FALSE);

   /* Is the widget null? */
   if (calendar == 0)
   {
      /* Clean up the memory. */
      destroyCDKScreen (cdkscreen);

      /* End curses... */
      endCDK ();

      /* Spit out a message. */
      printf ("Cannot create the calendar. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Create a key binding to mark days on the calendar. */
   bindCDKObject (vCALENDAR, calendar, 'm', createCalendarMarkCB, &appointmentInfo);
   bindCDKObject (vCALENDAR, calendar, 'M', createCalendarMarkCB, &appointmentInfo);
   bindCDKObject (vCALENDAR, calendar, 'r', removeCalendarMarkCB, &appointmentInfo);
   bindCDKObject (vCALENDAR, calendar, 'R', removeCalendarMarkCB, &appointmentInfo);
   bindCDKObject (vCALENDAR, calendar, '?', displayCalendarMarkCB, &appointmentInfo);
   bindCDKObject (vCALENDAR, calendar, 'j', accelerateToDateCB, &appointmentInfo);
   bindCDKObject (vCALENDAR, calendar, 'J', accelerateToDateCB, &appointmentInfo);

   /* Set all the appointments read from the file. */
   for (x = 0; x < appointmentInfo.appointmentCount; x++)
   {
      chtype marker = GPAppointmentAttributes[appointmentInfo.appointment[x].type];

      setCDKCalendarMarker (calendar,
			    appointmentInfo.appointment[x].day,
			    appointmentInfo.appointment[x].month,
			    appointmentInfo.appointment[x].year,
			    marker);
   }

   /* Draw the calendar widget. */
   drawCDKCalendar (calendar, ObjOf (calendar)->box);

   /* Let the user play with the widget. */
   activateCDKCalendar (calendar, 0);

   /* Save the appointment information. */
   saveAppointmentFile (filename, &appointmentInfo);

   free (filename);

   /* Clean up and exit. */
   destroyCDKCalendar (calendar);
   destroyCDKScreen (cdkscreen);
   endCDK ();

   ExitProgram (EXIT_SUCCESS);
}

/*
 * This reads a given appointment file.
 */
void readAppointmentFile (char *filename, struct AppointmentInfo *appInfo)
{
   /* *INDENT-EQLS* */
   int appointments     = 0;
   int linesRead        = 0;
   char **lines         = 0;
   int x;

   /* Read the appointment file. */
   linesRead = CDKreadFile (filename, &lines);
   if (linesRead == -1)
   {
      appInfo->appointmentCount = 0;
      return;
   }

   /* Split each line up and create an appointment. */
   for (x = 0; x < linesRead; x++)
   {
      char **temp = CDKsplitString (lines[x], CTRL ('V'));
      int segments = (int)CDKcountStrings ((CDK_CSTRING2)temp);

      /*
       * A valid line has 5 elements:
       *          Day, Month, Year, Type, Description.
       */
      if (segments == 5)
      {
	 int eType = atoi (temp[3]);

	 /* *INDENT-EQLS* */
	 appInfo->appointment[appointments].day       = atoi (temp[0]);
	 appInfo->appointment[appointments].month     = atoi (temp[1]);
	 appInfo->appointment[appointments].year      = atoi (temp[2]);
	 appInfo->appointment[appointments].type      = (EAppointmentType) eType;
	 appInfo->appointment[appointments].description = copyChar (temp[4]);
	 appointments++;
      }
      CDKfreeStrings (temp);
   }
   CDKfreeStrings (lines);

   /* Keep the amount of appointments read. */
   appInfo->appointmentCount = appointments;
}

/*
 * This saves a given appointment file.
 */
void saveAppointmentFile (char *filename, struct AppointmentInfo *appInfo)
{
   /* Declare local variables. */
   FILE *fd;
   int x;

   /* Can we open the file? */
   if ((fd = fopen (filename, "w")) == 0)
   {
      return;
   }

   /* Start writing. */
   for (x = 0; x < appInfo->appointmentCount; x++)
   {
      if (appInfo->appointment[x].description != 0)
      {
	 fprintf (fd, "%d%c%d%c%d%c%d%c%s\n",
		  appInfo->appointment[x].day, CTRL ('V'),
		  appInfo->appointment[x].month, CTRL ('V'),
		  appInfo->appointment[x].year, CTRL ('V'),
		  (int)appInfo->appointment[x].type, CTRL ('V'),
		  appInfo->appointment[x].description);

	 freeChar (appInfo->appointment[x].description);
      }
   }
   fclose (fd);
}

/*
 * This adds a marker to the calendar.
 */
static int createCalendarMarkCB (EObjectType objectType GCC_UNUSED, void *object,
				 void *clientData,
				 chtype key GCC_UNUSED)
{
   /* *INDENT-EQLS* */
   CDKCALENDAR *calendar                        = (CDKCALENDAR *)object;
   CDKENTRY *entry                              = 0;
   CDKITEMLIST *itemlist                        = 0;
   const char *items[]                          =
   {
      "Birthday",
      "Anniversary",
      "Appointment",
      "Other"
   };
   char *description                            = 0;
   struct AppointmentInfo *appointmentInfo      = (struct AppointmentInfo *)clientData;
   int current                                  = appointmentInfo->appointmentCount;
   chtype marker;
   int selection;

   /* Create the itemlist widget. */
   itemlist = newCDKItemlist (ScreenOf (calendar),
			      CENTER, CENTER, 0,
			      "Select Appointment Type: ",
			      (CDK_CSTRING2)items, 4, 0,
			      TRUE, FALSE);

   /* Get the appointment type from the user. */
   selection = activateCDKItemlist (itemlist, 0);

   /* They hit escape, kill the itemlist widget and leave. */
   if (selection == -1)
   {
      destroyCDKItemlist (itemlist);
      drawCDKCalendar (calendar, ObjOf (calendar)->box);
      return (FALSE);
   }

   /* Destroy the itemlist and set the marker. */
   destroyCDKItemlist (itemlist);
   drawCDKCalendar (calendar, ObjOf (calendar)->box);
   marker = GPAppointmentAttributes[selection];

   /* Create the entry field for the description. */
   entry = newCDKEntry (ScreenOf (calendar),
			CENTER, CENTER,
			"<C>Enter a description of the appointment.",
			"Description: ",
			A_NORMAL, (chtype)'.',
			vMIXED, 40, 1, 512,
			TRUE, FALSE);

   /* Get the description. */
   description = activateCDKEntry (entry, 0);
   if (description == 0)
   {
      destroyCDKEntry (entry);
      drawCDKCalendar (calendar, ObjOf (calendar)->box);
      return (FALSE);
   }

   /* Destroy the entry and set the marker. */
   description = copyChar (entry->info);
   destroyCDKEntry (entry);
   drawCDKCalendar (calendar, ObjOf (calendar)->box);

   /* Set the marker. */
   setCDKCalendarMarker (calendar,
			 calendar->day,
			 calendar->month,
			 calendar->year,
			 marker);

   /* Keep the marker. */
   appointmentInfo->appointment[current].day = calendar->day;
   appointmentInfo->appointment[current].month = calendar->month;
   appointmentInfo->appointment[current].year = calendar->year;
   appointmentInfo->appointment[current].type = (EAppointmentType) selection;
   appointmentInfo->appointment[current].description = description;
   appointmentInfo->appointmentCount++;

   /* Redraw the calendar. */
   drawCDKCalendar (calendar, ObjOf (calendar)->box);
   return (FALSE);
}

/*
 * This removes a marker from the calendar.
 */
static int removeCalendarMarkCB (EObjectType objectType GCC_UNUSED, void
				 *object, void *clientData, chtype key GCC_UNUSED)
{
   CDKCALENDAR *calendar = (CDKCALENDAR *)object;
   struct AppointmentInfo *appointmentInfo = (struct AppointmentInfo *)clientData;
   int x;

   /* Look for the marker in the list. */
   for (x = 0; x < appointmentInfo->appointmentCount; x++)
   {
      if ((appointmentInfo->appointment[x].day == calendar->day) &&
	  (appointmentInfo->appointment[x].month == calendar->month) &&
	  (appointmentInfo->appointment[x].year == calendar->year))
      {
	 freeChar (appointmentInfo->appointment[x].description);
	 appointmentInfo->appointment[x].description = 0;
	 break;
      }
   }

   /* Remove the marker from the calendar. */
   removeCDKCalendarMarker (calendar,
			    calendar->day,
			    calendar->month,
			    calendar->year);

   /* Redraw the calendar. */
   drawCDKCalendar (calendar, ObjOf (calendar)->box);
   return (FALSE);
}

/*
 * This displays the marker(s) on the given day.
 */
static int displayCalendarMarkCB (EObjectType objectType GCC_UNUSED, void
				  *object, void *clientData, chtype key GCC_UNUSED)
{
   /* *INDENT-EQLS* */
   CDKCALENDAR *calendar                   = (CDKCALENDAR *)object;
   CDKLABEL *label                         = 0;
   struct AppointmentInfo *appointmentInfo = (struct AppointmentInfo *)clientData;
   int found                               = 0;
   int mesgLines                           = 0;
   const char *type                        = 0;
   char *mesg[10], temp[256];
   int x;

   /* Look for the marker in the list. */
   for (x = 0; x < appointmentInfo->appointmentCount; x++)
   {
      /* Get the day month year. */
      /* *INDENT-EQLS* */
      int day   = appointmentInfo->appointment[x].day;
      int month = appointmentInfo->appointment[x].month;
      int year  = appointmentInfo->appointment[x].year;

      /* Determine the appointment type. */
      if (appointmentInfo->appointment[x].type == vBirthday)
      {
	 type = "Birthday";
      }
      else if (appointmentInfo->appointment[x].type == vAnniversary)
      {
	 type = "Anniversary";
      }
      else if (appointmentInfo->appointment[x].type == vAppointment)
      {
	 type = "Appointment";
      }
      else
      {
	 type = "Other";
      }

      /* Find the marker by the day/month/year. */
      if ((day == calendar->day) &&
	  (month == calendar->month) &&
	  (year == calendar->year) &&
	  (appointmentInfo->appointment[x].description != 0))
      {
	 /* Create the message for the label widget. */
	 sprintf (temp, "<C>Appointment Date: %02d/%02d/%d", day, month, year);
	 mesg[mesgLines++] = copyChar (temp);
	 mesg[mesgLines++] = copyChar (" ");
	 mesg[mesgLines++] = copyChar ("<C><#HL(35)>");

	 sprintf (temp, " Appointment Type: %s", type);
	 mesg[mesgLines++] = copyChar (temp);

	 mesg[mesgLines++] = copyChar (" Description     :");
	 sprintf (temp, "    %s", appointmentInfo->appointment[x].description);
	 mesg[mesgLines++] = copyChar (temp);

	 mesg[mesgLines++] = copyChar ("<C><#HL(35)>");
	 mesg[mesgLines++] = copyChar (" ");
	 mesg[mesgLines++] = copyChar ("<C>Press space to continue.");

	 found = 1;
	 break;
      }
   }

   /* If we didn't find the marker, create a different message. */
   if (found == 0)
   {
      sprintf (temp, "<C>There is no appointment for %02d/%02d/%d",
	       calendar->day, calendar->month, calendar->year);
      mesg[mesgLines++] = copyChar (temp);
      mesg[mesgLines++] = copyChar ("<C><#HL(30)>");
      mesg[mesgLines++] = copyChar ("<C>Press space to continue.");
   }

   /* Create the label widget. */
   label = newCDKLabel (ScreenOf (calendar), CENTER, CENTER,
			(CDK_CSTRING2)mesg, mesgLines, TRUE, FALSE);
   drawCDKLabel (label, ObjOf (label)->box);
   waitCDKLabel (label, ' ');
   destroyCDKLabel (label);

   /* Clean up the memory used. */
   for (x = 0; x < mesgLines; x++)
   {
      freeChar (mesg[x]);
   }

   /* Redraw the calendar widget. */
   drawCDKCalendar (calendar, ObjOf (calendar)->box);
   return (FALSE);
}

/*
 * This allows the user to accelerate to a given date.
 */
static int accelerateToDateCB (EObjectType objectType GCC_UNUSED, void
			       *object GCC_UNUSED, void *clientData
			       GCC_UNUSED, chtype key GCC_UNUSED)
{
   return (FALSE);
}

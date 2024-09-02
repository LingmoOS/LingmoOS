/* $Id: itemlist_ex.c,v 1.15 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "itemlist_ex";
#endif

#define MONTHS 12

/*
 * This program demonstrates the Cdk itemlist widget.
 *
 * Options (in addition to minimal CLI parameters):
 *	-c	create the data after the widget
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen         = 0;
   CDKITEMLIST *monthlist       = 0;
   const char *title            = "<C>Pick A Month";
   const char *label            = "</U/5>Month:";
   const char *info[MONTHS];
   const char *mesg[10];
   char temp[256];
   int choice, startMonth;
   struct tm *dateInfo;
   time_t clck;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, "c" CDK_MIN_PARAMS);

   /*
    * Get the current date and set the default month to the
    * current month.
    */
   time (&clck);
   dateInfo = localtime (&clck);
   startMonth = dateInfo->tm_mon;

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK colors. */
   initCDKColor ();

   /* Create the choice list. */
   /* *INDENT-EQLS* */
   info[0]      = "<C></5>January";
   info[1]      = "<C></5>February";
   info[2]      = "<C></B/19>March";
   info[3]      = "<C></5>April";
   info[4]      = "<C></5>May";
   info[5]      = "<C></K/5>June";
   info[6]      = "<C></12>July";
   info[7]      = "<C></5>August";
   info[8]      = "<C></5>September";
   info[9]      = "<C></32>October";
   info[10]     = "<C></5>November";
   info[11]     = "<C></11>December";

   /* Create the itemlist widget. */
   monthlist = newCDKItemlist (cdkscreen,
			       CDKparamValue (&params, 'X', CENTER),
			       CDKparamValue (&params, 'Y', CENTER),
			       title,
			       label,
			       (CDKparamNumber (&params, 'c')
				? 0
				: (CDK_CSTRING2) info),
			       (CDKparamNumber (&params, 'c')
				? 0
				: MONTHS),
			       startMonth,
			       CDKparamValue (&params, 'N', TRUE),
			       CDKparamValue (&params, 'S', FALSE));

   /* Is the widget null? */
   if (monthlist == 0)
   {
      /* Clean up. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create the itemlist box. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   if (CDKparamNumber (&params, 'c'))
   {
      setCDKItemlistValues (monthlist, (CDK_CSTRING2) info, MONTHS, 0);
   }

   /* Activate the widget. */
   choice = activateCDKItemlist (monthlist, 0);

   /* Check how they exited from the widget. */
   if (monthlist->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No item selected.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (ScreenOf (monthlist), (CDK_CSTRING2) mesg, 3);
   }
   else if (monthlist->exitType == vNORMAL)
   {
      sprintf (temp, "<C>You selected the %dth item which is", choice);
      mesg[0] = temp;
      mesg[1] = info[choice];
      mesg[2] = "";
      mesg[3] = "<C>Press any key to continue.";
      popupLabel (ScreenOf (monthlist), (CDK_CSTRING2) mesg, 4);
   }

   /* Clean up. */
   destroyCDKItemlist (monthlist);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

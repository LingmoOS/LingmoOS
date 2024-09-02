/* $Id: subwindow_ex.c,v 1.13 2016/12/04 16:20:33 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "subwindow_ex";
#endif

/*
 * This demo displays the ability to put widgets within a curses subwindow.
 */
int main (int argc, char **argv)
{
   CDKSCREEN *cdkscreen;
   CDKSCROLL *dowList;
   CDKLABEL *title;
   WINDOW *subWindow;
   const char *dow[] =
   {
      "Monday",
      "Tuesday",
      "Wednesday",
      "Thursday",
      "Friday",
      "Saturday",
      "Sunday"
   };
   const char *mesg[5];
   int pick;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, "s:" CDK_CLI_PARAMS);

   /* Start curses. */
   (void) initCDKScreen (NULL);
   curs_set (0);

   /* Create a basic window. */
   subWindow = newwin (LINES - 5, COLS - 10, 2, 5);

   /* Start Cdk. */
   cdkscreen = initCDKScreen (subWindow);

   /* Box our window. */
   box (subWindow, ACS_VLINE, ACS_HLINE);
   wrefresh (subWindow);

   /* Create a basic scrolling list inside the window. */
   dowList = newCDKScroll (cdkscreen,
			   CDKparamValue (&params, 'X', CENTER),
			   CDKparamValue (&params, 'Y', CENTER),
			   CDKparsePosition (CDKparamString2 (&params,
							      's',
							      "RIGHT")),
			   CDKparamValue (&params, 'H', 10),
			   CDKparamValue (&params, 'W', 15),
			   "<C></U>Pick a Day",
			   (CDK_CSTRING2) dow, 7,
			   NONUMBERS,
			   A_REVERSE,
			   CDKparamValue (&params, 'N', TRUE),
			   CDKparamValue (&params, 'S', FALSE));

   /* Put a title within the window. */
   mesg[0] = "<C><#HL(30)>";
   mesg[1] = "<C>This is a Cdk scrolling list";
   mesg[2] = "<C>inside a curses window.";
   mesg[3] = "<C><#HL(30)>";
   title = newCDKLabel (cdkscreen, CENTER, 0,
			(CDK_CSTRING2) mesg, 4,
			FALSE, FALSE);

   /* Refresh the screen. */
   refreshCDKScreen (cdkscreen);

   /* Let the user play. */
   pick = activateCDKScroll (dowList, 0);

   /* Clean up. */
   destroyCDKScroll (dowList);
   destroyCDKLabel (title);
   eraseCursesWindow (subWindow);
   destroyCDKScreen (cdkscreen);
   endCDK ();

   /* Tell them what they picked. */
   printf ("You picked %s\n", dow[pick]);
   ExitProgram (EXIT_SUCCESS);
}

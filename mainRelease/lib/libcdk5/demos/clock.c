/* $Id: clock.c,v 1.11 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "label_ex";
#endif

int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKLABEL *demo       = 0;
   int boxLabel         = 0;
   const char *mesg[4];
   char temp[256];
   time_t clck;
   int ret;

   /* Parse up the command line. */
   while ((ret = getopt (argc, argv, "b")) != -1)
   {
      switch (ret)
      {
      case 'b':
	 boxLabel = 1;
      }
   }

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Colors */
   initCDKColor ();

   /* Set the labels up. */
   mesg[0] = "</1/B>HH:MM:SS";

   /* Declare the labels. */
   demo = newCDKLabel (cdkscreen, CENTER, CENTER,
		       (CDK_CSTRING2)mesg, 1,
		       boxLabel, FALSE);

   /* Is the label null??? */
   if (demo == 0)
   {
      /* Clean up the memory. */
      destroyCDKScreen (cdkscreen);

      /* End curses... */
      endCDK ();

      printf ("Cannot create the label. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   curs_set (0);
   wtimeout (WindowOf (demo), 50);

   /* Do this for-a-while... */
   do
   {
      struct tm *currentTime;

      /* Get the current time. */
      time (&clck);
      currentTime = localtime (&clck);

      /* Put the current time in a string. */
      sprintf (temp, "<C></B/29>%02d:%02d:%02d",
	       currentTime->tm_hour,
	       currentTime->tm_min,
	       currentTime->tm_sec);
      mesg[0] = copyChar (temp);

      /* Set the label contents. */
      setCDKLabel (demo, (CDK_CSTRING2)mesg, 1, ObjOf (demo)->box);

      /* Draw the label, and sleep. */
      drawCDKLabel (demo, ObjOf (demo)->box);
      napms (500);
   }
   while (wgetch (WindowOf (demo)) == ERR);

   /* Clean up */
   destroyCDKLabel (demo);
   destroyCDKScreen (cdkscreen);
   endCDK ();

   ExitProgram (EXIT_SUCCESS);
}

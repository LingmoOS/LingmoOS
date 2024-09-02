/* $Id: marquee_ex.c,v 1.13 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "marquee_ex";
#endif

static char startAttr[100];
static char endAttr[100];

/*
 * This spits out help about this demo program.
 */
static void help (char *programName)
{
   const char *USAGE = "[-m Message] [-R repeat value] [-d delay value] [-b|r|u|k] [-h]";

   printf ("Usage: %s %s\n", programName, USAGE);
   printf (" -m TEXT   Sets the message to display in the marquee\n");
   printf ("           If no message is provided, one will be created.\n");
   printf (" -R COUNT  Repeat the message the given COUNT.\n");
   printf ("           A of -1 repeats the message forever.\n");
   printf (" -d COUNT  number of milliseconds to delay between repeats.\n");
   printf (" -b        display the message with the bold attribute.\n");
   printf (" -r        display the message with a reversed attribute.\n");
   printf (" -u        display the message with an underline attribute.\n");
   printf (" -k        display the message with the blinking attribute.\n");
}

static void myParseAttr (CDK_PARAMS * params, int lower, int upper)
{
   if (CDKparamString (params, lower) != 0)
   {
      char starting[3];
      char ending[3];

      if (startAttr[0] == '\0')
      {
	 startAttr[0] = '<';
	 endAttr[0] = '<';
      }
      sprintf (starting, "/%c", upper);
      sprintf (ending, "!%c", upper);
      strcat (startAttr, starting);
      strcat (endAttr, ending);
   }
}

int main (int argc, char **argv)
{
   /* *INDENT-OFF* */
   CDKSCREEN	*cdkscreen;
   CDKMARQUEE	*scrollMessage;
   char		message[1024];
   time_t	clck;

   CDK_PARAMS   params;
   char         *mesg;
   int          delay;
   int          repeat;
   /* *INDENT-ON* */


   CDKparseParams (argc, argv, &params, "brkud:R:m:hw:" CDK_MIN_PARAMS);
   myParseAttr (&params, 'b', 'B');
   myParseAttr (&params, 'r', 'R');
   myParseAttr (&params, 'k', 'K');
   myParseAttr (&params, 'u', 'U');
   repeat = CDKparamNumber2 (&params, 'R', 3);
   delay = CDKparamNumber2 (&params, 'd', 5);
   mesg = CDKparamString (&params, 'm');

   if (CDKparamString (&params, 'h') != 0)
      help (argv[0]);

   /* Clean up the strings. */
   cleanChar (message, sizeof (message), '\0');
   cleanChar (startAttr, sizeof (startAttr), '\0');
   cleanChar (endAttr, sizeof (endAttr), '\0');

   /* Put the end of the attributes if they asked for then. */
   if (startAttr[0] == '<')
   {
      strcat (startAttr, ">");
      strcat (endAttr, ">");
   }

   cdkscreen = initCDKScreen (NULL);
   curs_set (0);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Create the marquee. */
   scrollMessage = newCDKMarquee (cdkscreen,
				  CDKparamValue (&params, 'X', CENTER),
				  CDKparamValue (&params, 'Y', TOP),
				  CDKparamValue (&params, 'w', 30),
				  CDKparamValue (&params, 'N', FALSE),
				  CDKparamValue (&params, 'S', TRUE));

   /* Check if the marquee is null. */
   if (scrollMessage == 0)
   {
      /* Exit Cdk. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create the marquee window. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Draw the CDK screen. */
   refreshCDKScreen (cdkscreen);

   /* Create the marquee message. */
   if (mesg == 0)
   {
      char *currentTime;

      /* Get the current time and chop off the newline. */
      time (&clck);
      currentTime = ctime (&clck);
      currentTime[strlen (currentTime) - 1] = 0;

      if (startAttr[0] != '\0')
      {
	 currentTime[strlen (currentTime) - 1] = '\0';
	 sprintf (message, "%s%s%s (This Space For Rent) ",
		  startAttr,
		  currentTime,
		  endAttr);
      }
      else
      {
	 sprintf (message, "%s (This Space For Rent) ", currentTime);
      }
   }
   else
   {
      if (startAttr[0] != '\0')
      {
	 sprintf (message, "%s%s%s ", startAttr, mesg, endAttr);
      }
      else
      {
	 sprintf (message, "%s ", mesg);
      }
   }

   /* Run the marquee. */
   activateCDKMarquee (scrollMessage, message, delay, repeat, TRUE);
   activateCDKMarquee (scrollMessage, message, delay, repeat, FALSE);
   activateCDKMarquee (scrollMessage, message, delay, repeat, TRUE);

   /* Clean up. */
   destroyCDKMarquee (scrollMessage);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

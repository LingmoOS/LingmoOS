/* $Id: cdklabel.c,v 1.15 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef XCURSES
char *XCursesProgramName = "cdklabel";
#endif

#if !defined (HAVE_SLEEP) && defined (_WIN32)	/* Mingw */
#define sleep(x) _sleep(x*1000)
#endif

/*
 * Declare file local variables.
 */
static const char *FPUsage = "-m Message String | -f filename [-c Command] [-p Pause Character] [-s Sleep] [-X X Position] [-Y Y Position] [-N] [-S]";

/*
 *
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkScreen         = 0;
   CDKLABEL *widget             = 0;
   char *CDK_WIDGET_COLOR       = 0;
   char *temp                   = 0;
   chtype *holder               = 0;
   int messageLines             = -1;
   char **messageList           = 0;
   char tempCommand[1000];
   int j1, j2;

   CDK_PARAMS params;
   boolean boxWidget;
   boolean shadowWidget;
   char *command;
   char *filename;
   char *message;
   char waitChar = 0;
   int sleepLength;
   int xpos;
   int ypos;

   CDKparseParams (argc, argv, &params, "c:f:m:p:s:" CDK_MIN_PARAMS);

   /* *INDENT-EQLS* */
   xpos         = CDKparamValue (&params, 'X', CENTER);
   ypos         = CDKparamValue (&params, 'Y', CENTER);
   boxWidget    = CDKparamValue (&params, 'N', TRUE);
   shadowWidget = CDKparamValue (&params, 'S', FALSE);
   sleepLength  = CDKparamValue (&params, 's', 0);
   command      = CDKparamString (&params, 'c');
   filename     = CDKparamString (&params, 'f');
   message      = CDKparamString (&params, 'm');

   if ((temp = CDKparamString (&params, 'p')) != 0)
      waitChar = *temp;

   /* Make sure we have a message to display. */
   if (message == 0)
   {
      /* No message, maybe they provided a file to read. */
      if (filename != 0)
      {
	 /* Read the file in. */
	 messageLines = CDKreadFile (filename, &messageList);

	 /* Check if there was an error. */
	 if (messageLines == -1)
	 {
	    fprintf (stderr, "Error: Could not open the file %s\n", filename);
	    ExitProgram (CLI_ERROR);
	 }
      }
      else
      {
	 /* No message, no file, it's an error. */
	 fprintf (stderr, "Usage: %s %s\n", argv[0], FPUsage);
	 ExitProgram (CLI_ERROR);
      }
   }
   else
   {
      /* Split the message up. */
      messageList = CDKsplitString (message, '\n');
      messageLines = (int)CDKcountStrings ((CDK_CSTRING2) messageList);
   }

   cdkScreen = initCDKScreen (NULL);

   /* Start color. */
   initCDKColor ();

   /* Check if the user wants to set the background of the main screen. */
   if ((temp = getenv ("CDK_SCREEN_COLOR")) != 0)
   {
      holder = char2Chtype (temp, &j1, &j2);
      wbkgd (cdkScreen->window, holder[0]);
      wrefresh (cdkScreen->window);
      freeChtype (holder);
   }

   /* Get the widget color background color. */
   if ((CDK_WIDGET_COLOR = getenv ("CDK_WIDGET_COLOR")) == 0)
   {
      CDK_WIDGET_COLOR = 0;
   }

   /* Create the label widget. */
   widget = newCDKLabel (cdkScreen, xpos, ypos,
			 (CDK_CSTRING2) messageList, messageLines,
			 boxWidget, shadowWidget);

   /* Make sure we could create the widget. */
   if (widget == 0)
   {
      CDKfreeStrings (messageList);

      destroyCDKScreen (cdkScreen);
      endCDK ();

      fprintf (stderr,
	       "Error: Could not create the label. "
	       "Is the window too small?\n");

      ExitProgram (CLI_ERROR);
   }

   /* Check if the user wants to set the background of the widget. */
   setCDKLabelBackgroundColor (widget, CDK_WIDGET_COLOR);

   /* Draw the widget. */
   drawCDKLabel (widget, boxWidget);

   /* If they supplied a command, run it. */
   if (command != 0)
   {
      const char *fmt = "(sh -c %.*s) >/dev/null 2>&1";
      sprintf (tempCommand, fmt, (int)(sizeof (tempCommand) - strlen (fmt)), command);
      system (tempCommand);
   }

   /* If they supplied a wait character, wait for the user to hit it. */
   if (waitChar != 0)
   {
      waitCDKLabel (widget, waitChar);
   }

   /* If they supplied a sleep time, sleep for the given length. */
   if (sleepLength > 0)
   {
      sleep ((unsigned)sleepLength);
   }

   CDKfreeStrings (messageList);

   destroyCDKLabel (widget);
   destroyCDKScreen (cdkScreen);
   endCDK ();

   /* Exit cleanly. */
   ExitProgram (0);
}

/* $Id: cdkscroll.c,v 1.13 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef XCURSES
char *XCursesProgramName = "cdkscroll";
#endif

/*
 * Declare file local prototypes.
 */
static int widgetCB (EObjectType cdktype, void *object, void *clientData, chtype key);

/*
 * Define file local variables.
 */
static const char *FPUsage = "-l List | -f filename [-s Scroll Bar Position] [-n Numbers] [-i Item Index] [-T Title] [-B Buttons] [-O Output File] [-X X Position] [-Y Y Position] [-H Height] [-W Width] [-N] [-S]";

/*
 *
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkScreen         = 0;
   CDKSCROLL *widget            = 0;
   CDKBUTTONBOX *buttonWidget   = 0;
   char *CDK_WIDGET_COLOR       = 0;
   char *temp                   = 0;
   chtype *holder               = 0;
   int answer                   = 0;
   int spos                     = NONE;
   int buttonCount              = 0;
   int selection                = 0;
   int scrollLines              = -1;
   int shadowHeight             = 0;
   FILE *fp                     = stderr;
   char **scrollList            = 0;
   char **buttonList            = 0;
   int j1, j2;

   CDK_PARAMS params;
   boolean boxWidget;
   boolean shadowWidget;
   char *buttons;
   char *filename;
   char *list;
   char *outputFile;
   char *title;
   int height;
   int numberOutput;
   int numbers;
   int width;
   int xpos;
   int ypos;

   CDKparseParams (argc, argv, &params, "f:il:ns:B:O:T:" CDK_CLI_PARAMS);

   /* *INDENT-EQLS* */
   xpos            = CDKparamValue (&params, 'X', CENTER);
   ypos            = CDKparamValue (&params, 'Y', CENTER);
   height          = CDKparamValue (&params, 'H', 1);
   width           = CDKparamValue (&params, 'W', 1);
   boxWidget       = CDKparamValue (&params, 'N', TRUE);
   shadowWidget    = CDKparamValue (&params, 'S', FALSE);
   numberOutput    = CDKparamValue (&params, 'i', FALSE);
   numbers         = CDKparamValue (&params, 'n', FALSE);
   filename        = CDKparamString (&params, 'f');
   list            = CDKparamString (&params, 'l');
   buttons         = CDKparamString (&params, 'B');
   outputFile      = CDKparamString (&params, 'O');
   title           = CDKparamString (&params, 'T');

   spos = CDKparsePosition (CDKparamString (&params, 's'));

   /* If the user asked for an output file, try to open it. */
   if (outputFile != 0)
   {
      if ((fp = fopen (outputFile, "w")) == 0)
      {
	 fprintf (stderr, "%s: Can not open output file %s\n", argv[0], outputFile);
	 ExitProgram (CLI_ERROR);
      }
   }

   /* Did they provide a list of items. */
   if (list == 0)
   {
      /* Maybe they gave a filename to use to read. */
      if (filename != 0)
      {
	 /* Read the file in. */
	 scrollLines = CDKreadFile (filename, &scrollList);

	 /* Check if there was an error. */
	 if (scrollLines == -1)
	 {
	    fprintf (stderr, "Error: Could not open the file '%s'.\n", filename);
	    ExitProgram (CLI_ERROR);
	 }
      }
      else
      {
	 /* They didn't provide anything. */
	 fprintf (stderr, "Usage: %s %s\n", argv[0], FPUsage);
	 ExitProgram (CLI_ERROR);
      }
   }
   else
   {
      /* Split the scroll lines up. */
      scrollList = CDKsplitString (list, '\n');
      scrollLines = (int)CDKcountStrings ((CDK_CSTRING2) scrollList);
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

   /* Create the scrolling list. */
   widget = newCDKScroll (cdkScreen, xpos, ypos, spos,
			  height, width, title,
			  (CDK_CSTRING2) scrollList, scrollLines,
			  numbers, A_REVERSE,
			  boxWidget, shadowWidget);

   /* Make sure we could create the widget. */
   if (widget == 0)
   {
      CDKfreeStrings (scrollList);

      destroyCDKScreen (cdkScreen);
      endCDK ();

      fprintf (stderr,
	       "Error: Could not create the scrolling list. "
	       "Is the window too small?\n");

      ExitProgram (CLI_ERROR);
   }

   /* Split the buttons if they supplied some. */
   if (buttons != 0)
   {
      /* Split the button list up. */
      buttonList = CDKsplitString (buttons, '\n');
      buttonCount = (int)CDKcountStrings ((CDK_CSTRING2) buttonList);

      /* We need to create a buttonbox widget. */
      buttonWidget = newCDKButtonbox (cdkScreen,
				      getbegx (widget->win),
				      (getbegy (widget->win)
				       + widget->boxHeight - 1),
				      1, widget->boxWidth - 1,
				      0, 1, buttonCount,
				      (CDK_CSTRING2) buttonList, buttonCount,
				      A_REVERSE, boxWidget, FALSE);
      CDKfreeStrings (buttonList);

      setCDKButtonboxULChar (buttonWidget, ACS_LTEE);
      setCDKButtonboxURChar (buttonWidget, ACS_RTEE);

      /*
       * We need to set the lower left and right
       * characters of the widget.
       */
      setCDKScrollLLChar (widget, ACS_LTEE);
      setCDKScrollLRChar (widget, ACS_RTEE);

      /*
       * Bind the Tab key in the widget to send a
       * Tab key to the button box widget.
       */
      bindCDKObject (vSCROLL, widget, KEY_TAB, widgetCB, buttonWidget);
      bindCDKObject (vSCROLL, widget, CDK_NEXT, widgetCB, buttonWidget);
      bindCDKObject (vSCROLL, widget, CDK_PREV, widgetCB, buttonWidget);

      /* Check if the user wants to set the background of the widget. */
      setCDKButtonboxBackgroundColor (buttonWidget, CDK_WIDGET_COLOR);

      /* Draw the button widget. */
      drawCDKButtonbox (buttonWidget, boxWidget);
   }

   /*
    * If the user asked for a shadow, we need to create one.  Do this instead
    * of using the shadow parameter because the button widget is not part of
    * the main widget and if the user asks for both buttons and a shadow, we
    * need to create a shadow big enough for both widgets.  Create the shadow
    * window using the widgets shadowWin element, so screen refreshes will draw
    * them as well.
    */
   if (shadowWidget == TRUE)
   {
      /* Determine the height of the shadow window. */
      shadowHeight = (buttonWidget == 0 ?
		      widget->boxHeight :
		      widget->boxHeight + buttonWidget->boxHeight - 1);

      /* Create the shadow window. */
      widget->shadowWin = newwin (shadowHeight,
				  widget->boxWidth,
				  getbegy (widget->win) + 1,
				  getbegx (widget->win) + 1);

      /* Make sure we could have created the shadow window. */
      if (widget->shadowWin != 0)
      {
	 widget->shadow = TRUE;

	 /*
	  * We force the widget and buttonWidget to be drawn so the
	  * buttonbox widget will be drawn when the widget is activated.
	  * Otherwise the shadow window will draw over the button widget.
	  */
	 drawCDKScroll (widget, ObjOf (widget)->box);
	 eraseCDKButtonbox (buttonWidget);
	 drawCDKButtonbox (buttonWidget, ObjOf (buttonWidget)->box);
      }
   }

   /* Check if the user wants to set the background of the widget. */
   setCDKScrollBackgroundColor (widget, CDK_WIDGET_COLOR);

   /* Activate the scrolling list. */
   answer = activateCDKScroll (widget, 0);

   /* If there were buttons, get the button selected. */
   if (buttonWidget != 0)
   {
      selection = buttonWidget->currentButton;
      destroyCDKButtonbox (buttonWidget);
   }

   /* Shut down curses. */
   destroyCDKScroll (widget);
   destroyCDKScreen (cdkScreen);
   endCDK ();

   /* Print out the answer. */
   if (answer >= 0)
   {
      if (numberOutput == TRUE)
      {
	 fprintf (fp, "%d\n", answer);
      }
      else
      {
	 fprintf (fp, "%s\n", scrollList[answer]);
      }
   }
   fclose (fp);

   CDKfreeStrings (scrollList);

   /* Exit with the answer. */
   ExitProgram (selection);
}

static int widgetCB (EObjectType cdktype GCC_UNUSED,
		     void *object GCC_UNUSED,
		     void *clientData,
		     chtype key)
{
   CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)clientData;
   (void) injectCDKButtonbox (buttonbox, key);
   return (TRUE);
}

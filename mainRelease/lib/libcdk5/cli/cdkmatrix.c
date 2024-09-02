/* $Id: cdkmatrix.c,v 1.19 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef XCURSES
char *XCursesProgramName = "cdkmatrix";
#endif

#define MY_INFO(x,y)	info[(x + 1) * cols + (y + 1)]

/*
 * Declare file local prototypes.
 */
static int widgetCB (EObjectType cdktype, void *object, void *clientData, chtype key);

/*
 * Define file local variables.
 */
static const char *FPUsage = "-r Row Titles -c Column Titles -v Visible Rows -w Column Widths [-t Column Types] [-d Default Values] [-F Field Character] [-T Title] [-B Buttons] [-O Output File] [-X X Position] [-Y Y Position] [-N] [-S]";

/*
 *
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkScreen         = 0;
   CDKMATRIX *widget            = 0;
   CDKBUTTONBOX *buttonWidget   = 0;
   chtype *holder               = 0;
   char *buttons                = 0;
   char *CDK_WIDGET_COLOR       = 0;
   char *temp                   = 0;
   chtype filler                = A_NORMAL | '.';
   int rows                     = -1;
   int cols                     = -1;
   int buttonCount              = 0;
   int selection                = 0;
   int shadowHeight             = 0;
   FILE *fp                     = stderr;
   char **rowTitles;
   char **colTitles;
   char **rowTemp               = 0;
   char **colTemp               = 0;
   char **kolTemp               = 0;
   char **buttonList            = 0;
   int *colWidths;
   int *colTypes;
   int count, infoLines, x, y, j1, j2;

   CDK_PARAMS params;
   boolean boxWidget;
   boolean shadowWidget;
   char *defaultValue;
   char *myColTitles;
   char *myColTypes;
   char *myColWidths;
   char *myFiller;
   char *myRowTitles;
   char *outputFile;
   char *title;
   int vrows;
   int xpos;
   int ypos;

   CDKparseParams (argc, argv, &params, "c:d:r:t:w:v:B:F:O:T:" CDK_MIN_PARAMS);

   /* *INDENT-EQLS* */
   xpos         = CDKparamValue (&params, 'X', CENTER);
   ypos         = CDKparamValue (&params, 'Y', CENTER);
   boxWidget    = CDKparamValue (&params, 'N', TRUE);
   shadowWidget = CDKparamValue (&params, 'S', FALSE);
   vrows        = CDKparamValue (&params, 'v', -1);
   myColTitles  = CDKparamString (&params, 'c');
   defaultValue = CDKparamString (&params, 'd');
   myRowTitles  = CDKparamString (&params, 'r');
   myColTypes   = CDKparamString (&params, 't');
   myColWidths  = CDKparamString (&params, 'w');
   buttons      = CDKparamString (&params, 'B');
   myFiller     = CDKparamString (&params, 'F');
   outputFile   = CDKparamString (&params, 'O');
   title        = CDKparamString (&params, 'T');

   /* If the user asked for an output file, try to open it. */
   if (outputFile != 0)
   {
      if ((fp = fopen (outputFile, "w")) == 0)
      {
	 fprintf (stderr, "%s: Can not open output file %s\n", argv[0], outputFile);
	 ExitProgram (CLI_ERROR);
      }
   }

   /* Make sure all the needed command line parameters were provided. */
   if ((myRowTitles == 0) ||
       (myColTitles == 0) ||
       (myColWidths == 0) ||
       (vrows == -1))
   {
      fprintf (stderr, "Usage: %s %s\n", argv[0], FPUsage);
      ExitProgram (CLI_ERROR);
   }

   /* Convert the char * titles to a char **, offset by one */
   rowTemp = CDKsplitString (myRowTitles, '\n');
   rows = (int)CDKcountStrings ((CDK_CSTRING2)rowTemp);
   rowTitles = (char **)calloc ((size_t) rows + 1, sizeof (char *));
   for (x = 0; x < rows; x++)
   {
      rowTitles[x + 1] = rowTemp[x];
   }

   colTemp = CDKsplitString (myColTitles, '\n');
   cols = (int)CDKcountStrings ((CDK_CSTRING2)colTemp);
   colTitles = (char **)calloc ((size_t) cols + 1, sizeof (char *));
   for (x = 0; x < cols; x++)
   {
      colTitles[x + 1] = colTemp[x];
   }

   /* Convert the column widths. */
   kolTemp = CDKsplitString (myColWidths, '\n');
   count = (int)CDKcountStrings ((CDK_CSTRING2)kolTemp);
   colWidths = (int *)calloc ((size_t) count + 1, sizeof (int));
   for (x = 0; x < count; x++)
   {
      colWidths[x + 1] = atoi (kolTemp[x]);
   }

   /* If they passed in the column types, convert them. */
   if (myColTypes != 0)
   {
      char **ss = CDKsplitString (myColTypes, '\n');
      count = (int)CDKcountStrings ((CDK_CSTRING2)ss);
      colTypes = (int *)calloc ((size_t) MAXIMUM (cols, count) + 1, sizeof (int));
      for (x = 0; x < count; x++)
      {
	 colTypes[x + 1] = char2DisplayType (ss[x]);
      }
      CDKfreeStrings (ss);
   }
   else
   {
      /* If they didn't set default values. */
      colTypes = (int *)calloc ((size_t) cols + 1, sizeof (int));
      for (x = 0; x < cols; x++)
      {
	 colTypes[x + 1] = vMIXED;
      }
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

   /* If the set the filler character, set it now. */
   if (myFiller != 0)
   {
      holder = char2Chtype (myFiller, &j1, &j2);
      filler = holder[0];
      freeChtype (holder);
   }

   /* Create the matrix widget. */
   widget = newCDKMatrix (cdkScreen, xpos, ypos,
			  rows, cols, vrows, cols,
			  title, (CDK_CSTRING2)rowTitles, (CDK_CSTRING2)colTitles,
			  colWidths, colTypes, 1, 1,
			  filler, COL,
			  boxWidget, TRUE, shadowWidget);
   free (rowTitles);
   free (colTitles);

   /* Make sure we could create the widget. */
   if (widget == 0)
   {
      /* Shut down curses and CDK. */
      destroyCDKScreen (cdkScreen);
      endCDK ();

      fprintf (stderr,
	       "Error: Cannot create the matrix. "
	       "Is the window too small?\n");

      ExitProgram (CLI_ERROR);
   }

   /*
    * If the user sent in a file of default values, read it and
    * stick the values read in from the file into the matrix.
    */
   if (defaultValue != 0)
   {
      size_t limit = (size_t) ((rows + 1) * (cols + 1));
      char **info = (char **)calloc (limit, sizeof (char *));
      char **lineTemp = 0;

      /* Read the file. */
      infoLines = CDKreadFile (defaultValue, &lineTemp);
      if (infoLines > 0)
      {
	 int *subSize = (int *)calloc ((size_t) infoLines + 1, sizeof (int));

	 /* For each line, split on a CTRL-V. */
	 for (x = 0; x < infoLines; x++)
	 {
	    char **ss = CDKsplitString (lineTemp[x], CTRL ('V'));
	    subSize[x + 1] = (int)CDKcountStrings ((CDK_CSTRING2)ss);
	    for (y = 0; y < subSize[x + 1]; y++)
	    {
	       MY_INFO (x, y) = ss[y];
	    }
	    free (ss);
	 }
	 CDKfreeStrings (lineTemp);

	 setCDKMatrixCells (widget, (CDK_CSTRING2)info, rows, cols, subSize);

	 for (x = 0; x < infoLines; x++)
	 {
	    for (y = 0; y < subSize[x + 1]; y++)
	    {
	       freeChar (MY_INFO (x, y));
	    }
	 }
	 free (info);
	 free (subSize);
      }
   }

   /* Split the buttons if they supplied some. */
   if (buttons != 0)
   {
      /* Split the button list up. */
      buttonList = CDKsplitString (buttons, '\n');
      buttonCount = (int)CDKcountStrings ((CDK_CSTRING2)buttonList);

      /* We need to create a buttonbox widget. */
      buttonWidget = newCDKButtonbox (cdkScreen,
				      getbegx (widget->win),
				      (getbegy (widget->win)
				       + widget->boxHeight - 1),
				      1, widget->boxWidth - 1,
				      NULL, 1, buttonCount,
				      (CDK_CSTRING2)buttonList, buttonCount,
				      A_REVERSE, boxWidget, FALSE);

      setCDKButtonboxULChar (buttonWidget, ACS_LTEE);
      setCDKButtonboxURChar (buttonWidget, ACS_RTEE);

      /*
       * We need to set the lower left and right
       * characters of the widget.
       */
      setCDKMatrixLLChar (widget, ACS_LTEE);
      setCDKMatrixLRChar (widget, ACS_RTEE);

      /*
       * Bind the Tab key in the widget to send a
       * Tab key to the button box widget.
       */
      bindCDKObject (vMATRIX, widget, KEY_TAB, widgetCB, buttonWidget);
      bindCDKObject (vMATRIX, widget, CDK_NEXT, widgetCB, buttonWidget);
      bindCDKObject (vMATRIX, widget, CDK_PREV, widgetCB, buttonWidget);

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
      shadowHeight = (buttonWidget == (CDKBUTTONBOX *)NULL ?
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
	 drawCDKMatrix (widget, ObjOf (widget)->box);
	 eraseCDKButtonbox (buttonWidget);
	 drawCDKButtonbox (buttonWidget, ObjOf (buttonWidget)->box);
      }
   }

   /* Check if the user wants to set the background of the widget. */
   setCDKMatrixBackgroundColor (widget, CDK_WIDGET_COLOR);

   /* Let them play. */
   activateCDKMatrix (widget, 0);

   /* Print out the matrix cells. */
   if (widget->exitType == vNORMAL)
   {
      for (x = 0; x < widget->rows; x++)
      {
	 for (y = 0; y < widget->cols; y++)
	 {
	    char *data = getCDKMatrixCell (widget, x, y);
	    if (data != 0)
	    {
	       fprintf (fp, "%s%c", data, CTRL ('V'));
	    }
	    else
	    {
	       fprintf (fp, "%c", CTRL ('V'));
	    }
	 }
	 fprintf (fp, "\n");
      }
   }

   /* If there were buttons, get the button selected. */
   if (buttonWidget != 0)
   {
      selection = buttonWidget->currentButton;
      destroyCDKButtonbox (buttonWidget);
   }

   /* cleanup (not really needed) */
   CDKfreeStrings (buttonList);
   free (colTypes);
   free (colWidths);

   CDKfreeStrings (rowTemp);
   CDKfreeStrings (colTemp);
   CDKfreeStrings (kolTemp);

   destroyCDKMatrix (widget);
   destroyCDKScreen (cdkScreen);
   endCDK ();

   /* do this late, in case it was stderr */
   fclose (fp);

   ExitProgram (selection);
}

static int widgetCB (EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED,
		     void *clientData,
		     chtype key)
{
   CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)clientData;
   (void)injectCDKButtonbox (buttonbox, key);
   return (TRUE);
}

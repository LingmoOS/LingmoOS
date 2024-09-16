/* $Id: scale_ex.c,v 1.11 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "scale_ex";
#endif

/*
 * This program demonstrates the Cdk scale widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKSCALE *widget     = 0;
   const char *title    = "<C>Select a value";
   const char *label    = "</5>Current value";
   char temp[256];
   const char *mesg[5];
   int selection;

   CDK_PARAMS params;
   int high;
   int inc;
   int low;

   CDKparseParams (argc, argv, &params, "h:i:l:w:" CDK_MIN_PARAMS);
   high = CDKparamNumber2 (&params, 'h', 100);
   inc = CDKparamNumber2 (&params, 'i', 1);
   low = CDKparamNumber2 (&params, 'l', 0);

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Create the widget. */
   widget = newCDKScale (cdkscreen,
			 CDKparamValue (&params, 'X', CENTER),
			 CDKparamValue (&params, 'Y', CENTER),
			 title, label,
			 A_NORMAL,
			 CDKparamNumber2 (&params, 'w', 5),
			 low, low, high,
			 inc, (inc * 2),
			 CDKparamValue (&params, 'N', TRUE),
			 CDKparamValue (&params, 'S', FALSE));

   /* Is the widget null? */
   if (widget == 0)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot make the widget. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Activate the widget. */
   selection = activateCDKScale (widget, 0);

   /* Check the exit value of the widget. */
   if (widget->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No value selected.";
      mesg[1] = "",
	 mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }
   else if (widget->exitType == vNORMAL)
   {
      sprintf (temp, "<C>You selected %d", selection);
      mesg[0] = temp;
      mesg[1] = "",
	 mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }

   /* Clean up. */
   destroyCDKScale (widget);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

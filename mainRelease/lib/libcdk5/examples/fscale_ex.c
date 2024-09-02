/* $Id: fscale_ex.c,v 1.9 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "fscale_ex";
#endif

static float myFloatParam (CDK_PARAMS * params, int code, double missing)
{
   char *opt = CDKparamString (params, code);
   double result = missing;

   if (opt != 0)
      result = atof (opt);
   return (float)result;
}

/*
 * This program demonstrates the Cdk scale widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKFSCALE *scale     = 0;
   const char *title    = "<C>Select a value";
   const char *label    = "</5>Current value";
   char temp[256];
   const char *mesg[5];
   float selection;

   CDK_PARAMS params;
   float high;
   float inc;
   float low;

   /* *INDENT-EQLS* */
   CDKparseParams (argc, argv, &params, "h:i:l:w:" CDK_MIN_PARAMS);
   high   = myFloatParam (&params, 'h', 2.4);
   inc    = myFloatParam (&params, 'i', 0.2);
   low    = myFloatParam (&params, 'l', -1.2);

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Create the scale. */
   scale = newCDKFScale (cdkscreen,
			 CDKparamValue (&params, 'X', CENTER),
			 CDKparamValue (&params, 'Y', CENTER),
			 title, label, A_NORMAL,
			 CDKparamNumber2 (&params, 'w', 10),
			 low, low, high,
			 inc, (inc * (float)2.), 1,
			 CDKparamValue (&params, 'N', TRUE),
			 CDKparamValue (&params, 'S', FALSE));

   /* Is the scale null? */
   if (scale == 0)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Can't make the scale widget. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Activate the scale. */
   selection = activateCDKFScale (scale, 0);

   /* Check the exit value of the scale widget. */
   if (scale->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No value selected.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }
   else if (scale->exitType == vNORMAL)
   {
      sprintf (temp, "<C>You selected %f", selection);
      mesg[0] = temp;
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }

   /* Clean up. */
   destroyCDKFScale (scale);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

/* $Id: histogram_ex.c,v 1.12 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "histogram_ex";
#endif

#if !defined (HAVE_SLEEP) && defined (_WIN32)	/* Mingw */
#define sleep(x) _sleep(x*1000)
#endif

int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen         = 0;
   CDKHISTOGRAM *volume         = 0;
   CDKHISTOGRAM *bass           = 0;
   CDKHISTOGRAM *treble         = 0;
   const char *volumeTitle      = "<C></5>Volume<!5>";
   const char *bassTitle        = "<C></5>Bass  <!5>";
   const char *trebleTitle      = "<C></5>Treble<!5>";

   CDK_PARAMS params;
   boolean Box;

   CDKparseParams (argc, argv, &params, CDK_CLI_PARAMS);
   Box = CDKparamValue (&params, 'N', TRUE);

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Color. */
   initCDKColor ();

   /* Create the histogram objects. */
   volume = newCDKHistogram (cdkscreen,
			     CDKparamValue (&params, 'X', 10),
			     CDKparamValue (&params, 'Y', 10),
			     CDKparamValue (&params, 'H', 1),
			     CDKparamValue (&params, 'W', -2),
			     HORIZONTAL, volumeTitle,
			     Box,
			     CDKparamValue (&params, 'S', FALSE));
   if (volume == 0)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot make volume histogram. Is the window big enough??\n");
      ExitProgram (EXIT_FAILURE);
   }

   bass = newCDKHistogram (cdkscreen,
			   CDKparamValue (&params, 'X', 10),
			   CDKparamValue (&params, 'Y', 14),
			   CDKparamValue (&params, 'H', 1),
			   CDKparamValue (&params, 'W', -2),
			   HORIZONTAL, bassTitle,
			   Box,
			   CDKparamValue (&params, 'S', FALSE));
   if (bass == 0)
   {
      /* Exit CDK. */
      destroyCDKHistogram (volume);
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot make bass histogram. Is the window big enough??\n");
      ExitProgram (EXIT_FAILURE);
   }

   treble = newCDKHistogram (cdkscreen,
			     CDKparamValue (&params, 'X', 10),
			     CDKparamValue (&params, 'Y', 18),
			     CDKparamValue (&params, 'H', 1),
			     CDKparamValue (&params, 'W', -2),
			     HORIZONTAL, trebleTitle,
			     Box,
			     CDKparamValue (&params, 'S', FALSE));
   if (treble == 0)
   {
      /* Exit CDK. */
      destroyCDKHistogram (volume);
      destroyCDKHistogram (bass);
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot make treble histogram. Is the window big enough??\n");
      ExitProgram (EXIT_FAILURE);
   }

#define BAR(a,b,c) A_BOLD, a, b, c, ' '|A_REVERSE|COLOR_PAIR(3), Box

   /* Set the histogram values. */
   setCDKHistogram (volume, vPERCENT, CENTER, BAR (0, 10, 6));
   setCDKHistogram (bass, vPERCENT, CENTER, BAR (0, 10, 3));
   setCDKHistogram (treble, vPERCENT, CENTER, BAR (0, 10, 7));
   refreshCDKScreen (cdkscreen);
   sleep (4);

   /* Set the histogram values. */
   setCDKHistogram (volume, vPERCENT, CENTER, BAR (0, 10, 8));
   setCDKHistogram (bass, vPERCENT, CENTER, BAR (0, 10, 1));
   setCDKHistogram (treble, vPERCENT, CENTER, BAR (0, 10, 9));
   refreshCDKScreen (cdkscreen);
   sleep (4);

   /* Set the histogram values. */
   setCDKHistogram (volume, vPERCENT, CENTER, BAR (0, 10, 10));
   setCDKHistogram (bass, vPERCENT, CENTER, BAR (0, 10, 7));
   setCDKHistogram (treble, vPERCENT, CENTER, BAR (0, 10, 10));
   refreshCDKScreen (cdkscreen);
   sleep (4);

   /* Set the histogram values. */
   setCDKHistogram (volume, vPERCENT, CENTER, BAR (0, 10, 1));
   setCDKHistogram (bass, vPERCENT, CENTER, BAR (0, 10, 8));
   setCDKHistogram (treble, vPERCENT, CENTER, BAR (0, 10, 3));
   refreshCDKScreen (cdkscreen);
   sleep (4);

   /* Set the histogram values. */
   setCDKHistogram (volume, vPERCENT, CENTER, BAR (0, 10, 3));
   setCDKHistogram (bass, vPERCENT, CENTER, BAR (0, 10, 3));
   setCDKHistogram (treble, vPERCENT, CENTER, BAR (0, 10, 3));
   refreshCDKScreen (cdkscreen);
   sleep (4);

   /* Set the histogram values. */
   setCDKHistogram (volume, vPERCENT, CENTER, BAR (0, 10, 10));
   setCDKHistogram (bass, vPERCENT, CENTER, BAR (0, 10, 10));
   setCDKHistogram (treble, vPERCENT, CENTER, BAR (0, 10, 10));
   refreshCDKScreen (cdkscreen);
   sleep (4);

   /* Clean up. */
   destroyCDKHistogram (volume);
   destroyCDKHistogram (bass);
   destroyCDKHistogram (treble);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

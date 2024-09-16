/* $Id: label_ex.c,v 1.11 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "label_ex";
#endif

int main (int argc, char **argv)
{
   CDKSCREEN *cdkscreen;
   CDKLABEL *demo;
   const char *mesg[10];

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, CDK_MIN_PARAMS);

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Set the labels up. */
   mesg[0] = "</29/B>This line should have a yellow foreground and a blue background.";
   mesg[1] = "</5/B>This line should have a white  foreground and a blue background.";
   mesg[2] = "</26/B>This line should have a yellow foreground and a red  background.";
   mesg[3] = "<C>This line should be set to whatever the screen default is.";

   /* Declare the labels. */
   demo = newCDKLabel (cdkscreen,
		       CDKparamValue (&params, 'X', CENTER),
		       CDKparamValue (&params, 'Y', CENTER),
		       (CDK_CSTRING2) mesg, 4,
		       CDKparamValue (&params, 'N', TRUE),
		       CDKparamValue (&params, 'S', TRUE));

   /* Is the label null? */
   if (demo == 0)
   {
      /* Clean up the memory. */
      destroyCDKScreen (cdkscreen);

      /* End curses... */
      endCDK ();

      printf ("Cannot create the label. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Draw the CDK screen. */
   refreshCDKScreen (cdkscreen);
   waitCDKLabel (demo, ' ');

   /* Clean up. */
   destroyCDKLabel (demo);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

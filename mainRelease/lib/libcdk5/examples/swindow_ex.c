/* $Id: swindow_ex.c,v 1.14 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "swindow_ex";
#endif

#if !defined (HAVE_SLEEP) && defined (_WIN32)	/* Mingw */
#define sleep(x) _sleep(x*1000)
#endif

/*
 * Demonstrate a scrolling-window.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKSWINDOW *swindow  = 0;
   const char *title    = "<C></5>Error Log";
   const char *mesg[5];

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, CDK_CLI_PARAMS);

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK colors. */
   initCDKColor ();

   /* Create the scrolling window. */
   swindow = newCDKSwindow (cdkscreen,
			    CDKparamValue (&params, 'X', CENTER),
			    CDKparamValue (&params, 'Y', CENTER),
			    CDKparamValue (&params, 'H', 6),
			    CDKparamValue (&params, 'W', 65),
			    title, 100,
			    CDKparamValue (&params, 'N', TRUE),
			    CDKparamValue (&params, 'S', FALSE));

   /* Is the window null. */
   if (swindow == 0)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create the scrolling window. ");
      printf ("Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Draw the scrolling window. */
   drawCDKSwindow (swindow, ObjOf (swindow)->box);

   /* Load up the scrolling window. */
   addCDKSwindow (swindow, "<C></11>TOP: This is the first line.", BOTTOM);
   addCDKSwindow (swindow, "<C>Sleeping for 1 second.", BOTTOM);
   sleep (1);

   addCDKSwindow (swindow, "<L></11>1: This is another line.", BOTTOM);
   addCDKSwindow (swindow, "<C>Sleeping for 1 second.", BOTTOM);
   sleep (1);

   addCDKSwindow (swindow, "<C></11>2: This is another line.", BOTTOM);
   addCDKSwindow (swindow, "<C>Sleeping for 1 second.", BOTTOM);
   sleep (1);

   addCDKSwindow (swindow, "<R></11>3: This is another line.", BOTTOM);
   addCDKSwindow (swindow, "<C>Sleeping for 1 second.", BOTTOM);
   sleep (1);

   addCDKSwindow (swindow, "<C></11>4: This is another line.", BOTTOM);
   addCDKSwindow (swindow, "<C>Sleeping for 1 second.", BOTTOM);
   sleep (1);

   addCDKSwindow (swindow, "<L></11>5: This is another line.", BOTTOM);
   addCDKSwindow (swindow, "<C>Sleeping for 1 second.", BOTTOM);
   sleep (1);

   addCDKSwindow (swindow, "<C></11>6: This is another line.", BOTTOM);
   addCDKSwindow (swindow, "<C>Sleeping for 1 second.", BOTTOM);
   sleep (1);

   addCDKSwindow (swindow, "<C>Done. You can now play.", BOTTOM);

   addCDKSwindow (swindow, "<C>This is being added to the top.", TOP);

   /* Activate the scrolling window. */
   activateCDKSwindow (swindow, 0);

   /* Check how the user exited this widget. */
   if (swindow->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape to leave this widget.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }
   else if (swindow->exitType == vNORMAL)
   {
      mesg[0] = "<C>You hit return to exit this widget.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }

   /* Clean up. */
   destroyCDKSwindow (swindow);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

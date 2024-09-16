/* $Id: viewer_ex.c,v 1.22 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "viewer_ex";
#endif

/*
 * This program demonstrates the file selector and the viewer widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKVIEWER *example   = 0;
   CDKFSELECT *fSelect  = 0;
   char **info          = 0;
   const char *button[5];
   char vTitle[256];
   const char *mesg[4];
   char temp[256];
   int selected, lines;

   CDK_PARAMS params;
   char *filename;		/* specify filename, bypassing fselect */
   char *directory;		/* specify starting directory for fselect */
   int interp_it;		/* interpret embedded markup */
   int link_it;			/* load file via embedded link */

   CDKparseParams (argc, argv, &params, "f:d:il" CDK_CLI_PARAMS);
   /* *INDENT-EQLS* */
   filename     = CDKparamString (&params, 'f');
   directory    = CDKparamString2 (&params, 'd', ".");
   interp_it    = CDKparamNumber2 (&params, 'i', FALSE);
   link_it      = CDKparamNumber2 (&params, 'l', FALSE);

   /* Create the viewer buttons. */
   button[0] = "</5><OK><!5>";
   button[1] = "</5><Cancel><!5>";

   cdkscreen = initCDKScreen (NULL);

   /* Start color. */
   initCDKColor ();

   /* Get the filename. */
   if (filename == 0)
   {
      const char *title = "<C>Pick\n<C>A\n<C>File";
      const char *label = "File:  ";

      fSelect = newCDKFselect (cdkscreen,
			       CDKparamValue (&params, 'X', CENTER),
			       CDKparamValue (&params, 'Y', CENTER),
			       CDKparamValue (&params, 'H', 20),
			       CDKparamValue (&params, 'W', 65),
			       title, label, A_NORMAL, '_', A_REVERSE,
			       "</5>", "</48>", "</N>", "</N>",
			       CDKparamValue (&params, 'N', TRUE),
			       CDKparamValue (&params, 'S', FALSE));
      if (fSelect == 0)
      {
	 destroyCDKScreen (cdkscreen);
	 endCDK ();

	 fprintf (stderr, "Cannot create fselect-widget\n");
	 ExitProgram (EXIT_FAILURE);
      }

      /*
       * Set the starting directory. This is not necessary because when
       * the file selector starts it uses the present directory as a default.
       */
      setCDKFselect (fSelect, directory, A_NORMAL, '.', A_REVERSE,
		     "</5>", "</48>", "</N>", "</N>", ObjOf (fSelect)->box);

      /* Activate the file selector. */
      filename = activateCDKFselect (fSelect, 0);

      /* Check how the person exited from the widget. */
      if (fSelect->exitType == vESCAPE_HIT)
      {
	 /* Pop up a message for the user. */
	 mesg[0] = "<C>Escape hit. No file selected.";
	 mesg[1] = "";
	 mesg[2] = "<C>Press any key to continue.";
	 popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);

	 /* Exit CDK. */
	 destroyCDKFselect (fSelect);
	 destroyCDKScreen (cdkscreen);
	 endCDK ();
	 ExitProgram (EXIT_SUCCESS);
      }
   }

   /* Create the file viewer to view the file selected. */
   example = newCDKViewer (cdkscreen,
			   CDKparamValue (&params, 'X', CENTER),
			   CDKparamValue (&params, 'Y', CENTER),
			   CDKparamValue (&params, 'H', 20),
			   CDKparamValue (&params, 'W', -2),
			   (CDK_CSTRING2)button, 2, A_REVERSE,
			   CDKparamValue (&params, 'N', TRUE),
			   CDKparamValue (&params, 'S', FALSE));

   /* Could we create the viewer widget? */
   if (example == 0)
   {
      /* Exit CDK. */
      destroyCDKFselect (fSelect);
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create viewer. Is the window too small?\n");
      ExitProgram (EXIT_SUCCESS);
   }

   if (link_it)
   {
      info = (char **)calloc (2, sizeof (char *));
      info[0] = (char *)malloc (5 + strlen (filename));
      sprintf (info[0], "<F=%s>", filename);
      lines = -1;
      interp_it = TRUE;
   }
   else
   {
      setCDKViewer (example, "reading...", 0, 0, A_REVERSE, TRUE, TRUE, TRUE);
      /* Open the file and read the contents. */
      lines = CDKreadFile (filename, &info);
      if (lines == -1)
      {
	 endCDK ();
	 printf ("Could not open \"%s\"\n", filename);
	 ExitProgram (EXIT_FAILURE);
      }
   }

   /* Set up the viewer title, and the contents to the widget. */
   sprintf (vTitle, "<C></B/21>Filename:<!21></22>%20s<!22!B>", filename);
   setCDKViewer (example, vTitle,
		 (CDK_CSTRING2)info, lines,
		 A_REVERSE, interp_it, TRUE, TRUE);

   CDKfreeStrings (info);

   /* Destroy the file selector widget. */
   destroyCDKFselect (fSelect);

   /* Activate the viewer widget. */
   selected = activateCDKViewer (example, 0);

   /* Check how the person exited from the widget. */
   if (example->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>Escape hit. No Button selected.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
   }
   else if (example->exitType == vNORMAL)
   {
      sprintf (temp, "<C>You selected button %d", selected);
      mesg[0] = temp;
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
   }

   /* Clean up. */
   destroyCDKViewer (example);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

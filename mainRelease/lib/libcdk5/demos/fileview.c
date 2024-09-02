/* $Id: fileview.c,v 1.15 2021/01/09 22:41:39 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "codeViewer";
#endif

/*
 * This program demonstrates the file selector and the viewer widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen  = 0;
   CDKVIEWER *example    = 0;
   CDKFSELECT *fSelect   = 0;
   const char *directory = ".";
   char *filename        = 0;
   char **info           = 0;
   const char *button[5];
   const char *mesg[4];
   char vtitle[256];
   char temp[256];
   int selected, lines;

   /* Parse up the command line. */
   while (1)
   {
      int ret = getopt (argc, argv, "d:f:");
      if (ret == -1)
      {
	 break;
      }
      switch (ret)
      {
      case 'd':
	 directory = strdup (optarg);
	 break;

      case 'f':
	 filename = strdup (optarg);
	 break;
      }
   }

   /* Create the viewer buttons. */
   button[0] = "</5><OK><!5>";
   button[1] = "</5><Cancel><!5>";

   cdkscreen = initCDKScreen (NULL);

   /* Start color. */
   initCDKColor ();

   /* Get the filename. */
   if (filename == 0)
   {
      const char *title = "<C>Pick a file.";
      const char *label = "File:  ";

      fSelect = newCDKFselect (cdkscreen, CENTER, CENTER, 20, 65,
			       title, label, A_NORMAL, '_', A_REVERSE,
			       "</5>", "</48>", "</N>", "</N>", TRUE, FALSE);

      /*
       * Set the starting directory. This is not necessary because when
       * the file selector starts it uses the present directory as a default.
       */
      setCDKFselect (fSelect, directory, A_NORMAL, '.', A_REVERSE,
		     "</5>", "</48>", "</N>", "</N>", ObjOf (fSelect)->box);

      /* Activate the file selector. */
      filename = copyChar (activateCDKFselect (fSelect, 0));

      /* Check how the person exited from the widget. */
      if (fSelect->exitType == vESCAPE_HIT)
      {
	 /* Pop up a message for the user. */
	 mesg[0] = "<C>Escape hit. No file selected.";
	 mesg[1] = "";
	 mesg[2] = "<C>Press any key to continue.";
	 popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);

	 /* Destroy the file selector. */
	 destroyCDKFselect (fSelect);

	 /* Exit CDK. */
	 destroyCDKScreen (cdkscreen);
	 endCDK ();

	 ExitProgram (EXIT_SUCCESS);
      }
   }

   /* Destroy the file selector. */
   destroyCDKFselect (fSelect);

   /* Create the file viewer to view the file selected. */
   example = newCDKViewer (cdkscreen, CENTER, CENTER, 20, -2,
			   (CDK_CSTRING2)button, 2,
			   A_REVERSE, TRUE, FALSE);

   /* Could we create the viewer widget? */
   if (example == 0)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      /* Print out a message and exit. */
      printf ("Cannot create viewer. Is the window too small?\n");
      ExitProgram (EXIT_SUCCESS);
   }

   /* Open the file and read the contents. */
   lines = CDKreadFile (filename, &info);
   if (lines == -1)
   {
      printf ("Could not open %s\n", filename);
      ExitProgram (EXIT_FAILURE);
   }

   /* Set up the viewer title, and the contents to the widget. */
   sprintf (vtitle, "<C></B/22>%20s<!22!B>", filename);
   setCDKViewer (example, vtitle,
		 (CDK_CSTRING2)info, lines,
		 A_REVERSE, TRUE, TRUE, TRUE);

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
   CDKfreeStrings (info);
   freeChar (filename);

   endCDK ();

   ExitProgram (EXIT_SUCCESS);
}

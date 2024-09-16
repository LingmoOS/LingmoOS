/* $Id: radio_ex.c,v 1.21 2021/01/09 22:42:29 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "radio_ex";
#endif

/*
 * This program demonstrates the Cdk radio widget.
 *
 * Options (in addition to normal CLI parameters):
 *	-c	create the data after the widget
 *	-s SPOS	location for the scrollbar
 *	-t TEXT	title for the widget
 *
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKRADIO *radio      = 0;
   const char *title    = "<C></5>Select a filename";
   char **item          = 0;
   const char *mesg[5];
   char temp[256];
   int count;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, "cs:t:" CDK_CLI_PARAMS);

   /* Use the current directory list to fill the radio list. */
   count = CDKgetDirectoryContents (".", &item);
   if (count <= 0)
   {
      fprintf (stderr, "Cannot get directory list\n");
      ExitProgram (EXIT_FAILURE);
   }

   cdkscreen = initCDKScreen (NULL);

   /* Set up CDK Colors. */
   initCDKColor ();

   /* Create the radio list. */
   radio = newCDKRadio (cdkscreen,
			CDKparamValue (&params, 'X', CENTER),
			CDKparamValue (&params, 'Y', CENTER),
			CDKparsePosition (CDKparamString2 (&params,
							   's',
							   "RIGHT")),
			CDKparamValue (&params, 'H', 10),
			CDKparamValue (&params, 'W', 40),
			CDKparamString2 (&params, 't', title),
			CDKparamNumber (&params, 'c') ? 0 : (CDK_CSTRING2)item,
			CDKparamNumber (&params, 'c') ? 0 : count,
			'#' | A_REVERSE, 1,
			A_REVERSE,
			CDKparamValue (&params, 'N', TRUE),
			CDKparamValue (&params, 'S', FALSE));

   /* Check if the radio list is null. */
   if (radio == 0)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create the radio widget. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   if (CDKparamNumber (&params, 'c'))
   {
      setCDKRadioItems (radio, (CDK_CSTRING2)item, count);
   }

   /* loop until user selects a file, or cancels */
   for (;;)
   {
      /* Activate the radio list. */
      int selection = activateCDKRadio (radio, 0);

      /* Check the exit status of the widget. */
      if (radio->exitType == vESCAPE_HIT)
      {
	 mesg[0] = "<C>You hit escape. No item selected.";
	 mesg[1] = "";
	 mesg[2] = "<C>Press any key to continue.";
	 popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
	 break;
      }
      else if (radio->exitType == vNORMAL)
      {
	 struct stat sb;

	 if (stat (item[selection], &sb) == 0
	     && (sb.st_mode & S_IFMT) == S_IFDIR)
	 {
	    char **nitem = 0;

	    mesg[0] = "<C>You selected a directory";
	    sprintf (temp, "<C>%.*s", (int)(sizeof (temp) - 20), item[selection]);
	    mesg[1] = temp;
	    mesg[2] = "";
	    mesg[3] = "<C>Press any key to continue.";
	    popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 4);
	    count = CDKgetDirectoryContents (item[selection], &nitem);
	    if (count > 0)
	    {
	       CDKfreeStrings (item);
	       item = nitem;
	       if (chdir (item[selection]) == 0)
		  setCDKRadioItems (radio, (CDK_CSTRING2)item, count);
	    }
	 }
	 else
	 {
	    mesg[0] = "<C>You selected the filename";
	    sprintf (temp, "<C>%.*s", (int)(sizeof (temp) - 20), item[selection]);
	    mesg[1] = temp;
	    mesg[2] = "";
	    mesg[3] = "<C>Press any key to continue.";
	    popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 4);
	    break;
	 }
      }
   }

   /* Clean up. */
   CDKfreeStrings (item);
   destroyCDKRadio (radio);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

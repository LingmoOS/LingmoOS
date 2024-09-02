/* $Id: radio1_ex.c,v 1.14 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "radio_ex";
#endif

/*
 * This program demonstrates the Cdk radio widget.
 */
int main (int argc, char **argv)
{
   CDKSCREEN *cdkscreen;
   CDKRADIO *radio;
   const char *item[5] =
   {
      "Choice A",
      "Choice B",
      "Choice C"
   };
   const char *mesg[5];
   char temp[100];
   int selection;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, "s:t:" CDK_CLI_PARAMS);

   cdkscreen = initCDKScreen (NULL);

   /* Set up CDK Colors. */
   initCDKColor ();

   /* Create the radio list. */
   radio = newCDKRadio (cdkscreen,
			CDKparamValue (&params, 'X', CENTER),
			CDKparamValue (&params, 'Y', CENTER),
			CDKparsePosition (CDKparamString2 (&params, 's',
							   "NONE")),
			CDKparamValue (&params, 'H', 5),
			CDKparamValue (&params, 'W', 20),
			CDKparamString (&params, 't'),
			(CDK_CSTRING2)item, 3,
			'#' | A_REVERSE, 1,
			A_REVERSE,
			CDKparamValue (&params, 'N', TRUE),
			CDKparamValue (&params, 'S', FALSE));

   /* Check if the radio list is NULL. */
   if (radio == (CDKRADIO *)NULL)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create the radio widget. ");
      printf ("Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Activate the radio list. */
   selection = activateCDKRadio (radio, (chtype *)NULL);

   /* Check the exit status of the widget. */
   if (radio->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No item selected.";
      mesg[1] = "",
	 mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
   }
   else if (radio->exitType == vNORMAL)
   {
      mesg[0] = "<C>You selected the filename";
      sprintf (temp, "<C>%.*s", (int)(sizeof (temp) - 20), item[selection]);
      mesg[1] = temp;
      mesg[2] = "";
      mesg[3] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 4);
   }

   destroyCDKRadio (radio);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

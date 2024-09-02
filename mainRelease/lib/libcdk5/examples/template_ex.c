/* $Id: template_ex.c,v 1.13 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "template_ex";
#endif

/*
 * This program demonstrates the Cdk template widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen         = 0;
   CDKTEMPLATE *phoneNumber     = 0;
   const char *title            = "<C>Title";
   const char *label            = "</5>Phone Number:<!5>";
   const char *Overlay          = "</B/6>(___)<!6> </5>___-____";
   const char *plate            = "(###) ###-####";
   char *info;
   char *mixed;
   char temp[256];
   const char *mesg[5];

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, CDK_MIN_PARAMS);

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Declare the template. */
   phoneNumber = newCDKTemplate (cdkscreen,
				 CDKparamValue (&params, 'X', CENTER),
				 CDKparamValue (&params, 'Y', CENTER),
				 title, label,
				 plate, Overlay,
				 CDKparamValue (&params, 'N', TRUE),
				 CDKparamValue (&params, 'S', FALSE));

   /* Is the template pointer null? */
   if (phoneNumber == 0)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create template. Is the window too small?");
      ExitProgram (EXIT_FAILURE);
   }

   /* Activate the template. */
   info = activateCDKTemplate (phoneNumber, 0);

   /* Tell them what they typed. */
   if (phoneNumber->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No information typed in.";
      mesg[1] = "",
	 mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }
   else if (phoneNumber->exitType == vNORMAL)
   {
      char *with_mix;
      char *with_out;

      /* Mix the plate and the number. */
      mixed = mixCDKTemplate (phoneNumber);

      /* Create the message to display.                         */
      sprintf (temp, "Phone Number with out plate mixing  : %.*s",
	       (int)(sizeof (temp) - 50),
	       info);
      mesg[0] = with_out = copyChar (temp);
      sprintf (temp, "Phone Number with the plate mixed in: %.*s",
	       (int)(sizeof (temp) - 50),
	       mixed);
      mesg[1] = with_mix = copyChar (temp);
      mesg[2] = "";
      mesg[3] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 4);

      freeChar (with_out);
      freeChar (with_mix);
      freeChar (mixed);
   }

   /* Clean up. */
   destroyCDKTemplate (phoneNumber);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

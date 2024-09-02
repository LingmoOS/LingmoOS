/* $Id: lowerCDKObject_ex.c,v 1.10 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "lowerCDKObject_ex";
#endif

int main (int argc, char **argv)
{
   char ch;

   const char *mesg[1];
   const char *mesg1[10];
   const char *mesg2[10];

   CDKSCREEN *cdkscreen;
   CDKLABEL *label1, *label2, *instruct;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, CDK_MIN_PARAMS);

   cdkscreen = initCDKScreen (NULL);

   mesg1[0] = "label1 label1 label1 label1 label1 label1 label1";
   mesg1[1] = "label1 label1 label1 label1 label1 label1 label1";
   mesg1[2] = "label1 label1 label1 label1 label1 label1 label1";
   mesg1[3] = "label1 label1 label1 label1 label1 label1 label1";
   mesg1[4] = "label1 label1 label1 label1 label1 label1 label1";
   mesg1[5] = "label1 label1 label1 label1 label1 label1 label1";
   mesg1[6] = "label1 label1 label1 label1 label1 label1 label1";
   mesg1[7] = "label1 label1 label1 label1 label1 label1 label1";
   mesg1[8] = "label1 label1 label1 label1 label1 label1 label1";
   mesg1[9] = "label1 label1 label1 label1 label1 label1 label1";
   label1 = newCDKLabel (cdkscreen, 8, 5, (CDK_CSTRING2) mesg1, 10, TRUE, FALSE);

   mesg2[0] = "label2 label2 label2 label2 label2 label2 label2";
   mesg2[1] = "label2 label2 label2 label2 label2 label2 label2";
   mesg2[2] = "label2 label2 label2 label2 label2 label2 label2";
   mesg2[3] = "label2 label2 label2 label2 label2 label2 label2";
   mesg2[4] = "label2 label2 label2 label2 label2 label2 label2";
   mesg2[5] = "label2 label2 label2 label2 label2 label2 label2";
   mesg2[6] = "label2 label2 label2 label2 label2 label2 label2";
   mesg2[7] = "label2 label2 label2 label2 label2 label2 label2";
   mesg2[8] = "label2 label2 label2 label2 label2 label2 label2";
   mesg2[9] = "label2 label2 label2 label2 label2 label2 label2";
   label2 = newCDKLabel (cdkscreen, 14, 9, (CDK_CSTRING2) mesg2, 10, TRUE, FALSE);

   mesg[0] = "</B>1<!B> - lower </U>label1<!U>, </B>2<!B> - lower </U>label2<!U>, </B>q<!B> - </U>quit<!U>";
   instruct = newCDKLabel (cdkscreen,
			   CDKparamValue (&params, 'X', CENTER),
			   CDKparamValue (&params, 'Y', BOTTOM),
			   (CDK_CSTRING2) mesg, 1,
			   CDKparamValue (&params, 'N', FALSE),
			   CDKparamValue (&params, 'S', FALSE));

   refreshCDKScreen (cdkscreen);

   while ((ch = (char)getch ()) != 'q')
   {
      switch (ch)
      {
      case '1':
	 lowerCDKObject (vLABEL, label1);
	 break;
      case '2':
	 lowerCDKObject (vLABEL, label2);
	 break;
      default:
	 continue;
      }
      refreshCDKScreen (cdkscreen);
   }

   /* Clean up. */
   destroyCDKLabel (label1);
   destroyCDKLabel (label2);
   destroyCDKLabel (instruct);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

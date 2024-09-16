/* $Id: raiseCDKObject_ex.c,v 1.12 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "raiseCDKObject_ex";
#endif

#define MY_LABEL(p) ((chtype)ObjOf(p)->screenIndex | 0x30 | A_UNDERLINE | A_BOLD)

int main (int argc, char **argv)
{
   char ch;

   const char *mesg[1];
   const char *mesg1[10];
   const char *mesg2[10];
   const char *mesg3[10];
   const char *mesg4[10];

   CDKSCREEN *cdkscreen;
   CDKLABEL *label1, *label2, *label3, *label4, *instruct;

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
   label1 = newCDKLabel (cdkscreen, 10, 4, (CDK_CSTRING2)mesg1, 10, TRUE, FALSE);
   setCDKLabelULChar (label1, '1' | A_BOLD);

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
   label2 = newCDKLabel (cdkscreen, 8, 8, (CDK_CSTRING2)mesg2, 10, TRUE, FALSE);
   setCDKLabelULChar (label2, '2' | A_BOLD);

   mesg3[0] = "label3 label3 label3 label3 label3 label3 label3";
   mesg3[1] = "label3 label3 label3 label3 label3 label3 label3";
   mesg3[2] = "label3 label3 label3 label3 label3 label3 label3";
   mesg3[3] = "label3 label3 label3 label3 label3 label3 label3";
   mesg3[4] = "label3 label3 label3 label3 label3 label3 label3";
   mesg3[5] = "label3 label3 label3 label3 label3 label3 label3";
   mesg3[6] = "label3 label3 label3 label3 label3 label3 label3";
   mesg3[7] = "label3 label3 label3 label3 label3 label3 label3";
   mesg3[8] = "label3 label3 label3 label3 label3 label3 label3";
   mesg3[9] = "label3 label3 label3 label3 label3 label3 label3";
   label3 = newCDKLabel (cdkscreen, 6, 12, (CDK_CSTRING2)mesg3, 10, TRUE, FALSE);
   setCDKLabelULChar (label3, '3' | A_BOLD);

   mesg4[0] = "label4 label4 label4 label4 label4 label4 label4";
   mesg4[1] = "label4 label4 label4 label4 label4 label4 label4";
   mesg4[2] = "label4 label4 label4 label4 label4 label4 label4";
   mesg4[3] = "label4 label4 label4 label4 label4 label4 label4";
   mesg4[4] = "label4 label4 label4 label4 label4 label4 label4";
   mesg4[5] = "label4 label4 label4 label4 label4 label4 label4";
   mesg4[6] = "label4 label4 label4 label4 label4 label4 label4";
   mesg4[7] = "label4 label4 label4 label4 label4 label4 label4";
   mesg4[8] = "label4 label4 label4 label4 label4 label4 label4";
   mesg4[9] = "label4 label4 label4 label4 label4 label4 label4";
   label4 = newCDKLabel (cdkscreen, 4, 16, (CDK_CSTRING2)mesg4, 10, TRUE, FALSE);
   setCDKLabelULChar (label4, '4' | A_BOLD);

   mesg[0] = "</B>#<!B> - raise </U>label#<!U>, </B>r<!B> - </U>redraw<!U>, </B>q<!B> - </U>quit<!U>";
   instruct = newCDKLabel (cdkscreen,
			   CDKparamValue (&params, 'X', CENTER),
			   CDKparamValue (&params, 'Y', BOTTOM),
			   (CDK_CSTRING2)mesg, 1,
			   CDKparamValue (&params, 'N', TRUE),
			   CDKparamValue (&params, 'S', FALSE));
   setCDKLabelULChar (instruct, ' ' | A_NORMAL);
   setCDKLabelURChar (instruct, ' ' | A_NORMAL);
   setCDKLabelLLChar (instruct, ' ' | A_NORMAL);
   setCDKLabelVerticalChar (instruct, ' ' | A_NORMAL);
   setCDKLabelHorizontalChar (instruct, ' ' | A_NORMAL);

   setCDKLabelLRChar (label1, MY_LABEL (label1));
   setCDKLabelLRChar (label2, MY_LABEL (label2));
   setCDKLabelLRChar (label3, MY_LABEL (label3));
   setCDKLabelLRChar (label4, MY_LABEL (label4));
   setCDKLabelLRChar (instruct, MY_LABEL (instruct));
   refreshCDKScreen (cdkscreen);

   while ((ch = (char)getch ()) != 'q')
   {
      switch (ch)
      {
      case '1':
	 raiseCDKObject (vLABEL, label1);
	 break;
      case '2':
	 raiseCDKObject (vLABEL, label2);
	 break;
      case '3':
	 raiseCDKObject (vLABEL, label3);
	 break;
      case '4':
	 raiseCDKObject (vLABEL, label4);
	 break;
      case 'r':
	 refreshCDKScreen (cdkscreen);
	 break;
      default:
	 continue;
      }

      setCDKLabelLRChar (label1, MY_LABEL (label1));
      setCDKLabelLRChar (label2, MY_LABEL (label2));
      setCDKLabelLRChar (label3, MY_LABEL (label3));
      setCDKLabelLRChar (label4, MY_LABEL (label4));
      setCDKLabelLRChar (instruct, MY_LABEL (instruct));
      refreshCDKScreen (cdkscreen);
   }

   /* Clean up. */
   destroyCDKLabel (label1);
   destroyCDKLabel (label2);
   destroyCDKLabel (label3);
   destroyCDKLabel (label4);
   destroyCDKLabel (instruct);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

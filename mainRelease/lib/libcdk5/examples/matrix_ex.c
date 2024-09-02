/* $Id: matrix_ex.c,v 1.19 2016/12/10 13:29:08 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "matrix_ex";
#endif

#define MY_COLS 10

/*
 * This program demonstrates the Cdk matrix widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKMATRIX *courseList = 0;
   const char *title    = 0;
   int rows             = 8;
   int cols             = 5;
   int vrows            = 3;
   int vcols            = 5;

   bool use_coltitles;
   bool use_rowtitles;

   const char *coltitle[MY_COLS];
   const char *rowtitle[MY_COLS];
   const char *mesg[MY_COLS];

   int colwidth[MY_COLS];
   int colvalue[MY_COLS];

   int col_spacing;
   int row_spacing;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, "trcT:C:R:" CDK_MIN_PARAMS);

   /* invert, so giving -S causes the shadow to turn off */
   params.Shadow = !params.Shadow;

   /* cancel the default title, or supply a new one */
   if (CDKparamValue (&params, 't', FALSE))
   {
      title = 0;
   }
   else if ((title = CDKparamString (&params, 'T')) == 0)
   {
      title = "<C>This is the CDK\n<C>matrix widget.\n<C><#LT><#HL(30)><#RT>";
   }

   /* allow cancelling of column and/or row titles with -c and/or -r */
   use_coltitles = !CDKparamValue (&params, 'c', FALSE);
   use_rowtitles = !CDKparamValue (&params, 'r', FALSE);
   col_spacing = CDKparamNumber2 (&params, 'C', -1);
   row_spacing = CDKparamNumber2 (&params, 'R', -1);

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Create the horizontal and vertical matrix labels. */
#define set_col(n, width, string) \
   coltitle[n] = use_coltitles ? string : 0 ;\
   colwidth[n] = width ;\
   colvalue[n] = vUMIXED

   set_col (1, 7, "</B/5>Course");
   set_col (2, 7, "</B/33>Lec 1");
   set_col (3, 7, "</B/33>Lec 2");
   set_col (4, 7, "</B/33>Lec 3");
   set_col (5, 1, "</B/7>Flag");

#define set_row(n, string) \
   rowtitle[n] = use_rowtitles ? "<C></B/6>" string : 0

   set_row (1, "Course 1");
   set_row (2, "Course 2");
   set_row (3, "Course 3");
   set_row (4, "Course 4");
   set_row (5, "Course 5");
   set_row (6, "Course 6");
   set_row (7, "Course 7");
   set_row (8, "Course 8");

   /* Create the matrix object. */
   courseList = newCDKMatrix (cdkscreen,
			      CDKparamValue (&params, 'X', CENTER),
			      CDKparamValue (&params, 'Y', CENTER),
			      rows, cols, vrows, vcols,
			      title,
			      (CDK_CSTRING2)rowtitle,
			      (CDK_CSTRING2)coltitle,
			      colwidth, colvalue,
			      col_spacing, row_spacing, '.',
			      COL, params.Box,
			      params.Box,
			      params.Shadow);

   /* Check to see if the matrix is null. */
   if (courseList == 0)
   {
      /* Clean up. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create the matrix widget.\n");
      printf ("Is the window too small ?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Activate the matrix. */
   activateCDKMatrix (courseList, 0);

   /* Check if the user hit escape or not. */
   if (courseList->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No information passed back.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
   }
   else if (courseList->exitType == vNORMAL)
   {
      char temp[80];

      sprintf (temp, "Current cell (%d,%d)", courseList->crow, courseList->ccol);
      mesg[0] = "<L>You exited the matrix normally.";
      mesg[1] = temp;
      mesg[2] = "<L>To get the contents of the matrix cell, you can";
      mesg[3] = "<L>use getCDKMatrixCell():";
      mesg[4] = getCDKMatrixCell (courseList, courseList->crow, courseList->ccol);
      mesg[5] = "";
      mesg[6] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 7);
   }

   /* Clean up. */
   destroyCDKMatrix (courseList);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

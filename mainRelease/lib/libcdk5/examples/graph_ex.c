/* $Id: graph_ex.c,v 1.15 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "graph_ex";
#endif

int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen         = 0;
   CDKGRAPH *graph              = 0;
   CDKLABEL *pausep             = 0;
   const char *title            = 0;
   const char *xtitle           = 0;
   const char *ytitle           = 0;
   const char *graphChars       = 0;
   const char *mesg[2];
   int values[20];
   int count;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, CDK_CLI_PARAMS);	/* -N, -S unused */

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Create the graph values. */
   /* *INDENT-EQLS* */
   values[0]    = 10;
   values[1]    = 15;
   values[2]    = 20;
   values[3]    = 25;
   values[4]    = 30;
   values[5]    = 35;
   values[6]    = 40;
   values[7]    = 45;
   values[8]    = 50;
   values[9]    = 55;
   count	= 10;
   title	= "<C>Test Graph";
   xtitle	= "<C>X AXIS TITLE";
   ytitle	= "<C>Y AXIS TITLE";
   graphChars	= "0123456789";

   /* Create the label values. */
   mesg[0] = "Press any key when done viewing the graph.";

   /* Create the graph widget. */
   graph = newCDKGraph (cdkscreen,
			CDKparamValue (&params, 'X', CENTER),
			CDKparamValue (&params, 'Y', CENTER),
			CDKparamValue (&params, 'H', 10),
			CDKparamValue (&params, 'W', 20),
			title, xtitle, ytitle);

   /* Is the graph null? */
   if (graph == 0)
   {
      /* Shut down CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot make the graph widget. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Create the label widget. */
   pausep = newCDKLabel (cdkscreen, CENTER, BOTTOM,
			 (CDK_CSTRING2) mesg, 1,
			 TRUE, FALSE);
   if (pausep == 0)
   {
      /* Shut down CDK. */
      destroyCDKGraph (graph);
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot make the label widget. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Set the graph values. */
   setCDKGraph (graph, values, count, graphChars, FALSE, vPLOT);

   /* Draw the screen. */
   refreshCDKScreen (cdkscreen);
   drawCDKGraph (graph, FALSE);
   drawCDKLabel (pausep, TRUE);

   /* Pause until the user says so... */
   waitCDKLabel (pausep, 0);

   /* Clean up. */
   destroyCDKGraph (graph);
   destroyCDKLabel (pausep);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

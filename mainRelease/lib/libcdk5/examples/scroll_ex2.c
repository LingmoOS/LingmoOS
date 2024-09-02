#include <cdk_test.h>

#define NumElements(a) ((sizeof a)/(sizeof a[0]))

int main (int argc, char *argv[])
{
   CDKSCREEN *cdkscreen;
   CDKSCROLL *scrollList;
   CDK_CONST char *vegList[] =
   {"Artichoke", "Bean", "Cabbage", "Onion"};
   CDK_CONST char *furnList[] =
   {"Bed", "Chair", "Table"};
   const char *title = "<C></5>Replace scroll-items";

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, CDK_CLI_PARAMS);

   cdkscreen = initCDKScreen (NULL);

   initCDKColor ();
   scrollList = newCDKScroll (cdkscreen,
			      CDKparamValue (&params, 'X', CENTER),
			      CDKparamValue (&params, 'Y', CENTER),
			      CDKparsePosition (CDKparamString2 (&params,
								 's',
								 "RIGHT")),
			      CDKparamValue (&params, 'H', 8),
			      CDKparamValue (&params, 'W', 30),
			      CDKparamString2 (&params, 't', title),
			      (CDK_CSTRING2)vegList,
			      NumElements (vegList),
			      FALSE,
			      A_REVERSE,
			      CDKparamValue (&params, 'N', TRUE),
			      CDKparamValue (&params, 'S', FALSE));

   refreshCDKScreen (cdkscreen);
   napms (3000);
   eraseCDKScroll (scrollList);

   setCDKScrollItems (scrollList,
		      (CDK_CSTRING2)furnList,
		      NumElements (furnList),
		      FALSE);
   refreshCDKScreen (cdkscreen);

   while (scrollList->exitType != vNORMAL)
   {
      activateCDKScroll (scrollList, 0);
   }

   destroyCDKScroll (scrollList);

   destroyCDKScreen (cdkscreen);

   endCDK ();

   ExitProgram (EXIT_SUCCESS);
}

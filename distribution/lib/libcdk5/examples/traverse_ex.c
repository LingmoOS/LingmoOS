/* $Id: traverse_ex.c,v 1.26 2017/12/09 19:03:36 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "entry_ex";
#endif

#define NumElements(a) ((sizeof a)/(sizeof a[0]))

#define MY_MAX 3

static CDKOBJS *all_objects[MY_MAX];

static const char *yes_no[] =
{
   "Yes", "NO"
};
static const char *months[] =
{
   "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char *choices[] =
{
   "[ ]", "[*]"
};

/*
 * Exercise all widgets except
 *	CDKMENU
 * 	CDKTRAVERSE
 * The names in parentheses do not accept input, so they will never have
 * focus for traversal.  The names with leading "*" have some limitation
 * that makes them not useful in traversal.
 */
/* *INDENT-OFF* */
static const struct
{
   const char *name;
   EObjectType type;
}
menu_table[] =
{
   { "(CDKGRAPH)",	vGRAPH },	/* no traversal (not active) */
   { "(CDKHISTOGRAM)",	vHISTOGRAM },	/* no traversal (not active) */
   { "(CDKLABEL)",	vLABEL },	/* no traversal (not active) */
   { "(CDKMARQUEE)",	vMARQUEE },	/* hangs (leaves trash) */
   { "*CDKVIEWER",	vVIEWER },	/* traversal out-only on OK */
   { "CDKALPHALIST",	vALPHALIST },
   { "CDKBUTTON",	vBUTTON },
   { "CDKBUTTONBOX",	vBUTTONBOX },
   { "CDKCALENDAR",	vCALENDAR },
   { "CDKDIALOG",	vDIALOG },
   { "CDKDSCALE",	vDSCALE },
   { "CDKENTRY",	vENTRY },
   { "CDKFSCALE",	vFSCALE },
   { "CDKFSELECT",	vFSELECT },
   { "CDKFSLIDER",	vFSLIDER },
   { "CDKITEMLIST",	vITEMLIST },
   { "CDKMATRIX",	vMATRIX },
   { "CDKMENTRY",	vMENTRY },
   { "CDKRADIO",	vRADIO },
   { "CDKSCALE",	vSCALE },
   { "CDKSCROLL",	vSCROLL },
   { "CDKSELECTION",	vSELECTION },
   { "CDKSLIDER",	vSLIDER },
   { "CDKSWINDOW",	vSWINDOW },
   { "CDKTEMPLATE",	vTEMPLATE },
   { "CDKUSCALE",	vUSCALE },
   { "CDKUSLIDER",	vUSLIDER },
};
/* *INDENT-ON* */

static CDKOBJS *make_alphalist (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKALPHALIST *widget = newCDKAlphalist (cdkscreen,
					   x, y, 10, 15,
					   "AlphaList",
					   "->",
					   (CDK_CSTRING *)months,
					   NumElements (months),
					   '_', A_REVERSE,
					   TRUE, FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_button (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKBUTTON *widget = newCDKButton (cdkscreen, x, y,
				     "A Button!", NULL,
				     TRUE, FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_buttonbox (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKBUTTONBOX *widget = newCDKButtonbox (cdkscreen, x, y,
					   10, 16,
					   "ButtonBox", 6, 2,
					   (CDK_CSTRING2)months,
					   NumElements (months),
					   A_REVERSE,
					   TRUE, FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_calendar (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKCALENDAR *widget = newCDKCalendar (cdkscreen, x, y,
					 "Calendar", 25, 1, 2000,
					 COLOR_PAIR (16) | A_BOLD,
					 COLOR_PAIR (24) | A_BOLD,
					 COLOR_PAIR (32) | A_BOLD,
					 COLOR_PAIR (40) | A_REVERSE,
					 TRUE,
					 FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_dialog (CDKSCREEN *cdkscreen, int x, int y)
{
   static const char *message[] =
   {
      "This is a simple dialog box"
      ,"Is it simple enough?"
   };

   CDKDIALOG *widget = newCDKDialog (cdkscreen, x, y,
				     (CDK_CSTRING2)message, NumElements (message),
				     (CDK_CSTRING2)yes_no, NumElements (yes_no),
				     COLOR_PAIR (2) | A_REVERSE,
				     TRUE,
				     TRUE, FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_dscale (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKDSCALE *widget = newCDKDScale (cdkscreen, x, y,
				     "DScale", "Value", A_NORMAL,
				     15,
				     0.0, 0.0, 100.0,
				     1.0, (1.0 * 2.), 1,
				     TRUE,
				     FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_entry (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKENTRY *widget = newCDKEntry (cdkscreen, x, y,
				   NULL, "Entry: ", A_NORMAL,
				   '.', vMIXED, 40, 0, 256, TRUE, FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_fscale (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKFSCALE *widget = newCDKFScale (cdkscreen, x, y,
				     "FScale", "Value", A_NORMAL,
				     15,
				     0.0, 0.0, 100.0,
				     1.0, (1.0 * 2.), 1,
				     TRUE,
				     FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_fslider (CDKSCREEN *cdkscreen, int x, int y)
{
   float low = -32;
   float high = 64;
   float inc = (float)0.1;
   CDKFSLIDER *widget = newCDKFSlider (cdkscreen,
				       x,
				       y,
				       "FSlider", "Label",
				       A_REVERSE | COLOR_PAIR (29) | ' ',
				       20,
				       low, low, high, inc, (inc * 2),
				       3,
				       TRUE,
				       FALSE);
   /* selection = activateCDKSlider (widget, 0); */
   return ObjPtr (widget);
}

static CDKOBJS *make_fselect (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKFSELECT *widget = newCDKFselect (cdkscreen,
				       x,
				       y,
				       15,
				       25,
				       "FSelect", "->", A_NORMAL, '_', A_REVERSE,
				       "</5>", "</48>", "</N>", "</N>",
				       TRUE,
				       FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_graph (CDKSCREEN *cdkscreen, int x, int y)
{
   static int values[] =
   {
      10, 15, 20, 25, 30, 35, 40, 45, 50, 55
   };
   static const char *graphChars = "0123456789";
   CDKGRAPH *widget = newCDKGraph (cdkscreen, x, y,
				   10, 25,
				   "title", "X-axis", "Y-axis");
   setCDKGraph (widget, values, NumElements (values), graphChars, TRUE, vPLOT);
   return ObjPtr (widget);
}

static CDKOBJS *make_histogram (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKHISTOGRAM *widget = newCDKHistogram (cdkscreen,
					   x,
					   y,
					   1,
					   20,
					   HORIZONTAL, "Histogram",
					   TRUE,
					   FALSE);
   setCDKHistogram (widget, vPERCENT, CENTER, A_BOLD, 0, 10, 6, ' ' |
		    A_REVERSE, TRUE);
   return ObjPtr (widget);
}

static CDKOBJS *make_itemlist (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKITEMLIST *widget = newCDKItemlist (cdkscreen, x, y,
					 NULL, "Month ",
					 (CDK_CSTRING2)months,
					 NumElements (months),
					 1, TRUE, FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_label (CDKSCREEN *cdkscreen, int x, int y)
{
   static const char *message[] =
   {
      "This is a simple label."
      ,"Is it simple enough?"
   };
   CDKLABEL *widget = newCDKLabel (cdkscreen,
				   x,
				   y,
				   (CDK_CSTRING2)message, NumElements (message),
				   TRUE,
				   TRUE);
   return ObjPtr (widget);
}

static CDKOBJS *make_marquee (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKMARQUEE *widget = newCDKMarquee (cdkscreen,
				       x,
				       y,
				       30,
				       TRUE,
				       TRUE);
   activateCDKMarquee (widget, "This is a message", 5, 3, TRUE);
   destroyCDKMarquee (widget);
   return 0;
}

static CDKOBJS *make_matrix (CDKSCREEN *cdkscreen, int x, int y)
{
#define NUMROWS 8
#define NUMCOLS 5

   CDKMATRIX *widget;
   const char *coltitle[NUMCOLS + 1];
   const char *rowtitle[NUMROWS + 1];
   char temp[80];
   int cols = NUMCOLS;
   int colwidth[NUMCOLS + 1];
   int coltypes[NUMCOLS + 1];
   int maxwidth = 0;
   int n;
   int rows = NUMROWS;
   int vcols = 3;
   int vrows = 3;

   for (n = 0; n <= NUMROWS; ++n)
   {
      sprintf (temp, "row%d", n);
      rowtitle[n] = copyChar (temp);
   }
   for (n = 0; n <= NUMCOLS; ++n)
   {
      sprintf (temp, "col%d", n);
      coltitle[n] = copyChar (temp);
      colwidth[n] = (int)strlen (temp);
      coltypes[n] = vUCHAR;
      if (colwidth[n] > maxwidth)
	 maxwidth = colwidth[n];
   }

   widget = newCDKMatrix (cdkscreen,
			  x,
			  y,
			  rows, cols, vrows, vcols,
			  "Matrix", (CDK_CSTRING2)rowtitle, (CDK_CSTRING2)coltitle,
			  colwidth, coltypes,
			  -1, -1, '.',
			  COL, TRUE,
			  TRUE,
			  FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_mentry (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKMENTRY *widget = newCDKMentry (cdkscreen,
				     x,
				     y,
				     "MEntry", "Label", A_BOLD, '.', vMIXED,
				     20,
				     5,
				     20,
				     0,
				     TRUE,
				     FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_radio (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKRADIO *widget = newCDKRadio (cdkscreen,
				   x,
				   y,
				   RIGHT,
				   10,
				   20,
				   "Radio",
				   (CDK_CSTRING2)months, NumElements (months),
				   '#' | A_REVERSE, 1,
				   A_REVERSE,
				   TRUE,
				   FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_scale (CDKSCREEN *cdkscreen, int x, int y)
{
   int low = 2;
   int high = 25;
   int inc = 1;
   CDKSCALE *widget = newCDKScale (cdkscreen,
				   x,
				   y,
				   "Scale", "Label", A_NORMAL,
				   5,
				   low, low, high,
				   inc, (inc * 2),
				   TRUE,
				   FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_scroll (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKSCROLL *widget = newCDKScroll (cdkscreen,
				     x,
				     y,
				     RIGHT,
				     10,
				     20,
				     "Scroll",
				     (CDK_CSTRING2)months, NumElements (months),
				     TRUE,
				     A_REVERSE,
				     TRUE,
				     FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_slider (CDKSCREEN *cdkscreen, int x, int y)
{
   int low = 2;
   int high = 25;
   int inc = 1;
   CDKSLIDER *widget = newCDKSlider (cdkscreen,
				     x,
				     y,
				     "Slider", "Label",
				     A_REVERSE | COLOR_PAIR (29) | ' ',
				     20,
				     low, low, high, inc, (inc * 2),
				     TRUE,
				     FALSE);
   /* selection = activateCDKSlider (widget, 0); */
   return ObjPtr (widget);
}

static CDKOBJS *make_selection (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKSELECTION *widget = newCDKSelection (cdkscreen, x, y,
					   NONE, 8, 20, "Selection",
					   (CDK_CSTRING2)months,
					   NumElements (months),
					   (CDK_CSTRING2)choices,
					   NumElements (choices),
					   A_REVERSE, TRUE, FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_swindow (CDKSCREEN *cdkscreen, int x, int y)
{
   CDKSWINDOW *widget = newCDKSwindow (cdkscreen,
				       x,
				       y,
				       6,
				       25,
				       "SWindow", 100,
				       TRUE,
				       FALSE);
   int n;
   for (n = 0; n < 30; ++n)
   {
      char temp[80];
      sprintf (temp, "Line %d", n);
      addCDKSwindow (widget, temp, BOTTOM);
   }
   activateCDKSwindow (widget, 0);
   return ObjPtr (widget);
}

static CDKOBJS *make_template (CDKSCREEN *cdkscreen, int x, int y)
{
   const char *Overlay = "</B/6>(___)<!6> </5>___-____";
   const char *plate = "(###) ###-####";
   CDKTEMPLATE *widget = newCDKTemplate (cdkscreen,
					 x,
					 y,
					 "Template", "Label",
					 plate, Overlay,
					 TRUE,
					 FALSE);
   activateCDKTemplate (widget, 0);
   return ObjPtr (widget);
}

static CDKOBJS *make_uscale (CDKSCREEN *cdkscreen, int x, int y)
{
   unsigned low = 0;
   unsigned high = 65535;
   unsigned inc = 1;
   CDKUSCALE *widget = newCDKUScale (cdkscreen,
				     x,
				     y,
				     "UScale", "Label", A_NORMAL,
				     5,
				     low, low, high,
				     inc, (inc * 32),
				     TRUE,
				     FALSE);
   return ObjPtr (widget);
}

static CDKOBJS *make_uslider (CDKSCREEN *cdkscreen, int x, int y)
{
   unsigned low = 0;
   unsigned high = 65535;
   unsigned inc = 1;
   CDKUSLIDER *widget = newCDKUSlider (cdkscreen,
				       x,
				       y,
				       "USlider", "Label",
				       A_REVERSE | COLOR_PAIR (29) | ' ',
				       20,
				       low, low, high, inc, (inc * 32),
				       TRUE,
				       FALSE);
   /* selection = activateCDKSlider (widget, 0); */
   return ObjPtr (widget);
}

static CDKOBJS *make_viewer (CDKSCREEN *cdkscreen, int x, int y)
{
   static const char *button[1] =
   {
      "Ok"
   };
   CDKVIEWER *widget = newCDKViewer (cdkscreen,
				     x,
				     y,
				     10,
				     20,
				     (CDK_CSTRING2)button, 1, A_REVERSE,
				     TRUE,
				     FALSE);
   setCDKViewer (widget, "Viewer",
		 (CDK_CSTRING2)months, NumElements (months),
		 A_REVERSE, FALSE, TRUE, TRUE);
   activateCDKViewer (widget, 0);
   return ObjPtr (widget);
}

static void rebind_esc (CDKOBJS *obj)
{
   bindCDKObject (ObjTypeOf (obj), obj, KEY_F (1), getcCDKBind, (void *)KEY_ESC);
}

static void make_any (CDKSCREEN *cdkscreen, int menu, EObjectType type)
{
   CDKOBJS *(*func) (CDKSCREEN *, int, int) = 0;
   CDKOBJS *prior = 0;
   int x;
   int y;

   /* setup positions, staggered a little */
   switch (menu)
   {
   case 0:
      x = LEFT;
      y = 2;
      break;
   case 1:
      x = CENTER;
      y = 4;
      break;
   case 2:
      x = RIGHT;
      y = 6;
      break;
   default:
      beep ();
      return;
   }

   /* find the function to make a widget of the given type */
   switch (type)
   {
   case vALPHALIST:
      func = make_alphalist;
      break;
   case vBUTTON:
      func = make_button;
      break;
   case vBUTTONBOX:
      func = make_buttonbox;
      break;
   case vCALENDAR:
      func = make_calendar;
      break;
   case vDIALOG:
      func = make_dialog;
      break;
   case vDSCALE:
      func = make_dscale;
      break;
   case vENTRY:
      func = make_entry;
      break;
   case vFSCALE:
      func = make_fscale;
      break;
   case vFSELECT:
      func = make_fselect;
      break;
   case vFSLIDER:
      func = make_fslider;
      break;
   case vGRAPH:
      func = make_graph;
      break;
   case vHISTOGRAM:
      func = make_histogram;
      break;
   case vITEMLIST:
      func = make_itemlist;
      break;
   case vLABEL:
      func = make_label;
      break;
   case vMARQUEE:
      func = make_marquee;
      break;
   case vMATRIX:
      func = make_matrix;
      break;
   case vMENTRY:
      func = make_mentry;
      break;
   case vRADIO:
      func = make_radio;
      break;
   case vSCALE:
      func = make_scale;
      break;
   case vSCROLL:
      func = make_scroll;
      break;
   case vSELECTION:
      func = make_selection;
      break;
   case vSLIDER:
      func = make_slider;
      break;
   case vSWINDOW:
      func = make_swindow;
      break;
   case vTEMPLATE:
      func = make_template;
      break;
   case vUSCALE:
      func = make_uscale;
      break;
   case vUSLIDER:
      func = make_uslider;
      break;
   case vVIEWER:
      func = make_viewer;
      break;
   case vMENU:
   case vTRAVERSE:
   case vNULL:
      if (type > 0)
	 beep ();
      return;
   }

   /* erase the old widget */
   if ((prior = all_objects[menu]) != 0)
   {
      EraseObj (prior);
      _destroyCDKObject (prior);
      all_objects[menu] = 0;
   }

   /* create the new widget */
   if (func != 0)
   {
      CDKOBJS *widget = func (cdkscreen, x, y);
      if (widget != 0)
      {
	 all_objects[menu] = widget;
	 rebind_esc (widget);
      }
      else
	 flash ();
   }
   else
   {
      beep ();
   }
}

/*
 * Whenever we get a menu selection, create the selected widget.
 */
static int preHandler (EObjectType cdktype GCC_UNUSED, void *object,
		       void *clientData GCC_UNUSED,
		       chtype input GCC_UNUSED)
{
   int mp, sp;
   CDKSCREEN *screen;
   WINDOW *window;

   switch (input)
   {
   case KEY_ENTER:
      getCDKMenuCurrentItem ((CDKMENU *)object, &mp, &sp);

      screen = ScreenOf ((CDKMENU *)object);
      window = screen->window;
      mvwprintw (window, getmaxy (window) - 1, 0, "selection %d/%d", mp, sp);
      clrtoeol ();
      refresh ();
      if (sp >= 0 && sp < (int)NumElements (menu_table))
	 make_any (screen, mp, menu_table[sp].type);
      break;
   }
   return 1;
}

/*
 * This demonstrates the Cdk widget-traversal.
 */
int main (int argc GCC_UNUSED, char **argv GCC_UNUSED)
{
   unsigned j, k;

   /* Declare local variables. */
   CDKSCREEN *cdkscreen = NULL;
   CDKMENU *menu = NULL;
   const char *mesg[3];
   static const char *menulist[MAX_MENU_ITEMS][MAX_SUB_ITEMS] =
   {
      {"Left"},
      {"Center"},
      {"Right"},
   };
   static int submenusize[] =
   {
      NumElements (menu_table) + 1,
      NumElements (menu_table) + 1,
      NumElements (menu_table) + 1
   };
   static int menuloc[] =
   {LEFT, LEFT, RIGHT};

   for (j = 0; j < MY_MAX; ++j)
   {
      for (k = 0; k < NumElements (menu_table); ++k)
      {
	 menulist[j][k + 1] = menu_table[k].name;
      }
   }

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK colors. */
   initCDKColor ();


   menu = newCDKMenu (cdkscreen, menulist, MY_MAX, submenusize, menuloc,
		      TOP, A_UNDERLINE, A_REVERSE);
   if (menu == 0)
   {
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("? Cannot create menus\n");
      ExitProgram (EXIT_FAILURE);
   }
   rebind_esc (ObjOf (menu));

   setCDKMenuPreProcess (menu, preHandler, 0);

   /* setup the initial display */
   make_any (cdkscreen, 0, vENTRY);
#if MY_MAX > 1
   make_any (cdkscreen, 1, vITEMLIST);
#if MY_MAX > 2
   make_any (cdkscreen, 2, vSELECTION);
#endif
#endif

   /* Draw the screen. */
   refreshCDKScreen (cdkscreen);

   /* Traverse the screen */
   traverseCDKScreen (cdkscreen);

   mesg[0] = "Done";
   mesg[1] = "";
   mesg[2] = "<C>Press any key to continue.";
   popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);

   /* Clean up and exit. */
   for (j = 0; j < MY_MAX; ++j)
   {
      if (all_objects[j] != 0)
	 _destroyCDKObject (all_objects[j]);
   }
   destroyCDKMenu (menu);
   destroyCDKScreen (cdkscreen);
   endCDK ();

   ExitProgram (EXIT_SUCCESS);
}

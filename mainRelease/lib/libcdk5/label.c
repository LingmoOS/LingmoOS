#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2014/11/06 10:06:46 $
 * $Revision: 1.91 $
 */

DeclareCDKObjects (LABEL, Label, setCdk, Unknown);

/*
 * This creates a label widget.
 */
CDKLABEL *newCDKLabel (CDKSCREEN *cdkscreen,
		       int xplace,
		       int yplace,
		       CDK_CSTRING2 mesg,
		       int rows,
		       boolean Box,
		       boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKLABEL *label      = 0;
   int parentWidth      = getmaxx (cdkscreen->window);
   int parentHeight     = getmaxy (cdkscreen->window);
   int boxWidth         = INT_MIN;
   int boxHeight;
   int xpos             = xplace;
   int ypos             = yplace;
   int x                = 0;

   if (rows <= 0
       || (label = newCDKObject (CDKLABEL, &my_funcs)) == 0
       || (label->info = typeCallocN (chtype *, rows + 1)) == 0
       || (label->infoLen = typeCallocN (int, rows + 1)) == 0
       || (label->infoPos = typeCallocN (int, rows + 1)) == 0)
   {
      destroyCDKObject (label);
      return (0);
   }

   setCDKLabelBox (label, Box);
   boxHeight = rows + 2 * BorderOf (label);

   /* Determine the box width. */
   for (x = 0; x < rows; x++)
   {
      /* Translate the char * to a chtype. */
      label->info[x] = char2Chtype (mesg[x],
				    &label->infoLen[x],
				    &label->infoPos[x]);
      boxWidth = MAXIMUM (boxWidth, label->infoLen[x]);
   }
   boxWidth += 2 * BorderOf (label);

   /* Create the string alignments. */
   for (x = 0; x < rows; x++)
   {
      label->infoPos[x] = justifyString (boxWidth - 2 * BorderOf (label),
					 label->infoLen[x],
					 label->infoPos[x]);
   }

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   boxWidth = (boxWidth > parentWidth ? parentWidth : boxWidth);
   boxHeight = (boxHeight > parentHeight ? parentHeight : boxHeight);

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* *INDENT-EQLS* Create the label. */
   ScreenOf (label)     = cdkscreen;
   label->parent        = cdkscreen->window;
   label->win           = newwin (boxHeight, boxWidth, ypos, xpos);
   label->shadowWin     = 0;
   label->xpos          = xpos;
   label->ypos          = ypos;
   label->rows          = rows;
   label->boxWidth      = boxWidth;
   label->boxHeight     = boxHeight;
   ObjOf (label)->inputWindow = label->win;
   ObjOf (label)->hasFocus = FALSE;
   label->shadow        = shadow;

   /* Is the window null? */
   if (label->win == 0)
   {
      destroyCDKObject (label);
      return (0);
   }
   keypad (label->win, TRUE);

   /* If a shadow was requested, then create the shadow window. */
   if (shadow)
   {
      label->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Register this baby. */
   registerCDKObject (cdkscreen, vLABEL, label);

   /* Return the label pointer. */
   return (label);
}

/*
 * This was added for the builder.
 */
void activateCDKLabel (CDKLABEL *label, chtype *actions GCC_UNUSED)
{
   drawCDKLabel (label, ObjOf (label)->box);
}

/*
 * This sets multiple attributes of the widget.
 */
void setCDKLabel (CDKLABEL *label, CDK_CSTRING2 mesg, int lines, boolean Box)
{
   setCDKLabelMessage (label, mesg, lines);
   setCDKLabelBox (label, Box);
}

/*
 * This sets the information within the label.
 */
void setCDKLabelMessage (CDKLABEL *label, CDK_CSTRING2 info, int infoSize)
{
   int x;
   int limit;

   /* Clean out the old message. */
   for (x = 0; x < label->rows; x++)
   {
      freeChtype (label->info[x]);
      label->info[x] = 0;
      label->infoPos[x] = 0;
      label->infoLen[x] = 0;
   }

   /* update the label's length - but taking into account its window size */
   limit = label->boxHeight - (2 * BorderOf (label));
   if (infoSize > limit)
      infoSize = limit;
   label->rows = infoSize;

   /* Copy in the new message. */
   for (x = 0; x < label->rows; x++)
   {
      label->info[x] = char2Chtype (info[x],
				    &label->infoLen[x],
				    &label->infoPos[x]);
      label->infoPos[x] = justifyString (label->boxWidth - 2 * BorderOf (label),
					 label->infoLen[x],
					 label->infoPos[x]);
   }

   /* Redraw the label widget. */
   eraseCDKLabel (label);
   drawCDKLabel (label, ObjOf (label)->box);
}
chtype **getCDKLabelMessage (CDKLABEL *label, int *size)
{
   (*size) = label->rows;
   return label->info;
}

/*
 * This sets the box flag for the label widget.
 */
void setCDKLabelBox (CDKLABEL *label, boolean Box)
{
   ObjOf (label)->box = Box;
   ObjOf (label)->borderSize = Box ? 1 : 0;
}
boolean getCDKLabelBox (CDKLABEL *label)
{
   return ObjOf (label)->box;
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrLabel (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKLABEL *widget = (CDKLABEL *)object;

      wbkgd (widget->win, attrib);
   }
}

/*
 * This draws the label widget.
 */
static void _drawCDKLabel (CDKOBJS *object, boolean Box GCC_UNUSED)
{
   CDKLABEL *label = (CDKLABEL *)object;
   int x = 0;

   /* Is there a shadow? */
   if (label->shadowWin != 0)
   {
      drawShadow (label->shadowWin);
   }

   /* Box the widget if asked. */
   if (ObjOf (label)->box)
   {
      drawObjBox (label->win, ObjOf (label));
   }

   /* Draw in the message. */
   for (x = 0; x < label->rows; x++)
   {
      writeChtype (label->win,
		   label->infoPos[x] + BorderOf (label),
		   x + BorderOf (label),
		   label->info[x],
		   HORIZONTAL,
		   0,
		   label->infoLen[x]);
   }

   /* Refresh the window. */
   wrefresh (label->win);
}

/*
 * This erases the label widget.
 */
static void _eraseCDKLabel (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKLABEL *label = (CDKLABEL *)object;

      eraseCursesWindow (label->win);
      eraseCursesWindow (label->shadowWin);
   }
}

/*
 * This moves the label field to the given location.
 */
static void _moveCDKLabel (CDKOBJS *object,
			   int xplace,
			   int yplace,
			   boolean relative,
			   boolean refresh_flag)
{
   CDKLABEL *label = (CDKLABEL *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (label->win);
   int currentY = getbegy (label->win);
   int xpos     = xplace;
   int ypos     = yplace;
   int xdiff    = 0;
   int ydiff    = 0;

   /*
    * If this is a relative move, then we will adjust where we want
    * to move to.
    */
   if (relative)
   {
      xpos = getbegx (label->win) + xplace;
      ypos = getbegy (label->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (label), &xpos, &ypos, label->boxWidth, label->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (label->win, -xdiff, -ydiff);
   moveCursesWindow (label->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (label));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKLabel (label, ObjOf (label)->box);
   }
}

/*
 * This destroys the label object pointer.
 */
static void _destroyCDKLabel (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKLABEL *label = (CDKLABEL *)object;

      CDKfreeChtypes (label->info);
      freeChecked (label->infoLen);
      freeChecked (label->infoPos);

      /* Free up the window pointers. */
      deleteCursesWindow (label->shadowWin);
      deleteCursesWindow (label->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vLABEL, label);

      /* Unregister the object. */
      unregisterCDKObject (vLABEL, label);
   }
}

/*
 * This pauses until a user hits a key...
 */
char waitCDKLabel (CDKLABEL *label, char key)
{
   int code;
   boolean functionKey;

   /* If the key is null, we'll accept anything. */
   if (key == 0)
   {
      code = getchCDKObject (ObjOf (label), &functionKey);
   }
   else
   {
      /* Only exit when a specific key is hit. */
      for (;;)
      {
	 code = getchCDKObject (ObjOf (label), &functionKey);
	 if (code == key)
	 {
	    break;
	 }
      }
   }
   return (char)(code);
}

dummyInject (Label)

dummyFocus (Label)

dummyUnfocus (Label)

dummyRefreshData (Label)

dummySaveData (Label)

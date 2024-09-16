#include <cdk_int.h>

/*
 * Default method-functions for CDK objects.
 *
 * $Author: tom $
 * $Date: 2019/02/25 00:53:17 $
 * $Revision: 1.24 $
 */

#define MARKUP_CENTER  "<C>"
#define MARKUP_NEWLINE "<#10>"

/*
 * Set the object's upper-left-corner line-drawing character.
 */
void setCdkULchar (CDKOBJS *obj, chtype ch)
{
   obj->ULChar = ch;
}

/*
 * Set the object's upper-right-corner line-drawing character.
 */
void setCdkURchar (CDKOBJS *obj, chtype ch)
{
   obj->URChar = ch;
}

/*
 * Set the object's lower-left-corner line-drawing character.
 */
void setCdkLLchar (CDKOBJS *obj, chtype ch)
{
   obj->LLChar = ch;
}

/*
 * Set the object's upper-right-corner line-drawing character.
 */
void setCdkLRchar (CDKOBJS *obj, chtype ch)
{
   obj->LRChar = ch;
}

/*
 * Set the object's horizontal line-drawing character.
 */
void setCdkHZchar (CDKOBJS *obj, chtype ch)
{
   obj->HZChar = ch;
}

/*
 * Set the object's vertical line-drawing character.
 */
void setCdkVTchar (CDKOBJS *obj, chtype ch)
{
   obj->VTChar = ch;
}

/*
 * Set the object's box-attributes.
 */
void setCdkBXattr (CDKOBJS *obj, chtype ch)
{
   obj->BXAttr = ch;
}

/*
 * This sets the background color of the widget.
 */
void setCDKObjectBackgroundColor (CDKOBJS *obj, const char *color)
{
   chtype *holder = 0;
   int junk1, junk2;

   /* Make sure the color isn't null. */
   if (color == 0)
   {
      return;
   }

   /* Convert the value of the environment variable to a chtype. */
   holder = char2Chtype (color, &junk1, &junk2);

   /* Set the widget's background color. */
   SetBackAttrObj (obj, holder[0]);

   /* Clean up. */
   freeChtype (holder);
}

static char *convert_NL (const char *source)
{
   char *target = strdup (source);
   char *d = target;
   const char *s = source;
   size_t need = strlen (MARKUP_NEWLINE);

   while (*s != '\0')
   {
      if (!strncmp (s, MARKUP_NEWLINE, need))
      {
	 *d++ = '\n';
	 s += need;
      }
      else
      {
	 *d++ = *s++;
      }
   }
   *d = '\0';
   return target;
}

/*
 * Set the widget's title.
 */
int setCdkTitle (CDKOBJS *obj, const char *title, int boxWidth)
{
   if (obj != 0)
   {
      cleanCdkTitle (obj);

      if (title != 0)
      {
	 char *retitle = convert_NL (title);
	 char **temp = 0;
	 int titleWidth;
	 int x;
	 int len;
	 int align;

	 /* We need to split the title on \n. */
	 temp = CDKsplitString (retitle, '\n');
	 obj->titleLines = (int)CDKcountStrings ((CDK_CSTRING2)temp);

	 obj->title = typeCallocN (chtype *, obj->titleLines + 1);
	 obj->titlePos = typeCallocN (int, obj->titleLines + 1);
	 obj->titleLen = typeCallocN (int, obj->titleLines + 1);

	 if (boxWidth >= 0)
	 {
	    int maxWidth = 0;

	    /* We need to determine the widest title line. */
	    for (x = 0; x < obj->titleLines; x++)
	    {
	       chtype *holder = char2Chtype (temp[x], &len, &align);
	       maxWidth = MAXIMUM (maxWidth, len);
	       freeChtype (holder);
	    }
	    boxWidth = MAXIMUM (boxWidth, maxWidth + 2 * obj->borderSize);
	 }
	 else
	 {
	    boxWidth = -(boxWidth - 1);
	 }

	 /* For each line in the title, convert from char * to chtype * */
	 titleWidth = boxWidth - (2 * obj->borderSize);
	 for (x = 0; x < obj->titleLines; x++)
	 {
	    obj->title[x] = char2Chtype (temp[x], &obj->titleLen[x],
					 &obj->titlePos[x]);
	    obj->titlePos[x] = justifyString (titleWidth, obj->titleLen[x],
					      obj->titlePos[x]);
	 }
	 CDKfreeStrings (temp);
	 free (retitle);
      }
   }
   return boxWidth;
}

char *getCdkTitle (CDKOBJS *obj)
{
   int x;
   int pass;
   char *result = 0;
   for (pass = 0; pass < 2; ++pass)
   {
      size_t need = (size_t) (obj->titleLines + 1);
      for (x = 0; x < obj->titleLines; x++)
      {
	 char *title = chtype2String (obj->title[x]);
	 char *check = 0;
	 const char *format = 0;
	 if (title == 0)
	    continue;
	 need += strlen (title);
	 if ((check = chtype2Char (obj->title[x])) != 0)
	 {
	    /* FIXME - we could infer <R> as well */
	    if (isspace (*check) || obj->titlePos[x] > 0)
	       format = MARKUP_CENTER;
	    free (check);
	 }
	 if (x)
	    need += 5;
	 if (format)
	    need += strlen (format);
	 if (pass)
	 {
	    if (x)
	       strcat (result, MARKUP_NEWLINE);
	    if (format)
	       strcat (result, format);
	    strcat (result, title);
	 }
	 free (title);
      }
      if (!pass)
      {
	 result = typeCallocN (char, need);
      }
   }
   return result;
}

/*
 * Draw the widget's title.
 */
void drawCdkTitle (WINDOW *win, CDKOBJS *obj)
{
   if (obj != 0)
   {
      int x;

      for (x = 0; x < obj->titleLines; x++)
      {
	 writeChtype (win,
		      obj->titlePos[x] + obj->borderSize,
		      x + obj->borderSize,
		      obj->title[x],
		      HORIZONTAL, 0,
		      obj->titleLen[x]);
      }
   }
}

/*
 * Remove storage for the widget's title.
 */
void cleanCdkTitle (CDKOBJS *obj)
{
   if (obj != 0)
   {
      CDKfreeChtypes (obj->title);
      obj->title = 0;

      freeAndNull (obj->titlePos);
      freeAndNull (obj->titleLen);

      obj->titleLines = 0;
   }
}

/*
 * Set data for preprocessing.
 */
void setCDKObjectPreProcess (CDKOBJS *obj, PROCESSFN fn, void *data)
{
   obj->preProcessFunction = fn;
   obj->preProcessData = data;
}

/*
 * Set data for postprocessing.
 */
void setCDKObjectPostProcess (CDKOBJS *obj, PROCESSFN fn, void *data)
{
   obj->postProcessFunction = fn;
   obj->postProcessData = data;
}

/*
 * Set the object's exit-type based on the input.
 * The .exitType field should have been part of the CDKOBJS struct, but it
 * is used too pervasively in older applications to move (yet).
 */
void setCdkExitType (CDKOBJS *obj, EExitType *type, chtype ch)
{
   switch (ch)
   {
   case KEY_ERROR:
      *type = vERROR;
      break;
   case KEY_ESC:
      *type = vESCAPE_HIT;
      break;
   case KEY_TAB:
   case KEY_ENTER:
      *type = vNORMAL;
      break;
   case 0:
      *type = vEARLY_EXIT;
      break;
   }
   /* make the result available via ExitTypeOf(obj) */
   obj->exitType = *type;
}

#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2012/03/20 09:54:37 $
 * $Revision: 1.4 $
 */

/*
 * Given a character pointer, returns the equivalent display type.
 */
EDisplayType char2DisplayType (const char *string)
{
   /* *INDENT-OFF* */
   static const struct {
      const char *name;
      EDisplayType code;
   } table[] = {
      { "CHAR",		vCHAR },
      { "HCHAR",	vHCHAR },
      { "INT",		vINT },
      { "HINT",		vHINT },
      { "UCHAR",	vUCHAR },
      { "LCHAR",	vLCHAR },
      { "UHCHAR",	vUHCHAR },
      { "LHCHAR",	vLHCHAR },
      { "MIXED",	vMIXED },
      { "HMIXED",	vHMIXED },
      { "UMIXED",	vUMIXED },
      { "LMIXED",	vLMIXED },
      { "UHMIXED",	vUHMIXED },
      { "LHMIXED",	vLHMIXED },
      { "VIEWONLY",	vVIEWONLY },
      { 0,		vINVALID },
   };
   /* *INDENT-ON* */

   if (string != 0)
   {
      int n;
      for (n = 0; table[n].name != 0; n++)
      {
	 if (!strcmp (string, table[n].name))
	    return table[n].code;
      }
   }
   return (EDisplayType) vINVALID;
}

/*
 * Tell if a display type is "hidden"
 */
boolean isHiddenDisplayType (EDisplayType type)
{
   boolean result = FALSE;

   switch (type)
   {
   case vHCHAR:
   case vHINT:
   case vHMIXED:
   case vLHCHAR:
   case vLHMIXED:
   case vUHCHAR:
   case vUHMIXED:
      result = TRUE;
      break;
   case vCHAR:
   case vINT:
   case vINVALID:
   case vLCHAR:
   case vLMIXED:
   case vMIXED:
   case vUCHAR:
   case vUMIXED:
   case vVIEWONLY:
      result = FALSE;
      break;
   }
   return result;
}

/*
 * Given a character input, check if it is allowed by the display type,
 * and return the character to apply to the display, or ERR if not.
 */
int filterByDisplayType (EDisplayType type, chtype input)
{
   int result = CharOf (input);

   if (!isChar (input))
   {
      result = ERR;
   }
   else if ((type == vINT ||
	     type == vHINT) &&
	    !isdigit (CharOf (result)))
   {
      result = ERR;
   }
   else if ((type == vCHAR ||
	     type == vUCHAR ||
	     type == vLCHAR ||
	     type == vUHCHAR ||
	     type == vLHCHAR) &&
	    isdigit (CharOf (result)))
   {
      result = ERR;
   }
   else if (type == vVIEWONLY)
   {
      result = ERR;
   }
   else if ((type == vUCHAR ||
	     type == vUHCHAR ||
	     type == vUMIXED ||
	     type == vUHMIXED) &&
	    isalpha (CharOf (result)))
   {
      result = toupper (result);
   }
   else if ((type == vLCHAR ||
	     type == vLHCHAR ||
	     type == vLMIXED ||
	     type == vLHMIXED) &&
	    isalpha (CharOf (result)))
   {
      result = tolower (result);
   }
   return result;
}

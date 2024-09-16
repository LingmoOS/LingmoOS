#include <cdk_int.h>

/*
 * Useful functions for command-line parsing.
 *
 * $Author: tom $
 * $Date: 2016/11/20 18:32:53 $
 * $Revision: 1.12 $
 */

#define OPTION_ON	((char *)1)
#define OPTION_OFF	((char *)0)

static void usage (char **argv,
		   CDK_PARAMS * params,
		   const char *options)
{
   int n;
   const char *str;
   char *base = baseName (argv[0]);

   fprintf (stderr, "Usage: %s [options]\n\nOptions:\n", base);

   for (n = 1; n < MAX_CDK_PARAMS; ++n)
   {
      if (n != ':'
	  && (str = (strchr) (options, n)) != 0)
      {
	 int value = (str[1] == ':');
	 fprintf (stderr, "  -%c", n);
	 if (value)
	 {
	    fprintf (stderr, " (%s)\n",
		     (params->allParams[n]
		      ? params->allParams[n]
		      : "not set"));
	 }
	 else
	 {
	    fprintf (stderr, " (%s)\n",
		     (params->allParams[n]
		      ? "set"
		      : "not set"));
	 }
      }
   }
   free (base);
   exit (EXIT_FAILURE);
}

static int CDKparseSize (char *string, int fullSize)
{
   int result;
   if (strcmp (string, "FULL") == 0)
   {
      result = fullSize;
   }
   else
   {
      result = (int)strtol (string, (char **)0, 0);
   }
   return result;
}

/*
 * Parse the string as one of CDK's positioning keywords, or an actual
 * position.
 */
int CDKparsePosition (const char *string)
{
   int result;

   if (string == 0)
   {
      result = NONE;
   }
   else if (strcmp (string, "TOP") == 0)
   {
      result = TOP;
   }
   else if (strcmp (string, "BOTTOM") == 0)
   {
      result = BOTTOM;
   }
   else if (strcmp (string, "LEFT") == 0)
   {
      result = LEFT;
   }
   else if (strcmp (string, "RIGHT") == 0)
   {
      result = RIGHT;
   }
   else if (strcmp (string, "CENTER") == 0)
   {
      result = CENTER;
   }
   else
   {
      result = (int)strtol (string, (char **)0, 0);
   }
   return result;
}

/*
 * Parse the given argc/argv command-line, with the options passed to
 * getopt()'s 3rd parameter.
 */
void CDKparseParams (int argc,
		     char **argv,
		     CDK_PARAMS * params,
		     const char *options)
{
   int code;

   memset (params, 0, sizeof (*params));
   params->Box = TRUE;

   while ((code = getopt (argc, argv, options)) != EOF)
   {
      const char *str;

      if (code == '?' || (str = (strchr) (options, code)) == 0)
      {
	 usage (argv, params, options);
      }
      else
      {
	 params->allParams[code] = OPTION_ON;
	 if (str[1] == ':')
	 {
	    params->allParams[code] = optarg;
	 }
	 switch (code)
	 {
	 case 'W':
	    params->wValue = CDKparseSize (optarg, FULL);
	    break;

	 case 'H':
	    params->hValue = CDKparseSize (optarg, FULL);
	    break;

	 case 'X':
	    params->xValue = CDKparsePosition (optarg);
	    break;

	 case 'Y':
	    params->yValue = CDKparsePosition (optarg);
	    break;

	 case 'N':
	    params->Box = FALSE;
	    break;

	 case 'S':
	    params->Shadow = TRUE;
	    break;
	 }
      }
   }
}

/*
 * Retrieve a numeric option-value, default=0.
 */
int CDKparamNumber (CDK_PARAMS * params, int option)
{
   return CDKparamNumber2 (params, option, 0);
}

/*
 * Retrieve a numeric option-value, given default.
 */
int CDKparamNumber2 (CDK_PARAMS * params, int option, int missing)
{
   return CDKparamValue (params, option, missing);
}

/*
 * Retrieve the string value of an option, default=0.
 */
char *CDKparamString (CDK_PARAMS * params, int option)
{
   return CDKparamString2 (params, option, 0);
}

/*
 * Retrieve the string value of an option, with default for missing value.
 */
char *CDKparamString2 (CDK_PARAMS * params, int option, const char *missing)
{
   char *value = ((option > 0 && option < MAX_CDK_PARAMS)
		  ? params->allParams[option]
		  : 0);
   if (value == 0)
      value = copyChar (missing);
   return value;
}

/*
 * Retrieve an integer (or boolean) option value from the parsed command-line.
 * (prefer: CDKparamNumber).
 */
int CDKparamValue (CDK_PARAMS * params, int option, int missing)
{
   int result;
   char *value = CDKparamString (params, option);

   if (value == 0)
   {
      result = missing;
   }
   else if (strchr (CDK_CLI_PARAMS, option) != 0)
   {
      switch (option)
      {
      case 'H':
	 result = params->hValue;
	 break;
      case 'W':
	 result = params->wValue;
	 break;
      case 'X':
	 result = params->xValue;
	 break;
      case 'Y':
	 result = params->yValue;
	 break;
      case 'N':
	 result = params->Box;
	 break;
      case 'S':
	 result = params->Shadow;
	 break;
      default:
	 result = missing;
	 break;
      }
   }
   else if (value == OPTION_ON)
   {
      result = 1;
   }
   else
   {
      result = (int)strtol (value, (char **)0, 0);
   }

   return result;
}

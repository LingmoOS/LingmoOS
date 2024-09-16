/*
 * $Id: cdk_params.h,v 1.8 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDK_PARAMS_H
#define CDK_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CDK_H
#define CDKINCLUDES
#include <cdk.h>
#undef CDKINCLUDES
#endif

/*
 * Copyright 2003-2005,2012 Thomas E. Dickey
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, distribute with
 * modifications, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#define MAX_CDK_PARAMS 256

/*
 * CDKparseParams() knows about these options and will decode them into
 * the CDK_PARAMS struct.  They are the most generally useful for positioning
 * a widget.
 */
#define CDK_MIN_PARAMS	"NSX:Y:"
#define CDK_CLI_PARAMS	"NSX:Y:H:W:"

/*
 * Use this exit code rather than -1 for cli programs which have reported an
 * error.  Actually EXIT_FAILURE would be better, but the shell script samples
 * all are written to assume that the exit code can be used to indicate a
 * button number, etc.
 */
#define CLI_ERROR	255

/*
 * This records the values that CDKparseParams() decodes using getopt():
 */
typedef struct CDK_PARAMS {
   char *	allParams[MAX_CDK_PARAMS];
   bool		Box;
   bool		Shadow;
   int		hValue;
   int		wValue;
   int		xValue;
   int		yValue;
} CDK_PARAMS;

/*
 * Parse the given argc/argv command-line, with the options passed to
 * getopt()'s 3rd parameter.
 */
void CDKparseParams (
		int		/* argc */,
		char **		/* argv */,
		CDK_PARAMS *	/* params */,
		const char *	/* options */);

/*
 * Parse the string as one of CDK's positioning keywords, or an actual
 * position.
 */
int CDKparsePosition (
		const char *	/* string */);

/*
 * Retrieve an integer (or boolean) option value from the parsed command-line.
 */
int CDKparamNumber (
		CDK_PARAMS *	/* params */,
		int		/* option */);

/*
 * Retrieve an optional integer (or boolean) value from the parsed command-line.
 */
int CDKparamNumber2 (
		CDK_PARAMS *	/* params */,
		int		/* option */,
		int		/* missing */);

/*
 * Retrieve a string option value from the parsed command-line.
 */
char * CDKparamString (
		CDK_PARAMS *	/* params */,
		int		/* option */);

/*
 * Retrieve an optional string option value from the parsed command-line.
 */
char * CDKparamString2 (
		CDK_PARAMS *	/* params */,
		int		/* option */,
		const char *	/* missing */);

/*
 * Retrieve an integer (or boolean) option value from the parsed command-line.
 */
int CDKparamValue (
		CDK_PARAMS *	/* params */,
		int		/* option */,
		int		/* missing */);

#ifdef __cplusplus
}
#endif

#endif /* CDK_PARAMS_H */
#endif /* CDKINCLUDES */

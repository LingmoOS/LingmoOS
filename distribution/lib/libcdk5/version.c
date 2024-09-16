#include <cdk.h>

/*
 * $Author: tom $
 * $Date: 2012/03/20 22:06:08 $
 * $Revision: 1.2 $
 */
const char *CDKVersion (void)
{
   return CDK_VERSION_MAJOR "." CDK_VERSION_MINOR " - " CDK_VERSION_PATCH;
}

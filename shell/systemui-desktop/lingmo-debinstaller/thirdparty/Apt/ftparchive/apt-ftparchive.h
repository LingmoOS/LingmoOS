// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
/* ######################################################################

   Writer 
   
   The file writer classes. These write various types of output, sources,
   packages and contents.
   
   ##################################################################### */
									/*}}}*/
#ifndef APT_FTPARCHIVE_H
#define APT_FTPARCHIVE_H

#include <fstream>

using std::ostream;
using std::ofstream;

extern ostream c0out;
extern ostream c1out;
extern ostream c2out;
extern ofstream devnull;
extern unsigned Quiet;

#endif

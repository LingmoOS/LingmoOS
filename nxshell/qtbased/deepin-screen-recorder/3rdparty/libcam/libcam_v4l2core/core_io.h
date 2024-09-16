/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#                                                                               #
# This program is free software; you can redistribute it and/or modify          #
# it under the terms of the GNU General Public License as published by          #
# the Free Software Foundation; either version 2 of the License, or             #
# (at your option) any later version.                                           #
#                                                                               #
# This program is distributed in the hope that it will be useful,               #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 #
# GNU General Public License for more details.                                  #
#                                                                               #
# You should have received a copy of the GNU General Public License             #
# along with this program; if not, write to the Free Software                   #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     #
#                                                                               #
********************************************************************************/

#ifndef CORE_IO_H
#define CORE_IO_H

#include <inttypes.h>
/*
 * converts string to lowercase
 * args:
 *   str - string pointer
 *
 * asserts:
 *   none
 *
 * returns: pointer to converted string
 */
char *lowercase(char *str);

/*
 * trim leading white spaces from source string
 * args:
 *    src - source string
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int trim_leading_wspaces(char *src);

/*
 * trim trailing white spaces and control chars (\n) from source string
 * args:
 *    src - source string
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
int trim_trailing_wspaces(char *src);

/*
 * gets the number of chars to represent n
 * args:
 *    n - uint64_t number to represent
 *
 * asserts:
 *    none
 *
 * returns: number of chars needed to represent n
 */
int get_uint64_num_chars (uint64_t n);

/*
 * smart concatenation
 * args:
 *    dest - destination string
 *    c - connector char
 *    str1 - string to concat
 *
 * asserts:
 *    none
 *
 * returns: concatenated string (must free)
 */
char *smart_cat(const char *dest, char c, const char *str1);

/*
 * get the filename basename
 * args:
 *    filename - string with filename (full path)
 *
 * asserts:
 *    none
 *
 * returns: new string with basename (must free it)
 */
char *get_file_basename(const char *filename);

/*
 * get the filename path
 * args:
 *    filename - string with filename (full path)
 *
 * asserts:
 *    none
 *
 * returns: new string with path (must free it)
 *      or NULL if no path found
 */
char *get_file_pathname(const char *filename);

/*
 * get the filename extension
 * args:
 *    filename - string with filename (full path)
 *
 * asserts:
 *    none
 *
 * returns: new string with extension (must free it)
 *      or NULL if no extension found
 */
char *get_file_extension(const char *filename);

/*
 * change the filename extension
 * args:
 *    filename - string with filename
 *    ext - string with new extension
 *
 * asserts:
 *    none
 *
 * returns: string with new extension (must free it)
 */
char *set_file_extension(const char *filename, const char *ext);

/*
 * get the sufix for filename in path (e.g. for file-3.png sufix is 3)
 * args:
 *   path - string with file path
 *   filename - string with file basename
 *
 * asserts:
 *   none
 *
 * returns: none
 */
unsigned long long get_file_suffix(const char *path, const char* filename);

/*
 * add a number suffix to filename (e.g. name.ext => name-suffix.ext)
 *   the suffix depends on the existing values in the path dir
 * args:
 * 	  path - string with file path (to dir)
 *    filename - string with file basename (name.ext)
 *    suffix - suffix number
 *
 * asserts:
 *    none
 *
 * returns: newly allocated string with suffixed file name (must free)
 */
char *add_file_suffix(const char *path, const char *filename);

#endif
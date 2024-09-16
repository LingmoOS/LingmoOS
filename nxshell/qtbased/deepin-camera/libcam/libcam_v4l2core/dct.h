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

/*******************************************************************************#
#                                                                               #
#  dct for Jpeg encoder                                                         #
#                                                                               #
#  Adapted for linux, Paulo Assis, 2007 <pj.assis@gmail.com>                    #
********************************************************************************/

#ifndef DCT_H
#define DCT_H

#include <inttypes.h>
#include <sys/types.h>

/*
 * Level shifting to get 8 bit SIGNED values for the data
 * args:
 *    data - pointer to data
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void levelshift (int16_t * const data);

/*
 * DCT for One block(8x8)
 * args:
 *    data- pointer to data
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void DCT (int16_t *data);

#endif
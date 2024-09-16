/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Dr. Alexander K. Seewald <alex@seewald.at>                          #
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
#  autofocus - using dct for sharpness measure                                  #
#                                                                               #
#                                                                               #
********************************************************************************/

#ifndef SOFT_AUTOFOCUS_H
#define SOFT_AUTOFOCUS_H

#include <inttypes.h>
#include <sys/types.h>
#include "v4l2_core.h"

/*
 * initiate software autofocus
 * args:
 *    vd - pointer to device data
 *
 * asserts:
 *    vd is not null
 *
 * returns: error code (0 - E_OK)
 */
int soft_autofocus_init (v4l2_dev_t *vd);

/*
 * run the software autofocus
 * args:
 *    vd - pointer to device data
 *    frame - pointer to frame buffer
 *
 * asserts:
 *    vd is not null
 *
 * returns: 1 - running  0- focused
 * 	(only matters for non-continue focus)
 */
int soft_autofocus_run(v4l2_dev_t *vd, v4l2_frame_buff_t *frame);

/*
 * sharpness in focus window
 * args:
 *    frame - pointer to image frame
 *    width - frame width
 *    height - frame height
 *    t - highest order coef
 *
 * asserts:
 *    none
 *
 * returns: sharpness value
 */
int soft_autofocus_get_sharpness (uint8_t *frame, int width, int height, int t);

/*
 * get focus value
 * args:
 *    none
 *
 * asserts:
 *    focus_ctx is not null
 *
 * returns: focus code
 */
int soft_autofocus_get_focus_value ();

#endif
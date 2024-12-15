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

#ifndef CONTROL_PROFILE_H
#define CONTROL_PROFILE_H

#include "v4l2_core.h"

/*
 * save the device control values into a profile file
 * args:
 *   vd - pointer to video device data
 *   filename - profile filename
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code (0 -E_OK)
 */
int save_control_profile(v4l2_dev_t *vd, const char *filename);

/*
 * load the device control values from a profile file
 * args:
 *   vd - pointer to video device data
 *   filename - profile filename
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code (0 -E_OK)
 */
int load_control_profile(v4l2_dev_t *vd, const char *filename);


#endif
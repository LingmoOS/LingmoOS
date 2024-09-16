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
#ifndef CORE_TIME_H
#define CORE_TIME_H

#include <inttypes.h>
#include <sys/types.h>

/*
 * time in miliseconds
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: time of day in miliseconds
 */
//uint32_t ms_time ();

/*
 * time in microseconds
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: time of day in microseconds
 */
//uint64_t us_time();

/*
 * time in nanoseconds
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: time in nanoseconds
 */
//uint64_t ns_time ();

/*
 * monotonic time in nanoseconds
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: monotonic time in nanoseconds
 */
uint64_t ns_time_monotonic();

#endif


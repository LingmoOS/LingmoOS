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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "core_time.h"
#include "gview.h"

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
//uint32_t ms_time ()
//{
//	struct timeval now;
//
//	if(gettimeofday(&now, NULL) != 0)
//	{
//		fprintf(stderr, "V4L2_CORE: ms_time (gettimeofday) error: %s\n", strerror(errno));
//		return 0;
//	}
//
//	uint32_t mst = (uint32_t) now.tv_sec * 1000 + (uint32_t) now.tv_usec / 1000;
//
//	return (mst);
//}

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
//uint64_t us_time(void)
//{
//	struct timeval now;
//
//	if(gettimeofday(&now, NULL) != 0)
//	{
//		fprintf(stderr, "V4L2_CORE: us_time (gettimeofday) error: %s\n", strerror(errno));
//		return 0;
//	}
//
//	uint64_t ust = (uint64_t) now.tv_sec * USEC_PER_SEC + (uint64_t) now.tv_usec;
//
//	return (ust);
//}

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
//uint64_t ns_time (void)
//{
//	struct timespec now;
//
//	if(clock_gettime(CLOCK_REALTIME, &now) != 0)
//	{
//		fprintf(stderr, "V4L2_CORE: ns_time (clock_gettime) error: %s\n", strerror(errno));
//		return 0;
//	}
//
//	return ((uint64_t) now.tv_sec * NSEC_PER_SEC + (uint64_t) now.tv_nsec);
//}

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
uint64_t ns_time_monotonic()
{
	struct timespec now;

	if(clock_gettime(CLOCK_MONOTONIC, &now) != 0)
	{
		fprintf(stderr, "V4L2_CORE: ns_time_monotonic (clock_gettime) error: %s\n", strerror(errno));
		return 0;
	}

	return ((uint64_t)now.tv_sec * NSEC_PER_SEC + (uint64_t) now.tv_nsec);
}

/*
 * get current timestamp
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: monotonic time in nanoseconds
 */
uint64_t v4l2core_time_get_timestamp()
{
	return ns_time_monotonic();
}


/*
void sleep_ms(int ms_time)
{
	gulong sleep_us = ms_time *1000;
	g_usleep( sleep_us );
}


int wait_ms(gboolean* var, gboolean val, __MUTEX_TYPE *mutex, int ms_time, int n_loops)
{
	int n=n_loops;
	__LOCK_MUTEX(mutex);
		while( (*var!=val) && ( n > 0 ) )
		{
			__UNLOCK_MUTEX(mutex);
			n--;
			sleep_ms( ms_time );
			__LOCK_MUTEX(mutex);
		};
	__UNLOCK_MUTEX(mutex);
	return (n);
}
*/

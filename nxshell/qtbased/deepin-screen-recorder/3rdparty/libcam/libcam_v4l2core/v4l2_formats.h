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
#ifndef V4L2_FORMATS_H
#define V4L2_FORMATS_H

#include "gviewv4l2core.h"
#include "v4l2_core.h"

/*
 * check pixelformat against decoder support formats
 * args:
 *    pixelformat - v4l2 pixelformat
 *
 * asserts:
 *    none
 *
 * returns: TRUE(1) if format is supported
 *          FALSE(0) if not
 */
uint8_t can_decode_format(uint32_t pixelformat);

/*
 * enumerate frame formats (pixelformats, resolutions and fps)
 * and creates list in vd->list_stream_formats
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *   vd->list_stream_formats is null
 *
 * returns: 0 (E_OK) if enumeration succeded or error otherwise
 */
int enum_frame_formats(v4l2_dev_t *vd);

/* get frame format index from format list
 * args:
 *   vd - pointer to video device data
 *   format - v4l2 pixel format
 *
 * asserts:
 *   vd is not null
 *   vd->list_stream_formats is not null
 *
 * returns: format list index or -1 if not available
 */
int get_frame_format_index(v4l2_dev_t *vd, int format);

/* get resolution index for format index from format list
 * args:
 *   vd - pointer to video device data
 *   format - format index from format list
 *   width - requested width
 *   height - requested height
 *
 * asserts:
 *   vd is not null
 *   vd->list_stream_formats is not null
 *
 * returns: resolution list index for format index or -1 if not available
 */
int get_format_resolution_index(v4l2_dev_t *vd, int format, int width, int height);

/*
 * free frame formats list
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->list_stream_formats is not null
 *
 * returns: void
 */
void free_frame_formats(v4l2_dev_t *vd);

#endif

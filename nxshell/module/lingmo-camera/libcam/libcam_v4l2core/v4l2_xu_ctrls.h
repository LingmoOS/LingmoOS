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

#ifndef V4L2_XU_CTRLS_H
#define V4L2_XU_CTRLS_H

#include "gviewv4l2core.h"
#include "v4l2_core.h"

typedef struct
{
  int8_t bLength;
  int8_t bDescriptorType;
  int8_t bDescriptorSubType;
  int8_t bUnitID;
  uint8_t guidExtensionCode[16];
} __attribute__ ((__packed__)) xu_descriptor;

#define USB_VIDEO_CONTROL		    0x01
#define USB_VIDEO_CONTROL_INTERFACE	0x24
#define USB_VIDEO_CONTROL_XU_TYPE	0x06

/*
 * get GUID unit id, if any
 * args:
 *   vd - pointer to video device data
 *   guid - 16 byte xu GUID
 *
 * asserts:
 *   vd is not null
 *   device_list->list_devices is not null
 *
 * returns: unit id for the matching GUID or 0 if none
 */
uint8_t get_guid_unit_id (v4l2_dev_t *vd, uint8_t *guid);

/*
 * tries to map available xu controls for supported devices
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *
 * returns: 0 if map succeded or errno otherwise
 */
int init_xu_ctrls(v4l2_dev_t *vd);

/*
 * get lenght of xu control defined by unit id and selector
 * args:
 *   vd - pointer to video device data
 *   unit - unit id of xu control
 *   selector - selector for control
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *
 * returns: length of xu control
 */
uint16_t get_length_xu_control(v4l2_dev_t *vd, uint8_t unit, uint8_t selector);

/*
 * get uvc info for xu control defined by unit id and selector
 * args:
 *   vd - pointer to video device data
 *   unit - unit id of xu control
 *   selector - selector for control
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *
 * returns: info of xu control
 */
uint8_t get_info_xu_control(v4l2_dev_t *vd, uint8_t unit, uint8_t selector);

/*
 * runs a query on xu control defined by unit id and selector
 * args:
 *   vd - pointer to video device data
 *   unit - unit id of xu control
 *   selector - selector for control
 *   query - query type
 *   data - pointer to query data
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *
 * returns: 0 if query succeded or errno otherwise
 */
int query_xu_control(v4l2_dev_t *vd, uint8_t unit, uint8_t selector, uint8_t query, void *data);

#endif

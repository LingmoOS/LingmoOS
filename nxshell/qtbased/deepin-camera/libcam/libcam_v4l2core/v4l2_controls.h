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
#ifndef V4L2_CONTROLS_H
#define V4L2_CONTROLS_H

#include "gviewv4l2core.h"
#include "v4l2_core.h"

/*
 * enumerate device (read/write) controls
 * and creates list in vd->list_device_controls
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *   vd->list_device_controls is null
 *
 * returns: error code
 */
int enumerate_v4l2_control(v4l2_dev_t *vd);

/*
 * subscribe for v4l2 control events
 * args:
 *  vd - pointer to video device data
 *  control_id - id of control to subscribe events for
 *
 * asserts:
 *  vd is not null
 *
 * return: none
 */
void v4l2_subscribe_control_events(v4l2_dev_t *vd, unsigned int control_id);

/*
 * unsubscribev4l2 control events
 * args:
 *  vd - pointer to video device data
 *
 * asserts:
 *  vd is not null
 *
 * return: none
 */
void v4l2_unsubscribe_control_events(v4l2_dev_t *vd);

/*
 * return the control associated to id from device list
 * args:
 *   vd - pointer to video device data
 *   id - control id
 *
 * asserts:
 *   vd is not null
 *
 * returns: pointer to v4l2_control if succeded or null otherwise
 */
v4l2_ctrl_t *get_control_by_id(v4l2_dev_t *vd, int id);

/*
 * updates the value for control id from the device
 * also updates control flags
 * args:
 *   vd - pointer to video device data
 *   id - control id
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid
 *
 * returns: ioctl result
 */
int get_control_value_by_id (v4l2_dev_t *vd, int id);

/*
 * sets the value of control id in device
 * args:
 *   vd - pointer to video device data
 *   id - control id
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid
 *
 * returns: ioctl result
 */
int set_control_value_by_id(v4l2_dev_t *vd, int id);

/*
 * goes trough the control list and updates/retrieves current values
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
void get_v4l2_control_values (v4l2_dev_t *vd);

/*
 * goes trough the control list and sets values in device
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
void set_v4l2_control_values (v4l2_dev_t *vd);

/*
 * goes trough the control list and sets values in device to default
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
void set_control_defaults(v4l2_dev_t *vd);

/*
 * Disables special auto-controls with higher IDs than
 * their absolute/relative counterparts
 * this is needed before restoring controls state
 *
 * args:
 *   vd - pointer to video device data
 *   id - control id
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
void disable_special_auto (v4l2_dev_t *vd, int id);


/*
 * free control list
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
void free_v4l2_control_list(v4l2_dev_t *vd);

#endif

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
#include <linux/videodev2.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <libv4l2.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "gviewv4l2core.h"
#include "v4l2_devices.h"
#include "v4l2_controls.h"
#include "v4l2_xu_ctrls.h"
#include "cameraconfig.h"
#include "load_libs.h"

#ifndef V4L2_CTRL_ID2CLASS
#define V4L2_CTRL_ID2CLASS(id)    ((id) & 0x0fff0000UL)
#endif

extern int verbosity;

// GUID for logitech peripheral (pan/tilt) V3 extension unit: {FFE52D21-8030-4E2C-82d9-f587d00540bd}
#define GUID_LOGITECH_PERIPHERAL_XU {0x21, 0x2D, 0xE5, 0xFF, 0x30, 0x80, 0x2C, 0x4E, 0x82, 0xD9, 0xF5, 0x87, 0xD0, 0x05, 0x40, 0xBD}

/*
 * needed only for language files (not used)
 */
/* V4L2 control strings */
#define CSTR_USER_CLASS		N_("User Controls")
#define	CSTR_BRIGHT 		N_("Brightness")
#define	CSTR_CONTRAST 		N_("Contrast")
#define	CSTR_HUE 		N_("Hue")
#define	CSTR_SATURAT		N_("Saturation")
#define	CSTR_SHARP		N_("Sharpness")
#define	CSTR_GAMMA		N_("Gamma")
#define	CSTR_BLCOMP		N_("Backlight Compensation")
#define	CSTR_PLFREQ		N_("Power Line Frequency")
#define CSTR_HUEAUTO		N_("Hue, Automatic")
#define	CSTR_FOCUSAUTO		N_("Focus, Auto")
#define CSTR_EXPMENU1		N_("Manual Mode")
#define CSTR_EXPMENU2		N_("Auto Mode")
#define CSTR_EXPMENU3		N_("Shutter Priority Mode")
#define CSTR_EXPMENU4		N_("Aperture Priority Mode")
#define CSTR_BLACK_LEVEL	N_("Black Level")
#define CSTR_AUTO_WB		N_("White Balance, Automatic")
#define CSTR_DO_WB		N_("Do White Balance")
#define CSTR_RB			N_("Red Balance")
#define	CSTR_BB			N_("Blue Balance")
#define CSTR_EXP		N_("Exposure")
#define CSTR_AUTOGAIN		N_("Gain, Automatic")
#define	CSTR_GAIN		N_("Gain")
#define CSTR_HFLIP		N_("Horizontal Flip")
#define CSTR_VFLIP		N_("Vertical Flip")
#define CSTR_HCENTER		N_("Horizontal Center")
#define CSTR_VCENTER		N_("Vertical Center")
#define CSTR_CHR_AGC		N_("Chroma AGC")
#define CSTR_CLR_KILL		N_("Color Killer")
#define CSTR_COLORFX		N_("Color Effects")

/* CAMERA CLASS control strings */
#define CSTR_CAMERA_CLASS	N_("Camera Controls")
#define CSTR_EXPAUTO		N_("Auto Exposure")
#define	CSTR_EXPABS		    N_("Exposure Time, Absolute")
#define CSTR_EXPAUTOPRI		N_("Exposure, Dynamic Framerate")
#define	CSTR_PAN_REL		N_("Pan, Relative")
#define CSTR_TILT_REL		N_("Tilt, Relative")
#define CSTR_PAN_RESET		N_("Pan, Reset")
#define CSTR_TILT_RESET		N_("Tilt, Reset")
#define CSTR_PAN_ABS		N_("Pan, Absolute")
#define CSTR_TILT_ABS		N_("Tilt, Absolute")
#define CSTR_FOCUS_ABS		N_("Focus, Absolute")
#define CSTR_FOCUS_REL		N_("Focus, Relative")
#define CSTR_FOCUS_AUTO		N_("Focus, Automatic")
#define CSTR_ZOOM_ABS		N_("Zoom, Absolute")
#define CSTR_ZOOM_REL		N_("Zoom, Relative")
#define CSTR_ZOOM_CONT		N_("Zoom, Continuous")
#define CSTR_PRIV		N_("Privacy")

/* UVC specific control strings */
#define	CSTR_EXPAUTO_UVC	N_("Exposure, Auto")
#define	CSTR_EXPAUTOPRI_UVC	N_("Exposure, Auto Priority")
#define	CSTR_EXPABS_UVC		N_("Exposure (Absolute)")
#define	CSTR_WBTAUTO_UVC	N_("White Balance Temperature, Auto")
#define	CSTR_WBT_UVC		N_("White Balance Temperature")
#define CSTR_WBCAUTO_UVC	N_("White Balance Component, Auto")
#define CSTR_WBCB_UVC		N_("White Balance Blue Component")
#define	CSTR_WBCR_UVC		N_("White Balance Red Component")

/* libwebcam specific control strings */
#define CSTR_FOCUS_LIBWC	N_("Focus")
#define CSTR_FOCUSABS_LIBWC	N_("Focus (Absolute)")


/*
 * don't use xioctl for control query when using V4L2_CTRL_FLAG_NEXT_CTRL
 * args:
 *   vd - pointer to video device data
 *   current_ctrl - current control id
 *   ctrl - pointer to v4l2_queryctrl data
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid ( > 0 )
 *   ctrl is not null
 *
 * returns: error code
 */
static int query_ioctl(v4l2_dev_t *vd, int current_ctrl, struct v4l2_queryctrl* ctrl)
{
	/*assertions*/
	assert(vd != NULL);
	assert(vd->fd > 0);
	assert(ctrl != NULL);

    int ret = 0;
    int tries = 4;
    do
    {
        if(ret)
            ctrl->id = (__u32)current_ctrl | V4L2_CTRL_FLAG_NEXT_CTRL;
        ret = getV4l2()->m_v4l2_ioctl(vd->fd, VIDIOC_QUERYCTRL, ctrl);
    }
    while (ret && tries-- &&
        ((errno == EIO || errno == EPIPE || errno == ETIMEDOUT)));

    return(ret);
}

/*
 * output control data
 * args:
 *   control - pointer to control data
 *   i - control index (from control list)
 *
 * asserts:
 *   none
 *
 * returns: void
 */
static void print_control(v4l2_ctrl_t *control, int i)
{
	if(control == NULL)
	{
		printf("V4L2_CORE: null control at index %i\n", i);
		return;
	}

	int j=0;

	switch (control->control.type)
	{
		case V4L2_CTRL_TYPE_INTEGER:
			printf("control[%d]:(int) 0x%x '%s'\n",i ,control->control.id, control->name);
			printf("\tmin:%d max:%d step:%d def:%d curr:%d\n",
				control->control.minimum, control->control.maximum, control->control.step,
				control->control.default_value, control->value);
            free(control->name);
            control->name=NULL;
			break;

		case V4L2_CTRL_TYPE_INTEGER64:
			printf("control[%d]:(int64) 0x%x '%s'\n",i ,control->control.id, control->name);
			printf ("\tcurr:%" PRId64 "\n", control->value64);
            free(control->name);
            control->name=NULL;
			break;

		case V4L2_CTRL_TYPE_STRING:
			printf("control[%d]:(str) 0x%x '%s'\n",i ,control->control.id, control->name);
			printf ("\tmin:%d max:%d step:%d curr: %s\n",
				control->control.minimum, control->control.maximum, 
				control->control.step, control->string);
            free(control->name);
            control->name=NULL;
			break;

		case V4L2_CTRL_TYPE_BOOLEAN:
			printf("control[%d]:(bool) 0x%x '%s'\n",i ,control->control.id, control->name);
			printf ("\tdef:%d curr:%d\n",
				control->control.default_value, control->value);
            free(control->name);
            control->name=NULL;
			break;

		case V4L2_CTRL_TYPE_MENU:
			printf("control[%d]:(menu) 0x%x '%s'\n",i ,control->control.id, control->name);
			printf("\tmin:%d max:%d def:%d curr:%d\n",
				control->control.minimum, control->control.maximum,
				control->control.default_value, control->value);
            for (j = 0; (__s32)control->menu[j].index <= control->control.maximum; j++)
				printf("\tmenu[%d]: [%d] -> '%s'\n", j, control->menu[j].index, control->menu_entry[j]);
            free(control->name);
            control->name=NULL;
			break;

		case V4L2_CTRL_TYPE_INTEGER_MENU:
			printf("control[%d]:(intmenu) 0x%x '%s'\n",i ,control->control.id, control->name);
			printf("\tmin:%d max:%d def:%d curr:%d\n",
				control->control.minimum, control->control.maximum,
				control->control.default_value, control->value);
            for (j = 0; (__s32)control->menu[j].index <= control->control.maximum; j++)
				printf("\tmenu[%d]: [%d] -> %" PRId64 " (0x%" PRIx64 ")\n", j, control->menu[j].index,
					(int64_t) control->menu[j].value,
					(int64_t) control->menu[j].value);
            free(control->name);
            control->name=NULL;
            break;

		case V4L2_CTRL_TYPE_BUTTON:
			printf("control[%d]:(button) 0x%x '%s'\n",i ,control->control.id, control->name);
            free(control->name);
            control->name=NULL;
			break;

		case V4L2_CTRL_TYPE_BITMASK:
         //LMH0613减去这2条有警告的打印信息
//			printf("control[%d]:(bitmask) 0x%x '%s'\n",i ,control->control.id, control->name);

//			printf("\tmin:%x max:%x def:%x curr:%x\n",
//				control->control.minimum, control->control.maximum,
//				control->control.default_value, control->value);

		default:
			printf("control[%d]:(unknown - 0x%x) 0x%x '%s'\n",i ,control->control.type,
				control->control.id, control->control.name);
            free(control->name);
            control->name=NULL;
			break;
	}
}

/*
 * prints control list to stdout
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
static void print_control_list(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);
	
	if(vd->list_device_controls == NULL)
	{
		printf("V4L2_CORE: WARNING empty control list\n");
		return;
	}

	int i = 0;
	v4l2_ctrl_t *current = vd->list_device_controls;

    for(; current != NULL; current = current->next)
    {
        print_control(current, i);
        i++;
    }
}


/*
 * gets the logitech peripheral (pan/tilt) V3 xu control unit id, if any
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   none
 *
 * returns: unit id or 0 if none
 */
static uint8_t get_logitech_peripheral_unit_id (v4l2_dev_t *vd)
{
	if(verbosity > 1)
		printf("V4L2_CORE: checking for logitech peripheral unit id\n");
	
	uint8_t guid[16] =  GUID_LOGITECH_PERIPHERAL_XU;	
	return get_guid_unit_id (vd, guid);
}

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
void v4l2_subscribe_control_events(v4l2_dev_t *vd, unsigned int control_id)
{
	vd->evsub.type = V4L2_EVENT_CTRL;
	vd->evsub.id = control_id;

	int ret = xioctl(vd->fd, VIDIOC_SUBSCRIBE_EVENT, &vd->evsub);

	if(ret)
		fprintf(stderr, "V4L2_CORE: failed to subscribe events for control 0x%08x: %s\n",
			control_id, strerror(errno));
}

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
void v4l2_unsubscribe_control_events(v4l2_dev_t *vd)
{
	vd->evsub.type = V4L2_EVENT_ALL;
	vd->evsub.id = 0;

	int ret = xioctl(vd->fd, VIDIOC_UNSUBSCRIBE_EVENT, &vd->evsub);

	if(ret)
		fprintf(stderr, "V4L2_CORE: failed to unsubscribe events: %s\n",
			strerror(errno));
}

/*
 * add control to control list
 * args:
 *   vd - pointer to video device data
 *   queryctrl - pointer to v4l2_queryctrl data
 *   current - pointer to pointer of current control from control list
 *   first - pointer to pointer of first control from control list
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid
 *   queryctrl is not null
 *
 * returns: pointer to newly added control
 */
static v4l2_ctrl_t *add_control(v4l2_dev_t *vd, struct v4l2_queryctrl* queryctrl, v4l2_ctrl_t **current, v4l2_ctrl_t **first)
{
	/*assertions*/
	assert(vd != NULL);
	assert(vd->fd > 0);
	assert(queryctrl != NULL);
	int menu_entries = 0;

	v4l2_ctrl_t *control = NULL;
	struct v4l2_querymenu* menu = NULL; //menu list
	struct v4l2_querymenu* old_menu = menu; //temp menu list pointer
	
	if (queryctrl->flags & V4L2_CTRL_FLAG_DISABLED)
	{
		printf("V4L2_CORE: Control 0x%08x is disabled: remove it from control list\n", queryctrl->id);
		return NULL;
	}

	//check menu items if needed
    if(queryctrl->type == V4L2_CTRL_TYPE_MENU
		|| queryctrl->type == V4L2_CTRL_TYPE_INTEGER_MENU )
    {
        int i = 0;
        struct v4l2_querymenu querymenu={0};

        for (querymenu.index = (__u32)queryctrl->minimum;
            querymenu.index <= (__u32)queryctrl->maximum;
            querymenu.index++)
        {
            querymenu.id = queryctrl->id;
            if (xioctl (vd->fd, (int)VIDIOC_QUERYMENU, &querymenu) < 0)
            {
                continue;
            }
	        old_menu = menu;

			if(!menu)
                menu = calloc((size_t)(i+1), sizeof(struct v4l2_querymenu));
            else
                menu = realloc(menu, (size_t)(i+1) * sizeof(struct v4l2_querymenu));

            if(menu == NULL)
			{
				/*since we exit on failure there was no need to free any previous */
				/* menu allocation (realloc), but silence cppcheck anyway */
				if(old_menu)
					free(old_menu);

				fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (add_control): %s\n", strerror(errno));
				exit(-1);
			}

            memcpy(&(menu[i]), &querymenu, sizeof(struct v4l2_querymenu));
            i++;
        }

		old_menu = menu;

		/*last entry (NULL name)*/
        if(!menu)
            menu = calloc((size_t)(i+1), sizeof(struct v4l2_querymenu));
        else
            menu = realloc(menu, (size_t)(i+1) * sizeof(struct v4l2_querymenu));

		if(menu == NULL)
		{
			/*since we exit on failure there was no need to free any previous */
			/* menu allocation (realloc), but silence cppcheck anyway */
			if(old_menu)
				free(old_menu);

			fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (add_control): %s\n", strerror(errno));
			exit(-1);
		}

        menu[i].id = querymenu.id;
        menu[i].index = (__u32)queryctrl->maximum+1;
        if(queryctrl->type == V4L2_CTRL_TYPE_MENU)
			menu[i].name[0] = 0;
		menu_entries = i;
    }

    /*check for focus control to enable software autofocus*/
    if(queryctrl->id == V4L2_CID_FOCUS_LOGITECH ||
       queryctrl->id == V4L2_CID_FOCUS_ABSOLUTE)
        vd->has_focus_control_id = (int)queryctrl->id;
	/*check for pan/tilt control*/
	else if(queryctrl->id == V4L2_CID_TILT_RELATIVE ||
			queryctrl->id == V4L2_CID_PAN_RELATIVE)
	{
		/*get unit id for logitech pan_tilt V3 if any*/
		vd->has_pantilt_control_id = 1;
		vd->pantilt_unit_id = get_logitech_peripheral_unit_id(vd);
	}
    // Add the control to the linked list
    control = calloc (1, sizeof(v4l2_ctrl_t));
    if(control == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (add_control): %s\n", strerror(errno));
		exit(-1);
	}
    memcpy(&(control->control), queryctrl, sizeof(struct v4l2_queryctrl));
    control->cclass = V4L2_CTRL_ID2CLASS(control->control.id);
    control->name = dgettext(GETTEXT_PACKAGE_V4L2CORE, (char *) control->control.name);
    //add the menu adress (NULL if not a menu)
    control->menu = menu;
    if(control->menu != NULL && control->control.type == V4L2_CTRL_TYPE_MENU)
    {
		int i = 0;
        control->menu_entry = calloc((size_t)(menu_entries), sizeof(char *));
		if(control->menu_entry == NULL)
		{
			fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (add_control): %s\n", strerror(errno));
			exit(-1);
		}
		for(i = 0; i< menu_entries; i++)
			control->menu_entry[i] = strdup(dgettext(GETTEXT_PACKAGE_V4L2CORE, (char *) control->menu[i].name));
		control->menu_entries = menu_entries;
	}
	else
	{
		control->menu_entries = 0;
		control->menu_entry = NULL;
	}
    //allocate a string with max size if needed
    if(control->control.type == V4L2_CTRL_TYPE_STRING)
    {
        control->string = (char *) calloc ((size_t)control->control.maximum + 1, sizeof(char));
        if(control->string == NULL)
		{
			fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (add_control): %s\n", strerror(errno));
			exit(-1);
		}
    }
    else
        control->string = NULL;

    if(*first != NULL)
    {
        (*current)->next = control;
        *current = control;
    }
    else
    {
		*first = control;
        *current = *first;
    }

	//subscribe control events
	v4l2_subscribe_control_events(vd, queryctrl->id);

    return control;
}

/*
 * enumerate device (read/write) controls
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
int enumerate_v4l2_control(v4l2_dev_t *vd)
{
    /*assertions*/
    assert(vd != NULL);
    assert(vd->fd > 0);
    assert(vd->list_device_controls == NULL);

    int ret=0;
    v4l2_ctrl_t *current = NULL;

    int n = 0;
    struct v4l2_queryctrl queryctrl={0};

    int currentctrl = 0;
    queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

	/*try the next_flag method first*/
	while ((ret=query_ioctl(vd, currentctrl, &queryctrl)) == 0)
	{
		if(add_control(vd, &queryctrl, &current, &(vd->list_device_controls)) != NULL)
            n++;

        currentctrl = (int)queryctrl.id;

		queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	if (queryctrl.id != V4L2_CTRL_FLAG_NEXT_CTRL)
	{
		vd->num_controls = n;
		if(verbosity > 0)
			print_control_list(vd);
		return E_OK;
	}

	if(ret)
		fprintf(stderr, "V4L2_CORE: Control 0x%08x failed to query with error %i\n", queryctrl.id, ret);

	printf("buggy V4L2_CTRL_FLAG_NEXT_CTRL flag implementation (workaround enabled)\n");

	/*
	 * next_flag method failed, loop through the ids:
	 *
	 * USER CLASS Controls
	 */
	for (currentctrl = V4L2_CID_USER_BASE; currentctrl < V4L2_CID_LASTP1; currentctrl++)
	{
        queryctrl.id = (__u32)currentctrl;
        if (xioctl(vd->fd, (int)VIDIOC_QUERYCTRL, &queryctrl) == 0)
		{
			if(add_control(vd, &queryctrl, &current, &(vd->list_device_controls)) != NULL)
				n++;
		}
	}
	/* CAMERA CLASS Controls */
	for (currentctrl = V4L2_CID_CAMERA_CLASS_BASE; currentctrl < V4L2_CID_CAMERA_CLASS_BASE+32; currentctrl++)
	{
        queryctrl.id = (__u32)currentctrl;
        if (xioctl(vd->fd, (int)VIDIOC_QUERYCTRL, &queryctrl) == 0)
		{
			if(add_control(vd, &queryctrl, &current, &(vd->list_device_controls)) != NULL)
				n++;
		}
	}
	/* PRIVATE controls (deprecated) */
	for (queryctrl.id = V4L2_CID_PRIVATE_BASE;
         xioctl(vd->fd, (int)VIDIOC_QUERYCTRL, &queryctrl) == 0; queryctrl.id++)
	{
		if(add_control(vd, &queryctrl, &current, &(vd->list_device_controls)) != NULL)
            n++;
	}

    vd->num_controls = n;

    if(verbosity > 0)
		print_control_list(vd);

	return E_OK;
}

/*
 * update the control flags - called when setting controls
 * FIXME: use control events
 *
 * args:
 *   vd - pointer to video device data
 *   id - control id
 *
 * asserts:
 *   vd is not null
 *
 * returns: error code
 */
static void update_ctrl_flags(v4l2_dev_t *vd, int id)
{
	/*asserts*/
	assert(vd != NULL);

    switch (id)
    {
        case V4L2_CID_EXPOSURE_AUTO:
            {
                v4l2_ctrl_t *ctrl_this = v4l2core_get_control_by_id(vd, id);
                if(ctrl_this == NULL)
                    break;

                switch (ctrl_this->value)
                {
                    case V4L2_EXPOSURE_AUTO:
                        {
                            v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
								vd, V4L2_CID_IRIS_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;

                            ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_IRIS_RELATIVE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                            ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                        }
                        break;

                    case V4L2_EXPOSURE_APERTURE_PRIORITY:
                        {
                            v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                            ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_IRIS_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                            ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_IRIS_RELATIVE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        }
                        break;

                    case V4L2_EXPOSURE_SHUTTER_PRIORITY:
                        {
                            v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_IRIS_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;

                            ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_IRIS_RELATIVE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                            ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        }
                        break;

                    default:
                        {
                            v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                            ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_IRIS_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                            ctrl_that = v4l2core_get_control_by_id(
                                vd, V4L2_CID_IRIS_RELATIVE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        }
                        break;
                }
            }
            break;

        case V4L2_CID_FOCUS_AUTO:
            {
                v4l2_ctrl_t *ctrl_this = v4l2core_get_control_by_id(vd, id);
                if(ctrl_this == NULL)
                    break;
                if(ctrl_this->value > 0)
                {
                    v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_FOCUS_ABSOLUTE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;

                    ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_FOCUS_RELATIVE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                }
                else
                {
                    v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_FOCUS_ABSOLUTE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);

                    ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_FOCUS_RELATIVE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                }
            }
            break;

        case V4L2_CID_HUE_AUTO:
            {
                v4l2_ctrl_t *ctrl_this = v4l2core_get_control_by_id(vd, id);
                if(ctrl_this == NULL)
                    break;
                if(ctrl_this->value > 0)
                {
                    v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_HUE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                }
                else
                {
                    v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_HUE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                }
            }
            break;

        case V4L2_CID_AUTO_WHITE_BALANCE:
            {
                v4l2_ctrl_t *ctrl_this = v4l2core_get_control_by_id(vd, id);
                if(ctrl_this == NULL)
                    break;

                if(ctrl_this->value > 0)
                {
                    v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_WHITE_BALANCE_TEMPERATURE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                    ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_BLUE_BALANCE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                    ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_RED_BALANCE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                }
                else
                {
                    v4l2_ctrl_t *ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_WHITE_BALANCE_TEMPERATURE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                    ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_BLUE_BALANCE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                    ctrl_that = v4l2core_get_control_by_id(
                        vd, V4L2_CID_RED_BALANCE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                }
            }
            break;
    }
}

/*
 * update flags of entire control list
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *
 * returns: void
 */
static void update_ctrl_list_flags(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

    v4l2_ctrl_t *current = vd->list_device_controls;

    for(; current != NULL; current = current->next)
        update_ctrl_flags(vd, (int)current->control.id);
}

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
void disable_special_auto (v4l2_dev_t *vd, int id)
{
	/*asserts*/
	assert(vd != NULL);

    v4l2_ctrl_t *current = v4l2core_get_control_by_id(vd, id);
    if(current && ((id == V4L2_CID_FOCUS_AUTO) || (id == V4L2_CID_HUE_AUTO)))
    {
        current->value = 0;
        v4l2core_set_control_value_by_id(vd, id);
    }
}

/*
 * goes trough the control list and updates/retrieves current values
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid
 *
 * returns: void
 */
void get_v4l2_control_values (v4l2_dev_t *vd)
{
    /*asserts*/
    assert(vd != NULL);
    assert(vd->fd > 0);

    if(vd->list_device_controls == NULL)
	{
		printf("V4L2_CORE: (get control values) empty control list\n");
		return;
	}

    int ret = 0;

    struct v4l2_ext_control clist[vd->num_controls];
    v4l2_ctrl_t *current = vd->list_device_controls;

    int count = 0;
    int i = 0;

    for(; current != NULL; current = current->next)
    {
        if(current->control.flags & V4L2_CTRL_FLAG_WRITE_ONLY)
             continue;

        clist[count].id = current->control.id;
        clist[count].size = 0;
        if(current->control.type == V4L2_CTRL_TYPE_STRING)
        {
            clist[count].size = (__u32)current->control.maximum + 1;
            clist[count].string = (char *) calloc(clist[count].size,  sizeof(char));
            if(clist[count].string == NULL)
			{
				fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (get_v4l2_control_values): %s\n", strerror(errno));
				exit(-1);
			}
        }
        count++;

        if((current->next == NULL) || (current->next->cclass != current->cclass))
        {
            struct v4l2_ext_controls ctrls /*= {0}*/;
            ctrls.ctrl_class = (__u32)current->cclass;
            ctrls.count = (__u32)count;
            ctrls.controls = clist;
            ret = xioctl(vd->fd, (int)VIDIOC_G_EXT_CTRLS, &ctrls);
            if(ret)
            {
                fprintf(stderr, "V4L2_CORE: (VIDIOC_G_EXT_CTRLS) failed\n");
                struct v4l2_control ctrl;
                /*get the controls one by one*/
                if( current->cclass == V4L2_CTRL_CLASS_USER
					&& current->control.type != V4L2_CTRL_TYPE_STRING
					&& current->control.type != V4L2_CTRL_TYPE_INTEGER64)
                {
                    fprintf(stderr, "V4L2_CORE: using VIDIOC_G_CTRL for user class controls\n");
                    for(i=0; i < count; i++)
                    {
                        ctrl.id = clist[i].id;
                        ctrl.value = 0;
                        ret = xioctl(vd->fd, (int)VIDIOC_G_CTRL, &ctrl);
                        if(ret)
                            continue;
                        clist[i].value = ctrl.value;
                    }
                }
                else
                {
                    fprintf(stderr, "V4L2_CORE: using VIDIOC_G_EXT_CTRLS on single controls for class: 0x%08x\n",
                        current->cclass);
                    for(i=0;i < count; i++)
                    {
                        ctrls.count = 1;
                        ctrls.controls = &clist[i];
                        ret = xioctl(vd->fd, (int)VIDIOC_G_EXT_CTRLS, &ctrls);
                        if(ret)
                            fprintf(stderr, "V4L2_CORE: control id: 0x%08x failed to get (error %i)\n",
                                clist[i].id, ret);
                    }
                }
            }

            //fill in the values on the control list
            for(i=0; i<count; i++)
            {
                v4l2_ctrl_t *ctrl = v4l2core_get_control_by_id(vd, (int)clist[i].id);
                if(!ctrl)
                {
                    fprintf(stderr, "V4L2_CORE: couldn't get control for id: %i\n", clist[i].id);
                    continue;
                }
                switch(ctrl->control.type)
                {

                    case V4L2_CTRL_TYPE_STRING:
                    {
                        /*
                         * string gets set on VIDIOC_G_EXT_CTRLS
                         * add the maximum size to value
                         */
                        unsigned len =(unsigned) strlen(clist[i].string);
                        unsigned max_len =(unsigned) ctrl->control.maximum;

						strncpy(ctrl->string, clist[i].string, max_len + 1);
						if(len > max_len)
						{
							ctrl->string[max_len + 1] = 0; //Null terminated
                            fprintf(stderr, "V4L2_CORE: control (0x%08x) returned string size of %u when max is %u\n",
								ctrl->control.id, len, max_len);
						}

						/*clean up*/
						free(clist[i].string);
						clist[i].string = NULL;
                        break;
                    }
                    case V4L2_CTRL_TYPE_INTEGER64:
                        ctrl->value64 = clist[i].value64;
                        break;
                    default:
                        ctrl->value = clist[i].value;
                        //printf("V4L2_CORE: control %i [0x%08x] = %i\n",
                        //    i, clist[i].id, clist[i].value);
                        break;
                }
            }

            count = 0;
        }
    }

    update_ctrl_list_flags(vd);
}

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
v4l2_ctrl_t *get_control_by_id(v4l2_dev_t *vd, int id)
{
	/*asserts*/
	assert(vd != NULL);
	
	v4l2_ctrl_t *current = vd->list_device_controls;
    for(; current != NULL; current = current->next)
    {
//		if(current == NULL)
//			break;

        if(current->control.id == (__u32)id)
            return (current);
    }

    return(NULL);
}

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
int get_control_value_by_id (v4l2_dev_t *vd, int id)
{
	/*asserts*/
	assert(vd != NULL);
	assert(vd->fd > 0);

    v4l2_ctrl_t *control = v4l2core_get_control_by_id(vd, id);
    int ret = 0;

    if(!control)
        return (-1);
    if(control->control.flags & V4L2_CTRL_FLAG_WRITE_ONLY)
        return (-1);

    if( control->cclass == V4L2_CTRL_CLASS_USER
		&& control->control.type != V4L2_CTRL_TYPE_STRING
		&& control->control.type != V4L2_CTRL_TYPE_INTEGER64)
    {
        struct v4l2_control ctrl;
        ctrl.id = control->control.id;
        ctrl.value = 0;
        ret = xioctl(vd->fd, (int)VIDIOC_G_CTRL, &ctrl);
        if(ret)
            fprintf(stderr, "V4L2_CORE: control id: 0x%08x failed to get value (error %i)\n",
                ctrl.id, ret);
        else
            control->value = ctrl.value;
    }
    else
    {
        struct v4l2_ext_controls ctrls /*= {0}*/;
        struct v4l2_ext_control ctrl = {0};
        ctrl.id = control->control.id;
        ctrl.size = 0;
        if(control->control.type == V4L2_CTRL_TYPE_STRING)
        {
            ctrl.size = (__u32)control->control.maximum + 1;
            ctrl.string = (char *) calloc(ctrl.size, sizeof(char));
            if(ctrl.string == NULL)
			{
				fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (v4l2core_get_control_value_by_id): %s\n", strerror(errno));
				exit(-1);
			}
        }
        ctrls.ctrl_class = (__u32)control->cclass;
        ctrls.count = 1;
        ctrls.controls = &ctrl;
        ret = xioctl(vd->fd, (int)VIDIOC_G_EXT_CTRLS, &ctrls);
        if(ret)
            printf("control id: 0x%08x failed to get value (error %i)\n",
                ctrl.id, ret);
        else
        {
            switch(control->control.type)
            {
                case V4L2_CTRL_TYPE_STRING:
				{
					strncpy(control->string, ctrl.string, ctrl.size);

					//clean up
					free(ctrl.string);
					ctrl.string = NULL;

					break;
				}
                case V4L2_CTRL_TYPE_INTEGER64:
                    control->value64 = ctrl.value64;
                    break;

                default:
                    control->value = ctrl.value;
                    //printf("V4L2_CORE: control %i [0x%08x] = %i\n",
                    //    i, clist[i].id, clist[i].value);
                    break;
            }
        }
    }

    update_ctrl_flags(vd, id);

    return (ret);
}

/*
 * goes trough the control list and sets values in device
 * args:
 *   vd - pointer to video device data
 *
 * asserts:
 *   vd is not null
 *   vd->fd is valid
 *
 * returns: void
 */
void set_v4l2_control_values (v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);
	assert(vd->fd > 0);
	
	if(vd->list_device_controls == NULL)
	{
		printf("V4L2_CORE: (set control values) empty control list\n");
		return;
	}

    int ret = 0;
    struct v4l2_ext_control clist[vd->num_controls];
    v4l2_ctrl_t *current = vd->list_device_controls;

    int count = 0;
    int i = 0;

	if(verbosity > 0)
		printf("V4L2_CORE: setting control values\n");

    for(; current != NULL; current = current->next)
    {
        if(current->control.flags & V4L2_CTRL_FLAG_READ_ONLY)
            continue;

        clist[count].id = current->control.id;
        switch (current->control.type)
        {
            case V4L2_CTRL_TYPE_STRING:
            {
                unsigned len = (unsigned)strlen(current->string);
                unsigned max_len = (unsigned)current->control.maximum;

				if(len > max_len)
				{
					clist[count].size = max_len + 1;
					clist[count].string = (char *) calloc(max_len + 1, sizeof(char));
					if(clist[count].string == NULL)
					{
						fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (set_v4l2_control_values): %s\n", strerror(errno));
						exit(-1);
					}
					clist[count].string = strncpy(clist[count].string, current->string, max_len);
					clist[count].string[max_len + 1] = 0; /*NULL terminated*/
                    fprintf(stderr, "V4L2_CORE: control (0x%08x) trying to set string size of %u when max is %u (clip)\n",
						current->control.id, len, max_len);
				}
				else
				{
					clist[count].size = len + 1;
					clist[count].string = (char *) strdup(current->string);
				}
                break;
            }
            case V4L2_CTRL_TYPE_INTEGER64:
                clist[count].value64 = current->value64;
                break;
            default:
				if(verbosity > 0)
					printf("\tcontrol[%i] = %i\n", count, current->value);
                clist[count].value = current->value;
                break;
        }
        count++;

        if((current->next == NULL) || (current->next->cclass != current->cclass))
        {
            struct v4l2_ext_controls ctrls /*= {0}*/;
            ctrls.ctrl_class = (__u32)current->cclass;
            ctrls.count = (__u32)count;
            ctrls.controls = clist;
            ret = xioctl(vd->fd, (int)VIDIOC_S_EXT_CTRLS, &ctrls);
            if(ret)
            {
                fprintf(stderr, "V4L2_CORE: VIDIOC_S_EXT_CTRLS for multiple controls failed (error %i)\n", ret);
                struct v4l2_control ctrl;
                /*set the controls one by one*/
                if( current->cclass == V4L2_CTRL_CLASS_USER
					&& current->control.type != V4L2_CTRL_TYPE_STRING
					&& current->control.type != V4L2_CTRL_TYPE_INTEGER64)
                {
                    fprintf(stderr, "V4L2_CORE: using VIDIOC_S_CTRL for user class controls\n");
                    for(i=0;i < count; i++)
                    {
                        ctrl.id = clist[i].id;
                        ctrl.value = clist[i].value;
                        ret = xioctl(vd->fd, (int)VIDIOC_S_CTRL, &ctrl);
                        if(ret)
                        {
                            v4l2_ctrl_t *ctrl = v4l2core_get_control_by_id(vd, (int)clist[i].id);
                            if(ctrl)
                                fprintf(stderr, "V4L2_CORE: control(0x%08x) \"%s\" failed to set (error %i)\n",
                                    clist[i].id, ctrl->control.name, ret);
                            else
								fprintf(stderr, "V4L2_CORE: control(0x%08x) failed to set (error %i)\n",
                                    clist[i].id, ret);
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "V4L2_CORE: using VIDIOC_S_EXT_CTRLS on single controls for class: 0x%08x\n",
                        current->cclass);
                    for(i=0;i < count; i++)
                    {
                        ctrls.count = 1;
                        ctrls.controls = &clist[i];
                        ret = xioctl(vd->fd, (int)VIDIOC_S_EXT_CTRLS, &ctrls);

                        v4l2_ctrl_t *ctrl = v4l2core_get_control_by_id(vd, (int)clist[i].id);

                        if(ret)
                        {
                            if(ctrl)
                                fprintf(stderr, "V4L2_CORE: control(0x%08x) \"%s\" failed to set (error %i)\n",
                                    clist[i].id, ctrl->control.name, ret);
                            else
								fprintf(stderr, "V4L2_CORE: control(0x%08x) failed to set (error %i)\n",
                                    clist[i].id, ret);
                        }
                        if(ctrl && ctrl->control.type == V4L2_CTRL_TYPE_STRING)
                        {
							free(clist[i].string); //free allocated string
							clist[i].string = NULL;
						}
                    }
                }
            }
            count = 0;
        }
    }

}

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
void set_control_defaults(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);
	
	if(vd->list_device_controls == NULL)
	{
		printf("V4L2_CORE: (set control defaults) empty control list\n");
		return;
	}

    v4l2_ctrl_t *current = vd->list_device_controls;

    //v4l2_ctrl_t *next = current->next;

	if(verbosity > 0)
		printf("V4L2_CORE: loading defaults\n");

	int i = 0;
    for(; current != NULL; current = current->next, ++i)
    {
        if(current->control.flags & V4L2_CTRL_FLAG_READ_ONLY)
            continue;


        switch (current->control.type)
        {
            case V4L2_CTRL_TYPE_STRING: /* do string controls have a default value?*/
                break;
            case V4L2_CTRL_TYPE_INTEGER64: /* do int64 controls have a default value?*/
                break;
            default:
                /*if its one of the special auto controls disable it first*/
                disable_special_auto (vd, (int)current->control.id);
                if(verbosity > 1)
					printf("\tdefault[%i] = %i\n", i, current->control.default_value);
                current->value = current->control.default_value;
                break;
        }
    }

    set_v4l2_control_values(vd);

    get_v4l2_control_values(vd);
}

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
int set_control_value_by_id(v4l2_dev_t *vd, int id)
{
	/*asserts*/
	assert(vd != NULL);
	assert(vd->fd > 0);

    v4l2_ctrl_t *control = v4l2core_get_control_by_id(vd, id);
    int ret = 0;

    if(!control)
        return (-1);
    if(control->control.flags & V4L2_CTRL_FLAG_READ_ONLY)
        return (-1);

    if((id == V4L2_CID_PAN_RELATIVE || id == V4L2_CID_TILT_RELATIVE) &&
		vd->pantilt_unit_id > 0)
	{
		/*use raw control in this case - prevents uvcvideo cache bug*/
		uint32_t pantilt = 0;
		if(id == V4L2_CID_PAN_RELATIVE)
            pantilt |= (uint32_t) control->value;
		else
            pantilt |= ((uint32_t) control->value) << 16;

		return query_xu_control(vd, vd->pantilt_unit_id, 1, UVC_SET_CUR, &pantilt);
	}



    if( control->cclass == V4L2_CTRL_CLASS_USER
    && control->control.type != V4L2_CTRL_TYPE_STRING
    && control->control.type != V4L2_CTRL_TYPE_INTEGER64)
    {
        //using VIDIOC_G_CTRL for user class controls
        struct v4l2_control ctrl;
        ctrl.id = control->control.id;
        ctrl.value = control->value;
        ret = xioctl(vd->fd, (int)VIDIOC_S_CTRL, &ctrl);
    }
    else
    {
        //using VIDIOC_G_EXT_CTRLS on single controls
        struct v4l2_ext_controls ctrls /*= {0}*/;
        struct v4l2_ext_control ctrl = {0};
        ctrl.id = control->control.id;
        switch (control->control.type)
        {
            case V4L2_CTRL_TYPE_STRING:
            {
                unsigned len = (unsigned)strlen(control->string);
                unsigned max_len = (unsigned)control->control.maximum;

				if(len > max_len)
				{
					ctrl.size = max_len + 1;
					ctrl.string = (char *) calloc(max_len + 1, sizeof(char));
					if(ctrl.string == NULL)
					{
						fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (v4l2core_set_control_value_by_id): %s\n", strerror(errno));
						exit(-1);
					}
					ctrl.string = strncpy(ctrl.string, control->string, max_len);
					ctrl.string[max_len + 1] = 0; /*NULL terminated*/
                    fprintf(stderr, "V4L2_CORE: control (0x%08x) trying to set string size of %u when max is %u (clip)\n",
						control->control.id, len, max_len);
				}
				else
				{
					ctrl.size = len + 1;
					ctrl.string = (char *) strdup(control->string);
				}
                break;
            }
            case V4L2_CTRL_TYPE_INTEGER64:
                ctrl.value64 = control->value64;
                break;
            default:
                ctrl.value = control->value;
                break;
        }
        ctrls.ctrl_class = (__u32)control->cclass;
        ctrls.count = 1;
        ctrls.controls = &ctrl;
        ret = xioctl(vd->fd, (int)VIDIOC_S_EXT_CTRLS, &ctrls);
        if(ret)
            printf("control id: 0x%08x failed to set (error %i)\n",
                ctrl.id, ret);
        if(control->control.type == V4L2_CTRL_TYPE_STRING)
        {
			free(ctrl.string); //clean up string allocation
			ctrl.string = NULL;
		}
    }

    //update real value
    get_control_value_by_id(vd, id);

    return (ret);
}

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
void free_v4l2_control_list(v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

	if(vd->list_device_controls == NULL)
	{
		return;
	}

	v4l2_ctrl_t *first = vd->list_device_controls;

    while (first != NULL)
    {
		v4l2_ctrl_t *next = first->next;

        if(first->string) free(first->string);
        if(first->menu) free(first->menu);
        if(first->menu_entry)
        {
			int i = 0;
			for(i = 0; i < first->menu_entries; i++)
				free(first->menu_entry[i]);
			free(first->menu_entry);
		}
        free(first);
        first = next;
    }
    vd->list_device_controls = NULL;

	//unsubscibe control events
	v4l2_unsubscribe_control_events(vd);
}

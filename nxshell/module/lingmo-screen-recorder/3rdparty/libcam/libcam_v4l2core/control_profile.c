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
#include <sys/types.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "gviewv4l2core.h"
#include "v4l2_controls.h"
#include "control_profile.h"
#include "cameraconfig.h"

#define PACKAGE_STRING "cheese 1.0"

extern int verbosity;

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
int save_control_profile(v4l2_dev_t *vd, const char *filename)
{
	/*assertions*/
	assert(vd != NULL);

	FILE *fp;

	fp = fopen(filename, "w");
	if( fp == NULL )
	{
		fprintf(stderr, "V4L2_CORE: (save_control_profile) Could not open %s for write: %s\n",
			filename, strerror(errno));
		return (E_FILE_IO_ERR);
	}
	else
	{
		if (vd->list_device_controls)
		{
			v4l2_ctrl_t *current = vd->list_device_controls;


			/*write header*/
			fprintf(fp, "#V4L2/CTRL/0.0.2\n");
			fprintf(fp, "APP{\"%s\"}\n", PACKAGE_STRING);
            /*write control data*/
			fprintf(fp, "# control data\n");
			for( ; current != NULL; current = current->next)
			{
				if((current->control.flags & V4L2_CTRL_FLAG_WRITE_ONLY) ||
				   (current->control.flags & V4L2_CTRL_FLAG_READ_ONLY) ||
				   (current->control.flags & V4L2_CTRL_FLAG_GRABBED))
				{
					if(verbosity > 0)
						printf("V4L2_CORE: (save_control_profile) skiping control 0x%08x\n", current->control.id);
					continue;
				}
				fprintf(fp, "#%s\n", current->control.name);
				switch(current->control.type)
				{
					case V4L2_CTRL_TYPE_STRING :
						fprintf(fp, "ID{0x%08x};CHK{%i:%i:%i:0}=STR{\"%s\"}\n",
							current->control.id,
							current->control.minimum,
							current->control.maximum,
							current->control.step,
							current->string);
						break;

					case V4L2_CTRL_TYPE_INTEGER64 :
						fprintf(fp, "ID{0x%08x};CHK{0:0:0:0}=VAL64{%" PRId64 "}\n",
							current->control.id,
							current->value64);
						break;
					default :
						fprintf(fp, "ID{0x%08x};CHK{%i:%i:%i:%i}=VAL{%i}\n",
							current->control.id,
							current->control.minimum,
							current->control.maximum,
							current->control.step,
							current->control.default_value,
							current->value);
						break;
				}
			}
		}
	}

	fflush(fp); /*flush stream buffers to filesystem*/
	if(fsync(fileno(fp)) ||	fclose(fp))
	{
		fprintf(stderr, "V4L2_CORE: (save_control_profile) write to file failed: %s\n", strerror(errno));
		return(E_FILE_IO_ERR);
	}

	return (E_OK);
}

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
int load_control_profile(v4l2_dev_t *vd, const char *filename)
{
	/*assertions*/
	assert(vd != NULL);

	FILE *fp;
	int major=0, minor=0, rev=0;

	if((fp = fopen(filename,"r"))!=NULL)
	{
		char line[200];
		if(fgets(line, sizeof(line), fp) != NULL)
		{
			if(sscanf(line,"#V4L2/CTRL/%3i.%3i.%3i", &major, &minor, &rev) == 3)
			{
                //check standard version if needed
			}
			else
			{
				fprintf(stderr, "V4L2_CORE: (load_control_profile) no valid header found\n");
				fclose(fp);
				return(E_NO_DATA);
			}
		}
		else
		{
			fprintf(stderr, "V4L2_CORE: (load_control_profile) no valid header found\n");
            fclose(fp);
			return(E_NO_DATA);
		}

		while (fgets(line, sizeof(line), fp) != NULL)
		{
			int id = 0;
			int min = 0, max = 0, step = 0, def = 0;
			int32_t val = 0;
			int64_t val64 = 0;

			if ((line[0]!='#') && (line[0]!='\n'))
			{
                if(sscanf(line,"ID{0x%08i};CHK{%5i:%5i:%5i:%5i}=VAL{%5i}",
					&id, &min, &max, &step, &def, &val) == 6)
				{
					v4l2_ctrl_t *current = v4l2core_get_control_by_id(vd, id);

					if(current)
					{
                        /*check values*/
						if(current->control.minimum == min &&
						   current->control.maximum == max &&
						   current->control.step == step &&
						   current->control.default_value == def)
						{
							current->value = val;
						}
					}
				}
				else if(sscanf(line,"ID{0x%08x};CHK{0:0:0:0}=VAL64{%" PRId64 "}",
					&id, &val64) == 2)
				{
					v4l2_ctrl_t *current = v4l2core_get_control_by_id(vd, id);

					if(current)
					{
						current->value64 = val64;
					}
				}
                else if(sscanf(line,"ID{0x%08i};CHK{%5i:%5i:%5i:0}=STR{\"%*s\"}",
					&id, &min, &max, &step) == 5)
				{
					v4l2_ctrl_t *current = v4l2core_get_control_by_id(vd, id);

					if(current)
					{
                        /*check values*/
						if(current->control.minimum == min &&
						   current->control.maximum == max &&
						   current->control.step == step)
						{
                            char str[max+1];
							char fmt[48];
							sprintf(fmt,"ID{0x%%*x};CHK{%%*i:%%*i:%%*i:0}==STR{\"%%%is\"}", max);
                            sscanf(line, fmt, str);

							/*we are only scannig for max chars so this should never happen*/
                            if(strlen(str) > (size_t)max) /*FIXME: should also check (minimum +N*step)*/
							{
                                fprintf(stderr, "V4L2_CORE: (load_control_profile) string bigger than maximum buffer size (%i > %i)\n",
									(int) strlen(str), max);
								if(current->string)
									free(current->string);
                                current->string = strndup(str, (size_t)max); /*FIXME: does max includes '\0' ?*/
							}
							else
                            {
								if(current->string)
									free(current->string);
								current->string = strndup(str, strlen(str)+1);
							}
						}
					}
				}
			}
		}

		set_v4l2_control_values(vd);
		get_v4l2_control_values(vd);
	}
    else
    {
        fprintf(stderr, "V4L2_CORE: (load_control_profile) Could not open for %s read: %s\n",
			filename, strerror(errno));
        return (E_FILE_IO_ERR);
    }

    fclose(fp);
    return (E_OK);
}

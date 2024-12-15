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
#  autofocus - using dct                                                        #
#                                                                               #
#                                                                               #
********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <assert.h>

#include "gviewv4l2core.h"
#include "soft_autofocus.h"
#include "dct.h"
#include "gview.h"
#include "core_time.h"
#include "cameraconfig.h"

#define _TH_		(80) /* default treshold = 1/80 of focus sharpness value*/

#define FLAT 		(0)
#define LOCAL_MAX	(1)
#define LEFT		(2)
#define RIGHT		(3)
#define INCSTEP		(4)

#define MAX_ARR_S 20

#define SWAP(x, y) temp = (x); (x) = (y); (y) = temp

extern int verbosity;

typedef struct _focus_ctx_t
{
	int focus;
	int step;
	int right;
	int left;
	int sharpness;
	int focus_sharpness;
	int sharpLeft;
	int sharpRight;
    v4l2_ctrl_t* focus_control;
	int f_max;
	int f_min;
	int f_step;
	int i_step;
	int focusDir;
	int arr_sharp[MAX_ARR_S];
	int arr_foc[MAX_ARR_S];
	int ind;
	int flag;
	int setFocus;
	int focus_wait;
	int last_focus;
} focus_ctx_t;

static focus_ctx_t *focus_ctx = NULL;

static double sumAC[64];
static int ACweight[64] = {
	0,1,2,3,4,5,6,7,
	1,1,2,3,4,5,6,7,
	2,2,2,3,4,5,6,7,
	3,3,3,3,4,5,6,7,
	4,4,4,4,4,5,6,7,
	5,5,5,5,5,5,6,7,
	7,7,7,7,7,7,7,7
};

/*use insert sort by default - it's the fastest for small and almost sorted arrays (our case)*/
static int sort_method = AUTOF_SORT_INSERT; /* 1 - Quick sort   2 - Shell sort  3- insert sort  other - bubble sort*/

/*
 * sets a focus loop while autofocus is on
 * args:
 *    none
 *
 * asserts:
 *    focus_ctx is not null
 *
 * returns: none
 */
void v4l2core_soft_autofocus_set_focus()
{
	/*asserts*/
	assert(focus_ctx != NULL);

	focus_ctx->setFocus = 1;

	focus_ctx->ind = 0;
	focus_ctx->flag = 0;
	focus_ctx->right = 255;
	focus_ctx->left = 8;
	focus_ctx->focus = -1; /*reset focus*/
}

/*
 * set autofocus sort method
 * args:
 *    method - sort method
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void v4l2core_soft_autofocus_set_sort(int method)
{
	sort_method = method;
}

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
int soft_autofocus_init (v4l2_dev_t *vd)
{
	/*asserts*/
	assert(vd != NULL);

	if(!vd->has_focus_control_id)
	{
		fprintf(stderr, "V4L2_CORE: can't init software autofocus - no focus control detected\n");
		return (E_UNKNOWN_CID_ERR);
	}

	if(focus_ctx != NULL)
		free(focus_ctx);

	focus_ctx = calloc(1, sizeof(focus_ctx_t));
	if(focus_ctx == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (v4l2core_soft_autofocus_init): %s\n", strerror(errno));
		exit(-1);
	}

    focus_ctx->focus_control = v4l2core_get_control_by_id(vd, vd->has_focus_control_id);

    if(focus_ctx->focus_control == NULL)
	{
		fprintf(stderr, "V4L2_CORE: couldn't load focus control for id %x\n", vd->has_focus_control_id);
		free(focus_ctx);
		focus_ctx = NULL;
		return(E_UNKNOWN_CID_ERR);
	}

	focus_ctx->f_max = focus_ctx->focus_control->control.maximum;
	focus_ctx->f_min = focus_ctx->focus_control->control.minimum;
	focus_ctx->f_step = focus_ctx->focus_control->control.step;

	focus_ctx->i_step = (focus_ctx->f_max + 1 - focus_ctx->f_min)/32;
	if(focus_ctx->i_step <= focus_ctx->f_step)
		focus_ctx->i_step = focus_ctx->f_step * 2;
	//printf("V4L2_CORE: (soft_autofocus) focus step:%i\n", focus_ctx->i_step);
	focus_ctx->right = focus_ctx->f_max;
	focus_ctx->left = focus_ctx->f_min + focus_ctx->i_step; /*start with focus at 8*/
	focus_ctx->focus = -1;
	focus_ctx->focus_wait = 0;

	focus_ctx->last_focus = focus_ctx->focus_control->value;
	/*make sure we wait for focus to settle on first check*/
	if (focus_ctx->last_focus < 0)
		focus_ctx->last_focus = focus_ctx->f_max;

	memset(sumAC, 0, 64*sizeof(*sumAC)); /*reset array to 0*/

	return (E_OK);
}

/*
 * quick sort
 * (the fastest and more complex - recursive, doesn't do well on almost sorted data)
 * args:
 *   left -
 *   right -
 *
 * asserts:
 *   focus_ctx is not null
 *
 * returns: none
 */
static void q_sort(int left, int right)
{
	/*asserts*/
	assert(focus_ctx != NULL);

	int l_hold = left;
	int r_hold = right;
	int pivot = focus_ctx->arr_sharp[left];
	int temp = focus_ctx->arr_foc[left];

	while(left < right)
	{
		while((focus_ctx->arr_sharp[right] >= pivot) && (left < right))
			right--;
		if (left != right)
		{
			focus_ctx->arr_sharp[left] = focus_ctx->arr_sharp[right];
			focus_ctx->arr_foc[left] = focus_ctx->arr_foc[right];
			left++;
		}
		while((left < right) && (focus_ctx->arr_sharp[left] <= pivot))
			left++;
		if (left != right)
		{
			focus_ctx->arr_sharp[right] = focus_ctx->arr_sharp[left];
			focus_ctx->arr_foc[right] = focus_ctx->arr_foc[left];
			right--;
		}
	}
	focus_ctx->arr_sharp[left] = pivot;
	focus_ctx->arr_foc[left] = temp;
	pivot = left;

	if (l_hold < pivot) q_sort(l_hold, pivot-1);
	if (r_hold > pivot) q_sort(pivot+1, r_hold);
}

/*
 * shell sort
 * (based on insert sort, but with some optimization)
 * for small arrays insert sort is still faster
 * args:
 *    size -
 *
 * asserts:
 *    focus_ctx is not null
 *
 * returns: none
 */
static void s_sort(int size)
{
	/*asserts*/
	assert(focus_ctx != NULL);

	int i, j, temp, gap;

	for (gap = size / 2; gap > 0; gap /= 2)
	{
		for (i = gap; i <= size; i++)
		{
			for (j = i-gap; j >= 0 && (focus_ctx->arr_sharp[j] > focus_ctx->arr_sharp[j + gap]); j -= gap)
			{
				SWAP(focus_ctx->arr_sharp[j], focus_ctx->arr_sharp[j + gap]);
				SWAP(focus_ctx->arr_foc[j], focus_ctx->arr_foc[j + gap]);
			}
		}
	}
}

/*
 * insert sort
 * (fastest for small arrays, around 15 elements)
 * args:
 *    size -
 *
 * asserts:
 *    focus_ctx is not null
 *
 * returns: none
 */
static void i_sort (int size)
{
	/*asserts*/
	assert(focus_ctx != NULL);

	int i,j,temp;

	for (i = 1; i <= size; i++)
	{
		for(j = i; j > 0 && (focus_ctx->arr_sharp[j-1] > focus_ctx->arr_sharp[j]); j--)
		{
			SWAP(focus_ctx->arr_sharp[j],focus_ctx->arr_sharp[j-1]);
			SWAP(focus_ctx->arr_foc[j],focus_ctx->arr_foc[j-1]);
		}
	}
}

/*
 * bubble sort
 * (the simplest and most inefficient - in real test with focus data
 * it did better than shell or quick sort since focus data is almost
 * sorted)
 * args:
 *    size -
 *
 * asserts:
 *    focus_ctx is not null
 *
 * returns: none
 */
static void b_sort (int size)
{
	int i, temp, swapped;

	do
	{
		swapped = 0;
		size--;
		for (i = 0 ; i <= size; ++i)
		{
			if (focus_ctx->arr_sharp[i+1] < focus_ctx->arr_sharp[i])
			{
				SWAP(focus_ctx->arr_sharp[i],focus_ctx->arr_sharp[i+1]);
				SWAP(focus_ctx->arr_foc[i],focus_ctx->arr_foc[i+1]);
				swapped = 1;
			}
		}
	} while (swapped);
}

/*
 * sort focus values
 * args:
 *    size - focus array size
 *
 * returns: best focus value
 */
static int focus_sort(int size)
{
	if (size>=20)
	{
		fprintf(stderr, "V4L2_CORE: (soft_autofocus) focus array size=%d exceeds 20\n", size);
		size = 10;
	}

	switch(sort_method)
	{
		case AUTOF_SORT_QUICK:
			q_sort(0, size);
			break;

		case AUTOF_SORT_SHELL:
			s_sort(size);
			break;

		case AUTOF_SORT_BUBBLE:
			b_sort(size);
			break;

		default:
		case AUTOF_SORT_INSERT:
			i_sort(size);
			break;
	}

	/*better focus value*/
	return(focus_ctx->arr_foc[size]);
}

/*
 * extract lum (y) data from image
 * args:
 *    frame - image frame data pointer
 *    dataY - pointer for lum (y) data
 *    width - width of image frame (in pixels)
 *    height - height of image frame (in pixels)
 *
 * asserts:
 *
 * returns: pointer to lum data buffer
 */
static int16_t *focus_extract_Y (uint8_t *frame, int16_t *dataY, int width, int height)
{
	int i = 0;
	uint8_t *pimg;
	pimg = frame;

	for (i = 0; i < (height * width); ++i)
	{
		dataY[i]=(int16_t) *pimg++; // luma
	}

	return (dataY);
}

/*
 * check focus
 * args:
 *    none
 *
 * asserts:
 *    focus_ctx is not null
 *
 * returns: focus code
 */
static int checkFocus()
{
	/*asserts*/
	assert(focus_ctx != NULL);

	/*change treshold according to sharpness*/
	int TH = _TH_;
	//if(focus_ctx->focus_sharpness < (5 * _TH_)) TH = _TH_ * 4 ;

	if (focus_ctx->step <= focus_ctx->i_step)
	{
        if ((abs(focus_ctx->sharpLeft-focus_ctx->focus_sharpness)<(focus_ctx->focus_sharpness/TH)) &&
			(abs(focus_ctx->sharpRight-focus_ctx->focus_sharpness)<(focus_ctx->focus_sharpness/TH)))
		{
			return (FLAT);
		}
		else if (((focus_ctx->focus_sharpness-focus_ctx->sharpRight))>=(focus_ctx->focus_sharpness/TH) &&
			((focus_ctx->focus_sharpness-focus_ctx->sharpLeft))>=(focus_ctx->focus_sharpness/TH))
		{
			/*
			 *  significantly down in both directions -> check another step
			 *  outside for local maximum
			 */
			focus_ctx->step=16;
			return (INCSTEP);
		}
		else
		{
			// one is significant, the other is not...
			int left=0; int right=0;
			if (abs((focus_ctx->sharpLeft-focus_ctx->focus_sharpness))>=(focus_ctx->focus_sharpness/TH))
			{
				if (focus_ctx->sharpLeft>focus_ctx->focus_sharpness) left++;
				else right++;
			}
			if (abs((focus_ctx->sharpRight-focus_ctx->focus_sharpness))>=(focus_ctx->focus_sharpness/TH))
			{
				if (focus_ctx->sharpRight>focus_ctx->focus_sharpness) right++;
				else left++;
			}
			if (left==right) return (FLAT);
			else if (left>right) return (LEFT);
			else return (RIGHT);
		}
	}
	else
	{
		if (((focus_ctx->focus_sharpness-focus_ctx->sharpRight))>=(focus_ctx->focus_sharpness/TH) &&
			((focus_ctx->focus_sharpness-focus_ctx->sharpLeft))>=(focus_ctx->focus_sharpness/TH))
		{
			return (LOCAL_MAX);
		}
		else
		{
			return (FLAT);
		}
	}
}

/*
 * measure sharpness in MCU
 * args:
 *    data - MCU data [8x8]
 *    weight - MCU weight for sharpness measure.
 *
 * asserts:
 *    none
 *
 * returns: none
 */
static void getSharpnessMCU (int16_t *data, double weight)
{

	int i=0;
	int j=0;

	levelshift (data);
	DCT (data);

	for (i=0;i<8;i++)
	{
		for(j=0;j<8;j++)
		{
			sumAC[i*8+j]+=data[i*8+j]*data[i*8+j]*weight;
		}
	}
}

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
int soft_autofocus_get_sharpness (uint8_t *frame, int width, int height, int t)
{
	float res=0;
	int numMCUx = width/(8*2); /*covers 1/2 of width - width should be even*/
	int numMCUy = height/(8*2); /*covers 1/2 of height- height should be even*/
	int16_t dataMCU[64];
	int16_t* data;
    int16_t *Y = calloc((size_t)(width * height), sizeof(int16_t));

	if(Y == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (soft_autofocus_get_sharpness): %s\n", strerror(errno));
		exit(-1);
	}

	double weight;
	double xp_;
	int ctx = numMCUx >> 1; /*center*/
	int cty = numMCUy >> 1;
	double rad=ctx/2;
	if (cty<ctx) { rad=cty/2; }
	rad=rad*rad;
	int cnt2 =0;

	data=dataMCU;

	focus_extract_Y (frame, Y, width, height);

	int i=0;
	int j=0;
	int xp=0;
	int yp=0;
	/*calculate MCU sharpness*/
	for (yp=0;yp<numMCUy;yp++)
	{
		double yp_=yp-cty;
		for (xp=0;xp<numMCUx;xp++)
		{
			xp_=xp-ctx;
			weight = exp(-(xp_*xp_)/rad-(yp_*yp_)/rad);
			for (i=0;i<8;i++)
			{
				for(j=0;j<8;j++)
				{
					/*center*/
					dataMCU[i*8+j]=Y[(((height-(numMCUy-(yp*2))*8)>>1)+i)*width
						+(((width-(numMCUx-(xp*2))*8)>>1)+j)];
				}
			}
			getSharpnessMCU(data,weight);
			cnt2++;
		}
	}

	free(Y);

	for (i=0;i<=t;i++)
	{
		for(j=0;j<t;j++)
		{
			sumAC[i*8+j]/=(double) (cnt2); /*average = mean*/
            res+=(float)sumAC[i*8+j]*ACweight[i*8+j];
		}
	}
    return (int)(roundf(res*10)); /*round to int (4 digit precision)*/
}

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
int soft_autofocus_get_focus_value()
{
	int step = focus_ctx->i_step * 2;
	int step2 = focus_ctx->i_step / 2;
	if (step2 <= 0 ) step2 = 1;
	int focus=0;
//LMH0612从switch里面放出来
    /*--------- first time - run sharpness algorithm -----------------*/
    if(focus_ctx->ind >= 20)
    {
        fprintf (stderr, "V4L2_CORE: (soft_autofocus) ind=%d exceeds 20\n", focus_ctx->ind);
        focus_ctx->ind = 10;
    }

	switch (focus_ctx->flag)
	{


		case 0: /*sample left to right at higher step*/
			focus_ctx->arr_sharp[focus_ctx->ind] = focus_ctx->sharpness;
			focus_ctx->arr_foc[focus_ctx->ind] = focus_ctx->focus;
			/*reached max focus value*/
			if (focus_ctx->focus >= focus_ctx->right )
			{	/*get left and right from arr_sharp*/
				focus = focus_sort(focus_ctx->ind);
				/*get a window around the best value*/
				focus_ctx->left = (focus- step/2);
				focus_ctx->right = (focus + step/2);
				if (focus_ctx->left < focus_ctx->f_min) focus_ctx->left = focus_ctx->f_min;
				if (focus_ctx->right > focus_ctx->f_max) focus_ctx->right = focus_ctx->f_max;
				focus_ctx->focus = focus_ctx->left;
				focus_ctx->ind = 0;
				focus_ctx->flag = 1;
			}
			else /*increment focus*/
			{
				focus_ctx->focus=focus_ctx->arr_foc[focus_ctx->ind] + step; /*next focus*/
				focus_ctx->ind++;
				focus_ctx->flag = 0;
			}
			break;

		case 1: /*sample left to right at lower step - fine tune*/
			focus_ctx->arr_sharp[focus_ctx->ind] = focus_ctx->sharpness;
			focus_ctx->arr_foc[focus_ctx->ind] = focus_ctx->focus;
			/*reached window max focus*/
			if (focus_ctx->focus >= focus_ctx->right )
			{	/*get left and right from arr_sharp*/
				focus = focus_sort(focus_ctx->ind);
				/*get the best value*/
				focus_ctx->focus = focus;
				focus_ctx->focus_sharpness = focus_ctx->arr_sharp[focus_ctx->ind];
				focus_ctx->step = focus_ctx->i_step; /*first step for focus tracking*/
				focus_ctx->focusDir = FLAT; /*no direction for focus*/
				focus_ctx->flag = 2;
			}
			else /*increment focus*/
			{
				focus_ctx->focus=focus_ctx->arr_foc[focus_ctx->ind] + step2; /*next focus*/
				focus_ctx->ind++;
				focus_ctx->flag = 1;
			}
			break;

		case 2: /* set treshold in order to sharpness*/
			if (focus_ctx->setFocus)
			{
				/*reset*/
				focus_ctx->setFocus = 0;
				focus_ctx->flag= 0;
				focus_ctx->right = focus_ctx->f_max;
				focus_ctx->left = focus_ctx->f_min + focus_ctx->i_step;
				focus_ctx->ind = 0;
			}
			else
			{
				/*track focus*/
				focus_ctx->focus_sharpness = focus_ctx->sharpness;
				focus_ctx->flag = 3;
				focus_ctx->sharpLeft = 0;
				focus_ctx->sharpRight = 0;
				focus_ctx->focus += focus_ctx->step; /*check right*/
			}
			break;

		case 3:
			/*track focus*/
			focus_ctx->flag = 4;
			focus_ctx->sharpRight = focus_ctx->sharpness;
			focus_ctx->focus -= (2*focus_ctx->step); /*check left*/
			break;

		case 4:
			/*track focus*/
			focus_ctx->sharpLeft=focus_ctx->sharpness;
			int ret=0;
			ret = checkFocus();

			switch (ret)
			{
				case LOCAL_MAX:
					focus_ctx->focus += focus_ctx->step; /*return to orig. focus*/
					focus_ctx->step = focus_ctx->i_step;
					focus_ctx->flag = 2;
					break;

				case FLAT:
					if(focus_ctx->focusDir == FLAT)
					{
						focus_ctx->step = focus_ctx->i_step;
						if(focus_ctx->focus_sharpness < 4 * _TH_)
						{
							/* 99% chance we lost focus     */
							/* move focus to half the range */
							focus_ctx->focus = focus_ctx->f_max / 2;
						}
						else
						{
							focus_ctx->focus += focus_ctx->step; /*return to orig. focus*/
						}
						focus_ctx->flag = 2;
					}
					else if (focus_ctx->focusDir == RIGHT)
					{
						focus_ctx->focus += 2*focus_ctx->step; /*go right*/
						focus_ctx->step = focus_ctx->i_step;
						focus_ctx->flag = 2;
					}
					else
					{	/*go left*/
						focus_ctx->step = focus_ctx->i_step;
						focus_ctx->flag = 2;
					}
					break;

				case RIGHT:
					focus_ctx->focus += 2*focus_ctx->step; /*go right*/
					focus_ctx->flag = 2;
					break;

				case LEFT:
					/*keep focus on left*/
					focus_ctx->flag = 2;
					break;

				case INCSTEP:
					focus_ctx->focus += focus_ctx->step; /*return to orig. focus*/
					focus_ctx->step = 2 * focus_ctx->i_step;
					focus_ctx->flag = 2;
					break;
			}
			break;
	}
	/*clip focus, right and left*/
	focus_ctx->focus=(focus_ctx->focus > focus_ctx->f_max) ? focus_ctx->f_max : ((focus_ctx->focus < focus_ctx->f_min) ? focus_ctx->f_min : focus_ctx->focus);
	focus_ctx->right=(focus_ctx->right > focus_ctx->f_max) ? focus_ctx->f_max : ((focus_ctx->right < focus_ctx->f_min) ? focus_ctx->f_min : focus_ctx->right);
	focus_ctx->left =(focus_ctx->left > focus_ctx->f_max) ? focus_ctx->f_max : ((focus_ctx->left < focus_ctx->f_min) ? focus_ctx->f_min : focus_ctx->left);

	return focus_ctx->focus;
}

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
int soft_autofocus_run(v4l2_dev_t *vd, v4l2_frame_buff_t *frame)
{
	/*asserts*/
	assert(vd != NULL);

	if (focus_ctx->focus < 0)
	{
		/*starting autofocus*/
		focus_ctx->focus = focus_ctx->left; /*start left*/

		focus_ctx->focus_control->value = focus_ctx->focus;
        if (v4l2core_set_control_value_by_id(vd, (int)focus_ctx->focus_control->control.id) != 0)
			fprintf(stderr, "V4L2_CORE: (sof_autofocus) couldn't set focus to %d\n", focus_ctx->focus);

		/*number of frames until focus is stable*/
		/*1.4 ms focus time - every 1 step*/
        focus_ctx->focus_wait = (int) abs(focus_ctx->focus - focus_ctx->last_focus)*1.4/((1000*vd->fps_num)/vd->fps_denom)+1;
        focus_ctx->last_focus = focus_ctx->focus;
	}
	else
	{
		if (focus_ctx->focus_wait == 0)
		{
			focus_ctx->sharpness = soft_autofocus_get_sharpness (
				frame->yuv_frame,
				vd->format.fmt.pix.width,
				vd->format.fmt.pix.height,
				5);

			if (verbosity > 1)
				printf("V4L2_CORE: (sof_autofocus) sharp=%d focus_sharp=%d foc=%d right=%d left=%d ind=%d flag=%d\n",
					focus_ctx->sharpness,
					focus_ctx->focus_sharpness,
					focus_ctx->focus,
					focus_ctx->right,
					focus_ctx->left,
					focus_ctx->ind,
					focus_ctx->flag);

			focus_ctx->focus = soft_autofocus_get_focus_value();

			if ((focus_ctx->focus != focus_ctx->last_focus))
			{
				focus_ctx->focus_control->value = focus_ctx->focus;
				if (v4l2core_set_control_value_by_id(vd, focus_ctx->focus_control->control.id) != 0)
					fprintf(stderr, "V4L2_CORE: (sof_autofocus) couldn't set focus to %d\n",
						focus_ctx->focus);

				/*number of frames until focus is stable*/
				/*1.4 ms focus time - every 1 step*/
				focus_ctx->focus_wait = (int) abs(focus_ctx->focus - focus_ctx->last_focus)*1.4/((1000*vd->fps_num)/vd->fps_denom)+1;
			}
			focus_ctx->last_focus = focus_ctx->focus;
		}
		else
		{
			focus_ctx->focus_wait--;
			if (verbosity > 1)
				printf("V4L2_CORE: (soft_autofocus) Wait Frame: %d\n",
					focus_ctx->focus_wait);
		}
	}

	return (focus_ctx->setFocus);
}

/*
 * close and clean software autofocus
 * args:
 *   none
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void v4l2core_soft_autofocus_close()
{
	if(focus_ctx != NULL)
		free(focus_ctx);
	focus_ctx = NULL;
}

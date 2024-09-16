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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/* support for internationalization - i18n */
#include <locale.h>
#include <libintl.h>

#include "cameraconfig.h"
#include "gviewencoder.h"
#include "encoder.h"
#include "stream_io.h"
#include "gview.h"

/*
 * get last stream of the list (list tail)
 * args:
 *   stream_list - pointer to stream list
 *
 * asserts:
 *   none
 *
 * returns: pointer to last stream of the list
 *          or NULL if none
 */
stream_io_t *get_last_stream(stream_io_t *stream_list)
{
	stream_io_t *stream = stream_list;

	if(!stream)
		return NULL;

	while(stream->next != NULL)
		stream = stream->next;

	return stream;
}

/*
 * add a new stream to the list
 * args:
 *   stream_list - pointer to pointer of stream_list
 *   list_size - pointer to list size
 *
 * asserts:
 *   none
 *
 * returns: pointer to newly allocated stream
 */
stream_io_t *add_new_stream(stream_io_t **stream_list, int *list_size)
{


	stream_io_t *stream = calloc(1, sizeof(stream_io_t));
	if (stream == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (add_new_stream): %s\n", strerror(errno));
		exit(-1);
	}
	stream->next = NULL;
	stream->id = *list_size;

	fprintf(stderr, "ENCODER: add stream %i to stream list\n", stream->id);

	stream_io_t *last_stream = get_last_stream(*stream_list);
	stream->previous = last_stream;
	if(last_stream)
		last_stream->next = stream;
	else
		*stream_list = stream; /*first stream*/


	stream->indexes = NULL;

	*list_size = *list_size + 1;

	return(stream);
}

/*
 * destroy the sream list (free all streams)
 * args:
 *   stream_list - pointer to stream list
 *   list_size - pointer to list size
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void destroy_stream_list(stream_io_t *stream_list, int *list_size)
{
	stream_io_t *stream = get_last_stream(stream_list);
	while(stream != NULL) //from end to start
	{
		stream_io_t *prev_stream = stream->previous;
		if(stream->indexes != NULL)
			free(stream->indexes);
		free(stream);
		stream = prev_stream;
		*list_size = *list_size - 1;
	}
}

/*
 * get stream with index from list
 * args:
 *   stream_list - pointer to pointer of stream_list
 *   index - stream index in the list
 *
 * asserts:
 *   none
 *
 * returns: pointer to stream
 */
stream_io_t *get_stream(stream_io_t *stream_list, int index)
{
	stream_io_t *stream = stream_list;

	if(!stream)
		return NULL;

	int j = 0;

	while(stream->next != NULL && (j < index))
	{
		stream = stream->next;
		j++;
	}

	if(j != index)
		return NULL;

	return stream;
}

/*
 * get first video stream
 * args:
 *   stream_list - pointer to  stream list
 *
 * asserts:
 *   none
 *
 * returns: pointer to stream
 */
stream_io_t *get_first_video_stream(stream_io_t *stream_list)
{
	stream_io_t *stream = stream_list;

	while(stream != NULL)
	{
		if(stream->type == STREAM_TYPE_VIDEO)
			return stream;

		stream = stream->next;
	}

	return NULL;
}

/*
 * get first audio stream
 * args:
 *   stream_list - pointer to  stream list
 *
 * asserts:
 *   none
 *
 * returns: pointer to stream
 */
//stream_io_t *get_first_audio_stream(stream_io_t *stream_list)
//{
//	stream_io_t *stream = stream_list;
//
//	while(stream != NULL)
//	{
//		if(stream->type == STREAM_TYPE_AUDIO)
//			return stream;
//
//		stream = stream->next;
//	}
//
//	return NULL;
//}

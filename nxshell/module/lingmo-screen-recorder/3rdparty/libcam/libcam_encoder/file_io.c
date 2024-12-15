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

#include "gviewencoder.h"
#include "file_io.h"
#include "gview.h"
#include "camview.h"
#include "gviewencoder.h"


/*
 * get the file position pointer
 * args:
 *   writer - pointer to io_writer
 *
 * asserts:
 *   writer is not null
 *
 * returns: the file position pointer
 */
static int64_t io_tell(io_writer_t *writer)
{
	/*assertions*/
	assert(writer != NULL);

	if(writer->fp == NULL)
	{
		fprintf(stderr, "ENCODER: (io_tell) no file pointer associated with io_writer (mem only ?)\n");
		return -1;
	}
	/*flush the file buffer*/
	fflush(writer->fp);

	/*return the file pointer position*/
	return ((int64_t) ftello(writer->fp));
}

/* flush a mem only writer(buf_writer) into a file writer
 * args:
 *   file_writer - pointer to a file io_writer
 *   buf_writer -pointer to a buf io_writer
 *
 * asserts:
 *
 * returns: delta between file and buffer writer positions
 */
//static int io_flush_buf_writer(io_writer_t *file_writer, io_writer_t *buf_writer)
//{
//	int size = (int) (buf_writer->buf_ptr - buf_writer->buffer);
//	io_write_buf(file_writer, buf_writer->buffer, size);
//	buf_writer->buf_ptr = buf_writer->buffer;
//
//	return size;
//}

/*
 * create a new writer:
 * args:
 *   filename - file for write to (if NULL mem only writer)
 *   max_size - mem buffer size (if 0 use default)
 *
 * asserts:
 *   none
 *
 * returns: pointer to io_writer
 */
io_writer_t *io_create_writer(const char *filename, int max_size)
{
	io_writer_t *writer = calloc(1, sizeof(io_writer_t));

	if(writer == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (io_create_writer): %s\n", strerror(errno));
		exit(-1);
	}

	if(max_size > 0)
		writer->buffer_size = max_size;
	else
		writer->buffer_size = IO_BUFFER_SIZE;

    writer->buffer = calloc((size_t)(writer->buffer_size), sizeof(uint8_t));
	if(writer->buffer == NULL)
	{
		fprintf(stderr, "ENCODER: FATAL memory allocation failure (io_create_writer): %s\n", strerror(errno));
		exit(-1);
	}
	
	writer->buf_ptr = writer->buffer;
	writer->buf_end = writer->buf_ptr + writer->buffer_size;

	if(filename != NULL)
	{
		writer->fp = fopen(filename, "wb");
		if (writer->fp == NULL)
		{
			fprintf(stderr, "ENCODER: Could not open file for writing: %s\n",
				strerror(errno));
			free(writer);
			return NULL;
		}
	}
	else
		writer->fp = NULL; /*mem only writer (must be flushed to a file writer*/

	return writer;
}

/*
 * destroy the writer (clean up)
 * args:
 *   writer - pointer to io_writer
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_destroy_writer(io_writer_t *writer)
{
	/*assertions*/
	assert(writer != NULL);

	if(writer->fp != NULL)
	{
		/* flush the buffer to file*/
		io_flush_buffer(writer);
		/* flush the file buffer*/
		fflush(writer->fp);
		/* close the file pointer */
		fclose(writer->fp);
	}

	/*clean the mem buffer*/
	free(writer->buffer);
    free(writer);
}

/*
 * flush the writer buffer to disk
 * args:
 *   writer - pointer to io_writer
 *
 * asserts:
 *   writer is not null
 *
 * returns: current offset
 */
int64_t io_flush_buffer(io_writer_t *writer)
{
	/*assertions*/
	assert(writer != NULL);

	if(writer->fp == NULL)
	{
		fprintf(stderr, "ENCODER: (io_flush) no file pointer associated with writer (mem only ?)\n");
		fprintf(stderr, "ENCODER: (io_flush) try to increase buffer size\n");
		return -1;
	}

	size_t nitems = 0;
	if (writer->buf_ptr > writer->buffer)
	{
        nitems= (size_t)(writer->buf_ptr - writer->buffer);
		if(fwrite(writer->buffer, 1, nitems, writer->fp) < nitems)
		{
			fprintf(stderr, "ENCODER: (io_flush) file write error: %s\n", strerror(errno));
            //stop_encoder_thread();
            return -1;
		}
	}
	else if (writer->buf_ptr < writer->buffer)
	{
		fprintf(stderr, "ENCODER: (io_flush) bad buffer pointer - dropping buffer\n");
		writer->buf_ptr = writer->buffer;
		return -1;
	}

    int64_t size_inc = (int64_t)nitems - (writer->size - writer->position);
	if(size_inc > 0)
		writer->size += size_inc;

	writer->position = io_tell(writer); /*update current file pointer position*/

	writer->buf_ptr = writer->buffer;

	/*should never happen*/
	if(writer->position > writer->size)
	{
		fprintf(stderr, "ENCODER: (io_flush) file pointer ( %" PRIu64 " ) above expected file size ( %" PRIu64 " )\n", writer->position, writer->size);
		writer->size = writer->position;
	}

	return writer->position;
}

/*
 * move the writer pointer to position
 * args:
 *   writer - pointer to io_writer
 *   position - new position offset
 *
 * asserts:
 *   writer is not null
 *
 * returns: error code
 */
int io_seek(io_writer_t *writer, int64_t position)
{
	/*assertions*/
	assert(writer != NULL);

	int ret = 0;

	if(position <= writer->size) //position is on the file
	{
		if(writer->fp == NULL)
		{
			fprintf(stderr, "ENCODER: (io_seek) no file pointer associated with writer (mem only ?)\n");
			return -1;
		}
		/*flush the memory buffer (we need an empty buffer)*/
		io_flush_buffer(writer);
		/*try to move the file pointer to position*/
		int ret = fseeko(writer->fp, position, SEEK_SET);
		if(ret != 0)
			fprintf(stderr, "ENCODER: (io_seek) seek to file position %" PRIu64 "failed\n", position);
		else
			writer->position = io_tell(writer); /*update current file pointer position*/

		/*we are now on position with an empty memory buffer*/
	}
	else /* position is on the buffer*/
	{
		/*move file pointer to EOF*/
		if(writer->position != writer->size)
		{
			fseeko(writer->fp, writer->size, SEEK_SET);
			writer->position = writer->size;
		}
		/*move buffer pointer to position*/
		writer->buf_ptr = writer->buffer + (position - writer->size);
	}

	return ret;
}

/*
 * move file pointer by offset
 * args:
 *   writer - pointer to io_writer
 *   offset - offset value to skip
 *
 * asserts:
 *   writer is not null
 *
 * returns: error code
 */
int io_skip(io_writer_t *writer, int offset)
{
	/*assertions*/
	assert(writer != NULL);

	if(writer->fp == NULL)
	{
		fprintf(stderr, "ENCODER: (io_skip) no file pointer associated with writer (mem only ?)\n");
		return -1;
	}
	/*flush the memory buffer (clean buffer)*/
	io_flush_buffer(writer);
	/*try to move the file pointer to position*/
	int ret = fseeko(writer->fp, offset, SEEK_CUR);
	if(ret != 0)
		fprintf(stderr, "ENCODER: (io_skip) skip file pointer by 0x%x failed\n", offset);

	writer->position = io_tell(writer); //update current file pointer position

	/*we are on position with an empty memory buffer*/
	return ret;
}

/*
 * get writer offset (current position)
 * args:
 *   writer - pointer to io_writer
 *
 * asserts:
 *   writer is not null
 *
 * returns: current writer position offset
 */
int64_t io_get_offset(io_writer_t *writer)
{
	/*assertions*/
	assert(writer != NULL);

	/*buffer offset*/
	int64_t offset = writer->buf_ptr - writer->buffer;
	if(offset < 0)
	{
		fprintf(stderr, "ENCODER: (io_get_offset) bad buf pointer\n");
		writer->buf_ptr = writer->buffer;
		offset = 0;
	}
	/*add to file offset*/
	offset += writer->position;

	return offset;
}

/*
 * write 1 octet
 * args:
 *   writer - pointer to io_writer
 *   b - octect to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_w8(io_writer_t *writer, uint8_t b)
{
	*writer->buf_ptr++ = b;
    if (writer->buf_ptr >= writer->buf_end)
        io_flush_buffer(writer);
}

/*
 * write a buffer of size
 * args:
 *   writer - pointer to io_writer
 *   buf - data buffer to write
 *   size - size of buffer
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_buf(io_writer_t *writer, uint8_t *buf, int size)
{
	while (size > 0)
	{
        int len = (int)(writer->buf_end - writer->buf_ptr);
		if(len < 0)
			fprintf(stderr,"ENCODER: (io_write_buf) buff pointer outside buffer\n");
		if(len >= size)
			len = size;

        memcpy(writer->buf_ptr, buf,(size_t) len);
        writer->buf_ptr += len;

       if (writer->buf_ptr >= writer->buf_end)
            io_flush_buffer(writer);

        buf += len;
        size -= len;
    }
}

/*
 * write 2 octets (little endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_wl16(io_writer_t *writer, uint16_t val)
{
    io_write_w8(writer, (uint8_t) val);
    io_write_w8(writer, (uint8_t) (val >> 8));
}

/*
 *  write 2 octets (bid endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_wb16(io_writer_t *writer, uint16_t val)
{
    io_write_w8(writer, (uint8_t) (val >> 8));
    io_write_w8(writer, (uint8_t) val);
}

/*
 * write 3 octets (little endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
//void io_write_wl24(io_writer_t *writer, uint32_t val)
//{
//    io_write_wl16(writer, (uint16_t) (val & 0xffff));
//    io_write_w8(writer, (uint8_t) (val >> 16));
//}

/*
 * write 3 octets (bid endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
//void io_write_wb24(io_writer_t *writer, uint32_t val)
//{
//    io_write_wb16(writer, (uint16_t) (val >> 8));
//    io_write_w8(writer, (uint8_t) val);
//}

/*
 * write 4 octets (little endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_wl32(io_writer_t *writer, uint32_t val)
{
    io_write_w8(writer, (uint8_t) val);
    io_write_w8(writer, (uint8_t) (val >> 8));
    io_write_w8(writer, (uint8_t) (val >> 16));
    io_write_w8(writer, (uint8_t) (val >> 24));
}

/*
 * write 4 octets (bid endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_wb32(io_writer_t *writer, uint32_t val)
{
    io_write_w8(writer, (uint8_t) (val >> 24));
    io_write_w8(writer, (uint8_t) (val >> 16));
    io_write_w8(writer, (uint8_t) (val >> 8));
    io_write_w8(writer, (uint8_t) val);
}

/*
 *  write 8 octets (little endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_wl64(io_writer_t *writer, uint64_t val)
{
    io_write_wl32(writer, (uint32_t)(val & 0xffffffff));
    io_write_wl32(writer, (uint32_t)(val >> 32));
}

/*
 *  write 4 octets (bid endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_wb64(io_writer_t *writer, uint64_t val)
{
    io_write_wb32(writer, (uint32_t)(val >> 32));
    io_write_wb32(writer, (uint32_t)(val & 0xffffffff));
}

/*
 * write 4cc code
 * args:
 *   writer - pointer to io_writer
 *   str -  4 char string with 4cc code
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_4cc(io_writer_t *writer, const char *str)
{
    int len = 4;
    if((int)strlen(str) < len )
	{
        len = (int)strlen(str);
	}

    io_write_buf(writer, (uint8_t *) str, len);

    len = 4 - len;
    /*fill remaining chars with spaces*/
    while(len > 0)
    {
		io_write_w8(writer, ' ');
		len--;
	}
}

/*
 * write a string (null terminated)
 * args:
 *   writer - pointer to io_writer
 *   str -  4 char string with 4cc code
 *
 * asserts:
 *   writer is not null
 *
 * returns: the size writen
 */
//int io_write_str(io_writer_t * writer, const char *str)
//{
//    int len = 1;
//    if (str) {
//        len += strlen(str);
//        io_write_buf(writer, (uint8_t *) str, len);
//    } else
//        io_write_w8(writer, 0);
//    return len;
//}

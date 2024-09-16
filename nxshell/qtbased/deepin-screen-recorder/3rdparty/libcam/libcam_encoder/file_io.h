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

#ifndef FILE_IO_H
#define FILE_IO_H

#include <inttypes.h>
#include <sys/types.h>
#include <stdio.h>

#include "cameraconfig.h"


#define IO_BUFFER_SIZE 32768

typedef struct _io_writer_t {
    FILE *fp;      /* file pointer     */

    uint8_t *buffer;  /* Start of the buffer. */
    int buffer_size;  /* Maximum buffer size */
    uint8_t *buf_ptr; /* Current position in the buffer */
    uint8_t *buf_end; /* End of the buffer. */

    int64_t size; //file size (end of file position)
    int64_t position; //file pointer position (updates on buffer flush)
} io_writer_t;

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
io_writer_t *io_create_writer(const char *filename, int max_size);

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
void io_destroy_writer(io_writer_t *writer);

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
int64_t io_flush_buffer(io_writer_t *writer);

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
int io_seek(io_writer_t *writer, int64_t position);

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
int io_skip(io_writer_t *writer, int offset);

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
int64_t io_get_offset(io_writer_t *writer);

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
void io_write_w8(io_writer_t *writer, uint8_t b);

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
void io_write_buf(io_writer_t *writer, uint8_t *buf, int size);

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
void io_write_wl16(io_writer_t *writer, uint16_t val);

/*
 * write 2 octets (bid endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_wb16(io_writer_t *writer, uint16_t val);

/* write 3 octets (little endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
//void io_write_wl24(io_writer_t *writer, uint32_t val);

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
//void io_write_wb24(io_writer_t *writer, uint32_t val);

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
void io_write_wl32(io_writer_t *writer, uint32_t val);

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
void io_write_wb32(io_writer_t *writer, uint32_t val);

/*
 * write 8 octets (little endian)
 * args:
 *   writer - pointer to io_writer
 *   val - value to write
 *
 * asserts:
 *   writer is not null
 *
 * returns: none
 */
void io_write_wl64(io_writer_t *writer, uint64_t val);

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
void io_write_wb64(io_writer_t *writer, uint64_t val);

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
void io_write_4cc(io_writer_t *writer, const char *str);

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
//int io_write_str(io_writer_t * writer, const char *str);

#if BIGENDIAN
#define io_write_w16 io_write_wb16
#define io_write_w24 io_write_wb24
#define io_write_w32 io_write_wb32
#define io_write_w64 io_write_wb64
#else
#define io_write_w16 io_write_wl16
#define io_write_w24 io_write_wl24
#define io_write_w32 io_write_wl32
#define io_write_w64 io_write_wl64
#endif

#endif

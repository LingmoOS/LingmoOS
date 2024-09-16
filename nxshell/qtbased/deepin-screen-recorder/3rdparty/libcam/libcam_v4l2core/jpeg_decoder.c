/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
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
#  M/Jpeg decoding and frame capture taken from luvcview                        #
#                                                                               #
********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "gviewv4l2core.h"
#include "colorspaces.h"
#include "frame_decoder.h"
#include "jpeg_decoder.h"
#include "gview.h"
#include "cameraconfig.h"

//LMH0613++
#include <libavutil/imgutils.h>

#include "load_libs.h"
extern int verbosity;

/* default Huffman table*/
#define JPG_HUFFMAN_TABLE_LENGTH 0x01A0

const uint8_t jpeg_huffman_table[JPG_HUFFMAN_TABLE_LENGTH] =
{
	// luminance dc - length bits
	0x00,
	0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// luminance dc - code
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0A, 0x0B,
	// chrominance dc - length bits
	0x01,
	0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	// chrominance dc - code
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0A, 0x0B,
	// luminance ac - number of codes with # bits (ordered by code length 1-16)
	0x10,
	0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05,
	0x04, 0x04, 0x00, 0x00, 0x01, 0x7D,
	// luminance ac - run size (ordered by code length)
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31,
	0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32,
	0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52,
	0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
	0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
	0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83,
	0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94,
	0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
	0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
	0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
	0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8,
	0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8,
	0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA,
	// chrominance ac -number of codes with # bits (ordered by code length 1-16)
	0x11,
	0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05,
	0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
	// chrominance ac - run size (ordered by code length)
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06,
	0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81,
	0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33,
	0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34,
	0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
	0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56,
	0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
	0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92,
	0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3,
	0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
	0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
	0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
	0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
	0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA
};

typedef struct _jpeg_decoder_context_t
{
	void *codec_data;

	int width;
	int height;
	int pic_size;

	uint8_t *tmp_frame; //temp frame buffer

} jpeg_decoder_context_t;

static jpeg_decoder_context_t *jpeg_ctx = NULL;

//LMH0613
#if 0 //use internal jpeg decoder
//#if MJPG_BUILTIN //use internal jpeg decoder

#define ISHIFT 11

#define IFIX(a) ((int)((a) * (1 << ISHIFT) + .5))

#ifndef __P
# define __P(x) x
#endif

/* special markers */
#define M_BADHUFF	-1
#define M_EOF		0x80

#undef PREC
#define PREC int

/******** Markers *********/
#define M_SOI   0xd8
#define M_APP0  0xe0
#define M_DQT   0xdb
#define M_SOF0  0xc0
#define M_DHT   0xc4
#define M_DRI   0xdd
#define M_SOS   0xda
#define M_RST0  0xd0
#define M_EOI   0xd9
#define M_COM   0xfe

/*
 * IDCT data
 */
#define IMULT(a, b) (((a) * (b)) >> ISHIFT)
#define ITOINT(a) ((a) >> ISHIFT)

#define S22 ((PREC)IFIX(2 * 0.382683432))
#define C22 ((PREC)IFIX(2 * 0.923879532))
#define IC4 ((PREC)IFIX(1 / 0.707106781))

/*zigzag order used by idct*/
static unsigned char zig2[64] = {
    0, 2, 3, 9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
    5, 7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
    1, 4, 8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
    6, 13, 17, 24, 32, 38, 47, 49
};

static uint8_t zig[64] = {
    0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7, 13, 16, 26, 29, 42,
    3, 8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

/*coef used in idct*/
static PREC aaidct[8] = {
    IFIX(0.3535533906), IFIX(0.4903926402),
    IFIX(0.4619397663), IFIX(0.4157348062),
    IFIX(0.3535533906), IFIX(0.2777851165),
    IFIX(0.1913417162), IFIX(0.0975451610)
};

/*
 * decoder structs
 */
struct jpeg_decdata
{
	int dcts[6 * 64 + 16];
	int out[64 * 6];
	int dquant[3][64];
};

struct in
{
	uint8_t *p;
	uint32_t bits;
	int left;
	int marker;
	int (*func) __P((void *));
	void *data;
};

#define LEBI_DCL	int le, bi
#define LEBI_GET(in)	(le = in->left, bi = in->bits)
#define LEBI_PUT(in)	(in->left = le, in->bits = bi)

/*********************************/
#define DECBITS 10		/* seems to be the optimum */

struct dec_hufftbl
{
	int maxcode[17];
	int valptr[16];
	uint8_t vals[256];
	uint32_t llvals[1 << DECBITS];
};

//struct enc_hufftbl;

union hufftblp
{
	struct dec_hufftbl *dhuff;
	//struct enc_hufftbl *ehuff;
};

struct scan
{
	int dc;			/* old dc value */

	union hufftblp hudc;
	union hufftblp huac;
	int next;		/* when to switch to next scan */

	int cid;		/* component id */
	int hv;			/* horiz/vert, copied from comp */
	int tq;			/* quant tbl, copied from comp */
};

#define MAXCOMP 4

struct comp
{
	int cid;
	int hv;
	int tq;
};

struct jpginfo
{
	int nc;			/* number of components */
	int ns;			/* number of scans */
	int dri;		/* restart interval */
	int nm;			/* mcus til next marker */
	int rm;			/* next restart marker */
};

static struct jpginfo info;
static struct comp comps[MAXCOMP];

static struct scan dscans[MAXCOMP];

static uint8_t quant[4][64];

static struct dec_hufftbl dhuff[4];

#define dec_huffdc (dhuff + 0)
#define dec_huffac (dhuff + 2)

/*
 * build huffman data
 * args:
 *    hu - pointer to dec_hufftbl struct
 *    hufflen - pointer to int with code size
 *    huffvals - pointer to uint8_t with huffman values
 *
 * asserts:
 *    hu not null
 *    hufflen not null
 *    huffvals not null
 *
 * returns: error code (0 - OK)
 */
static void dec_makehuff(struct dec_hufftbl *hu, int *hufflen, uint8_t *huffvals)
{
	/*assertions*/
	assert(hu != NULL);
	assert(hufflen != NULL);
	assert(huffvals != NULL);

	int code, k, i, j, d, x, c, v;
	for (i = 0; i < (1 << DECBITS); i++)
		hu->llvals[i] = 0;

	/*
	* llvals layout:
	*
	* value v already known, run r, backup u bits:
	*  vvvvvvvvvvvvvvvv 0000 rrrr 1 uuuuuuu
	* value unknown, size b bits, run r, backup u bits:
	*  000000000000bbbb 0000 rrrr 0 uuuuuuu
	* value and size unknown:
	*  0000000000000000 0000 0000 0 0000000
	*/
	code = 0;
	k = 0;
	for (i = 0; i < 16; i++, code <<= 1)
	{	/* sizes */
		hu->valptr[i] = k;
		for (j = 0; j < hufflen[i]; j++)
		{
			hu->vals[k] = *huffvals++;
			if (i < DECBITS)
			{
				c = code << (DECBITS - 1 - i);
				v = hu->vals[k] & 0x0f;	/* size */
				for (d = 1 << (DECBITS - 1 - i); --d >= 0;)
				{
					if (v + i < DECBITS)
					{	/* both fit in table */
						x = d >> (DECBITS - 1 - v - i);
						if (v && x < (1 << (v - 1)))
							x += (-1 << v) + 1;
						x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
							(DECBITS - (i + 1 + v)) | 128;
					}
					else
						x = v << 16 | (hu->vals[k] & 0xf0) << 4 |
							(DECBITS - (i + 1));
					hu->llvals[c | d] = x;
				}
			}
			code++;
			k++;
		}
		hu->maxcode[i] = code;
	}
	hu->maxcode[16] = 0x20000;	/* always terminate decode */
}

/*
 * huffman decoder initialization
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: error code (0 - OK)
 */
static int huffman_init(void)
{
	uint8_t *ptr= (uint8_t *) jpeg_huffman_table ;
	int i, j, l;
	l = JPG_HUFFMAN_TABLE_LENGTH ;
	while (l > 0)
	{
		int hufflen[16], k;
		uint8_t huffvals[256];

		int tc = *ptr++;
		int th = tc & 15;
		tc >>= 4;
		int tt = tc * 2 + th;
		if (tc > 1 || th > 1)
			return E_BAD_TABLES_ERR;
		for (i = 0; i < 16; i++)
			hufflen[i] = *ptr++;
		l -= 1 + 16;
		k = 0;
		for (i = 0; i < 16; i++)
		{
			for (j = 0; j < hufflen[i]; j++)
				huffvals[k++] = *ptr++;
			l -= hufflen[i];
		}
		dec_makehuff(dhuff + tt, hufflen, huffvals);
	}
	return 0;
}

/*
 * fillbits
 * args:
 *    inp - pointer to struct in
 *    le - left
 *    bi - bits
 *
 * asserts:
 *    inp not null
 *
 * returns: error code (0 - OK)
 */
static int fillbits(struct in *inp, int le, unsigned int bi)
{
	/*asserts*/
	assert(inp != NULL);

	if (inp->marker)
	{
		if (le <= 16)
			inp->bits = bi << 16, le += 16;
		return le;
	}
	while (le <= 24)
	{
		int b = *inp->p++;
		int m = 0;

		if (b == 0xff && (m = *inp->p++) != 0)
		{
			if (m == M_EOF)
			{
				if (inp->func && (m = inp->func(inp->data)) == 0)
					continue;
			}
			inp->marker = m;
			if (le <= 16)
				bi = bi << 16, le += 16;
			break;
		}
		bi = bi << 8 | b;
		le += 8;
	}
	inp->bits = bi;		/* tmp... 2 return values needed */
	return le;
}

static int dec_rec2
__P((struct in *, struct dec_hufftbl *, int *, int, int));

#define GETBITS(in, n) (					\
  (le < (n) ? le = fillbits(in, le, bi), bi = in->bits : 0),	\
  (le -= (n)),							\
  bi >> le & ((1 << (n)) - 1)					\
)

#define UNGETBITS(in, n) (	\
  le += (n)			\
)

#define DEC_REC(in, hu, r, i)	 (	\
  r = GETBITS(in, DECBITS),		\
  i = hu->llvals[r],			\
  i & 128 ?				\
    (					\
      UNGETBITS(in, i & 127),		\
      r = i >> 8 & 15,			\
      i >> 16				\
    )					\
  :					\
    (					\
      LEBI_PUT(in),			\
      i = dec_rec2(in, hu, &r, r, i),	\
      LEBI_GET(in),			\
      i					\
    )					\
)

/*
 * mcus decoder
 * args:
 *    inp - pointer to struct in
 *    dct - pointer to int with dct values
 *    n - number of dct values
 *    sc - pointer to struct scan
 *    maxp - pointer to int
 *
 * asserts:
 *    none
 *
 * returns: error code (0 - OK)
 */
static void decode_mcus(struct in *inp, int *dct, int n, struct scan *sc, int *maxp)
{
	struct dec_hufftbl *hu;
	int r = 0, t = 0;
	LEBI_DCL;

	memset(dct, 0, n * 64 * sizeof(*dct));
	LEBI_GET(inp);
	while (n-- > 0)
	{
		hu = sc->hudc.dhuff;
		*dct++ = (sc->dc += DEC_REC(inp, hu, r, t));

		hu = sc->huac.dhuff;
		int i = 63;

		while (i > 0)
		{
			t = DEC_REC(inp, hu, r, t);
			if (t == 0 && r == 0)
			{
				dct += i;
				break;
			}
			dct += r;
			*dct++ = t;
			i -= r + 1;
		}
		*maxp++ = 64 - i;
		if (n == sc->next)
		sc++;
	}
	LEBI_PUT(inp);
}

/*
 * readmarker decoder
 * args:
 *    inp - pointer to struct in
 *
 * asserts:
 *    inp not null
 *
 * returns: error code (0 - OK)
 */
static int dec_readmarker(struct in *inp)
{
	/*asserts*/
	assert(inp != NULL);

	int m;

	inp->left = fillbits(inp, inp->left, inp->bits);
	if ((m = inp->marker) == 0)
		return 0;
	inp->left = 0;
	inp->marker = 0;
	return m;
}

/*
 * set input
 * args:
 *    inp - pointer to struct in
 *    p - pointer to pixel data
 *
 * asserts:
 *    inp not null
 *
 * returns: error code (0 - OK)
 */
static void setinput(struct in *inp, uint8_t *p)
{
	/*asserts*/
	assert(inp != NULL);

	inp->p = p;
	inp->left = 0;
	inp->bits = 0;
	inp->marker = 0;
}

/*
 * IDCT quantization table
 */
static void idctqtab(uint8_t *qin, PREC *qout)
{
	int i, j;

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
				IMULT(aaidct[i], aaidct[j]);
}

/****************************************************************/
/**************             idct                  ***************/
/****************************************************************/

/*
 * inverse dct for jpeg decoding
 * args:
 *   in -  pointer to input data ( mcu - after huffman decoding)
 *   out - pointer to data with output of idct (to be filled)
 *   quant - pointer to quantization data tables
 *   off - offset value (128.5 or 0.5)
 *   max - maximum input mcu index?
 *
 * asserts:
 *   none
 *
 * returns: none
 */
inline static void idct(int *inp, int *out, int *quant, long off, int max)
{
	long t0, t1, t2, t3, t4, t5, t6, t7;	// t ;
	long tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	long tmp[64], *tmpp;
	int i, j, te;
	uint8_t *zig2p;

	t0 = off;
	if (max == 1) //single color mcu
	{
		t0 += inp[0] * quant[0];     //only DC available
		for (i = 0; i < 64; i++)    // fill mcu with DC value
			out[i] = ITOINT(t0);
		return;
	}
	zig2p = zig2;
	tmpp = tmp;
	for (i = 0; i < 8; i++) //apply quantization table in zigzag order
	{
		j = *zig2p++;
		t0 += inp[j] * (long) quant[j];
		j = *zig2p++;
		t5 = inp[j] * (long) quant[j];
		j = *zig2p++;
		t2 = inp[j] * (long) quant[j];
		j = *zig2p++;
		t7 = inp[j] * (long) quant[j];
		j = *zig2p++;
		t1 = inp[j] * (long) quant[j];
		j = *zig2p++;
		t4 = inp[j] * (long) quant[j];
		j = *zig2p++;
		t3 = inp[j] * (long) quant[j];
		j = *zig2p++;
		t6 = inp[j] * (long) quant[j];


		if ((t1 | t2 | t3 | t4 | t5 | t6 | t7) == 0)
		{
			tmpp[0 * 8] = t0; //DC
			tmpp[1 * 8] = t0;
			tmpp[2 * 8] = t0;
			tmpp[3 * 8] = t0;
			tmpp[4 * 8] = t0;
			tmpp[5 * 8] = t0;
			tmpp[6 * 8] = t0;
			tmpp[7 * 8] = t0;

			tmpp++;
			t0 = 0;
			continue;
		}
		//IDCT;
		tmp0 = t0 + t1;
		t1 = t0 - t1;
		tmp2 = t2 - t3;
		t3 = t2 + t3;
		tmp2 = IMULT(tmp2, IC4) - t3;
		tmp3 = tmp0 + t3;
		t3 = tmp0 - t3;
		tmp1 = t1 + tmp2;
		tmp2 = t1 - tmp2;
		tmp4 = t4 - t7;
		t7 = t4 + t7;
		tmp5 = t5 + t6;
		t6 = t5 - t6;
		tmp6 = tmp5 - t7;
		t7 = tmp5 + t7;
		tmp5 = IMULT(tmp6, IC4);
		tmp6 = IMULT((tmp4 + t6), S22);
		tmp4 = IMULT(tmp4, (C22 - S22)) + tmp6;
		t6 = IMULT(t6, (C22 + S22)) - tmp6;
		t6 = t6 - t7;
		t5 = tmp5 - t6;
		t4 = tmp4 - t5;

		tmpp[0 * 8] = tmp3 + t7;        //t0;
		tmpp[1 * 8] = tmp1 + t6;        //t1;
		tmpp[2 * 8] = tmp2 + t5;        //t2;
		tmpp[3 * 8] = t3 + t4;          //t3;
		tmpp[4 * 8] = t3 - t4;          //t4;
		tmpp[5 * 8] = tmp2 - t5;        //t5;
		tmpp[6 * 8] = tmp1 - t6;        //t6;
		tmpp[7 * 8] = tmp3 - t7;        //t7;
		tmpp++;
		t0 = 0;
	}
	for (i = 0, j = 0; i < 8; i++)
	{
		t0 = tmp[j + 0];
		t1 = tmp[j + 1];
		t2 = tmp[j + 2];
		t3 = tmp[j + 3];
		t4 = tmp[j + 4];
		t5 = tmp[j + 5];
		t6 = tmp[j + 6];
		t7 = tmp[j + 7];
		if ((t1 | t2 | t3 | t4 | t5 | t6 | t7) == 0)
		{
			te = ITOINT(t0);
			out[j + 0] = te;
			out[j + 1] = te;
			out[j + 2] = te;
			out[j + 3] = te;
			out[j + 4] = te;
			out[j + 5] = te;
			out[j + 6] = te;
			out[j + 7] = te;
			j += 8;
			continue;
		}
		//IDCT;
		tmp0 = t0 + t1;
		t1 = t0 - t1;
		tmp2 = t2 - t3;
		t3 = t2 + t3;
		tmp2 = IMULT(tmp2, IC4) - t3;
		tmp3 = tmp0 + t3;
		t3 = tmp0 - t3;
		tmp1 = t1 + tmp2;
		tmp2 = t1 - tmp2;
		tmp4 = t4 - t7;
		t7 = t4 + t7;
		tmp5 = t5 + t6;
		t6 = t5 - t6;
		tmp6 = tmp5 - t7;
		t7 = tmp5 + t7;
		tmp5 = IMULT(tmp6, IC4);
		tmp6 = IMULT((tmp4 + t6), S22);
		tmp4 = IMULT(tmp4, (C22 - S22)) + tmp6;
		t6 = IMULT(t6, (C22 + S22)) - tmp6;
		t6 = t6 - t7;
		t5 = tmp5 - t6;
		t4 = tmp4 - t5;

		out[j + 0] = ITOINT(tmp3 + t7);
		out[j + 1] = ITOINT(tmp1 + t6);
		out[j + 2] = ITOINT(tmp2 + t5);
		out[j + 3] = ITOINT(t3 + t4);
		out[j + 4] = ITOINT(t3 - t4);
		out[j + 5] = ITOINT(tmp2 - t5);
		out[j + 6] = ITOINT(tmp1 - t6);
		out[j + 7] = ITOINT(tmp3 - t7);
		j += 8;
	}
}

/*********************************/
//static void col221111 __P((int *, unsigned char *, int));

typedef void (*ftopict) (int * out, uint8_t *pic, int width) ;

/*********************************/
/*
 * pointer to pixel data
 */
static uint8_t *datap;

/*
 * input structure (in)
 */
static struct in inp;


/*
 * get byte (8 bit) from datap
 */
static int getbyte(void)
{
	return *datap++;
}

/*
 * get word (16 bit) from datap
 */
static int getword(void)
{
	int c1, c2;
	c1 = *datap++;
	c2 = *datap++;
	return c1 << 8 | c2;
}

/*
 * read jpeg tables (huffman and quantization)
 * args:
 *    till - Marker (frame - SOF0   scan - SOS)
 *    isDHT - flag indicating the presence of huffman tables (if 0 must use default ones - MJPG frame)
 * asserts:
 *    none
 *
 * returns: error code (0 - OK)
 */
static int readtables(int till, int *isDHT)
{
	int l, i, j, lq, pq, tq;
	int tc, th, tt;

	for (;;)
	{
		if (getbyte() != 0xff)
			return -1;

		int m = 0;

		if ((m = getbyte()) == till)
			break;

		switch (m)
		{
			case 0xc2:
				return 0;
			/*read quantization tables (Lqt and Cqt)*/
			case M_DQT:
				lq = getword();
				while (lq > 2)
				{
					pq = getbyte();
					/*Lqt=0x00   Cqt=0x01*/
					tq = pq & 15;
					if (tq > 3)
					return -1;
					pq >>= 4;
					if (pq != 0)
					return -1;
					for (i = 0; i < 64; i++)
						quant[tq][i] = getbyte();
					lq -= 64 + 1;
				}
				break;
			/*read huffman table*/
			case M_DHT:
				l = getword();
				while (l > 2)
				{
					int hufflen[16], k;
					uint8_t huffvals[256];

					tc = getbyte();
					th = tc & 15;
					tc >>= 4;
					tt = tc * 2 + th;
					if (tc > 1 || th > 1)
					return -1;

					for (i = 0; i < 16; i++)
						hufflen[i] = getbyte();
					l -= 1 + 16;
					k = 0;
					for (i = 0; i < 16; i++)
					{
						for (j = 0; j < hufflen[i]; j++)
							huffvals[k++] = getbyte();
						l -= hufflen[i];
					}
					dec_makehuff(dhuff + tt, hufflen, huffvals);
				}
				/* has huffman tables defined (JPEG)*/
				*isDHT= 1;
				break;
			/*restart interval*/
			case M_DRI:
				l = getword();
				info.dri = getword();
				break;

			default:
				l = getword();
				while (l-- > 2)
					getbyte();
				break;
		}
	}
	return 0;
}

/*
 * init dscans
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
static void dec_initscans(void)
{
	int i;

	info.nm = info.dri + 1;
	info.rm = M_RST0;
	for (i = 0; i < info.ns; i++)
		dscans[i].dc = 0;
}

/*
 * check markers
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: error code (0 - OK)
 */
static int dec_checkmarker(void)
{
	int i;

	if (dec_readmarker(&inp) != info.rm)
		return -1;
	info.nm = info.dri;
	info.rm = (info.rm + 1) & ~0x08;
	for (i = 0; i < info.ns; i++)
		dscans[i].dc = 0;
	return 0;
}

/*
 * check markers
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: error code (0 - OK)
 */
static int dec_rec2(struct in *inp, struct dec_hufftbl *hu, int *runp, int c, int i)
{
	LEBI_DCL;

	LEBI_GET(inp);
	if (i)
	{
		UNGETBITS(inp, i & 127);
		*runp = i >> 8 & 15;
		i >>= 16;
	}
	else
	{
		for (i = DECBITS;
		(c = ((c << 1) | GETBITS(inp, 1))) >= (hu->maxcode[i]); i++);
		if (i >= 16)
		{
			inp->marker = M_BADHUFF;
			return 0;
		}
		i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
		*runp = i >> 4;
		i &= 15;
	}
	if (i == 0)
	{	/* sigh, 0xf0 is 11 bit */
		LEBI_PUT(inp);
		return 0;
	}
	/* receive part */
	c = GETBITS(inp, i);
	if (c < (1 << (i - 1)))
		c += (-1 << i) + 1;
	LEBI_PUT(inp);
	return c;
}

/*
 * init (m)jpeg decoder context
 * args:
 *    width - image width
 *    height - image height
 *
 * asserts:
 *    none
 *
 * returns: error code (0 - E_OK)
 */
int jpeg_init_decoder(int width, int height)
{
	if(jpeg_ctx != NULL)
		jpeg_close_decoder();

	jpeg_ctx = calloc(1, sizeof(jpeg_decoder_context_t));
	if(jpeg_ctx == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (jpeg_init_decoder): %s\n", strerror(errno));
		exit(-1);
	}

	jpeg_ctx->width = width;
	jpeg_ctx->height = height;
	jpeg_ctx->pic_size = width * height * 2; //yuyv
	jpeg_ctx->codec_data = NULL;

	jpeg_ctx->tmp_frame = calloc(jpeg_ctx->pic_size, sizeof(uint8_t));
	if(jpeg_ctx->tmp_frame == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (jpeg_init_decoder): %s\n", strerror(errno));
		exit(-1);
	}

	return E_OK;
}

/*
 * jpeg decode
 * args:
 *   out_buf -  pointer to picture data ( decoded image - yuyv format)
 *   in_buf -  pointer to input data ( compressed jpeg )
 *   size - picture size
 *
 * asserts:
 *   out_buf not null
 *   in_buf not null
 *
 * returns: error code (0 - OK)
 */
//int jpeg_decode(uint8_t **pic, uint8_t *buf, int width, int height)
int jpeg_decode(uint8_t *out_buf, uint8_t *in_buf, int size)
{
	/*asserts*/
	assert(in_buf != NULL);
	assert(out_buf != NULL);

	memcpy(jpeg_ctx->tmp_frame, in_buf, size);

	struct jpeg_decdata *decdata;
	int i=0, j=0, m=0, tac=0, tdc=0;
	int intwidth=0, intheight=0;
	int mcusx=0, mcusy=0, mx=0, my=0;
	int ypitch=0 ,xpitch=0,bpp=0,pitch=0,x=0,y=0;
	int mb=0;
	int max[6];
	ftopict convert;
	int err = 0;
	int isInitHuffman = 0;
	decdata = calloc(1, sizeof(struct jpeg_decdata));
	if(decdata == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (jpeg_decode): %s\n", strerror(errno));
		exit(-1);
	}

	for(i=0;i<6;i++)
		max[i]=0;

	if (!decdata)
	{
		err = E_ALLOC_ERR;
		goto error;
	}

	datap = jpeg_ctx->tmp_frame;
	/*check SOI (0xFFD8)*/
	if (getbyte() != 0xff)
	{
		err = E_NO_SOI_ERR;
		goto error;
	}
	if (getbyte() != M_SOI)
	{
		err = E_NO_SOI_ERR;
		goto error;
	}
	/*read tables - if exist, up to start frame marker (0xFFC0)*/
	if (readtables(M_SOF0, &isInitHuffman))
	{
		err = E_BAD_TABLES_ERR;
		goto error;
	}
	getword();     /*header lenght*/
	i = getbyte(); /*precision (8 bit)*/
	if (i != 8)
	{
		err = E_NOT_8BIT_ERR;
		goto error;
	}
	intheight = getword(); /*height*/
	intwidth = getword();  /*width */

	if ((intheight & 7) || (intwidth & 7)) /*must be even*/
	{
		err = E_BAD_WIDTH_OR_HEIGHT_ERR;
		goto error;
	}
	info.nc = getbyte(); /*number of components*/
	if (info.nc > MAXCOMP)
	{
		err = E_TOO_MANY_COMPPS_ERR;
		goto error;
	}
	/*for each component*/
	for (i = 0; i < info.nc; i++)
	{
		int h, v;
		comps[i].cid = getbyte(); /*component id*/
		comps[i].hv = getbyte();
		v = comps[i].hv & 15; /*vertical sampling   */
		h = comps[i].hv >> 4; /*horizontal sampling */
		comps[i].tq = getbyte(); /*quantization table used*/
		if (h > 3 || v > 3)
		{
			err = E_ILLEGAL_HV_ERR;
			goto error;
		}
		if (comps[i].tq > 3)
		{
			err = E_QUANT_TBL_SEL_ERR;
			goto error;
		}
	}
	/*read tables - if exist, up to start of scan marker (0xFFDA)*/
	if (readtables(M_SOS,&isInitHuffman))
	{
		err = E_BAD_TABLES_ERR;
		goto error;
	}
	getword(); /* header lenght */
	info.ns = getbyte(); /* number of scans */
	if (!info.ns)
	{
		printf("V4L2_CORE: (jpeg decoder) info ns %d/n",info.ns);
		err = E_NOT_YCBCR_ERR;
		goto error;
	}
	/*for each scan*/
	for (i = 0; i < info.ns; i++)
	{
		dscans[i].cid = getbyte(); /*component id*/
		tdc = getbyte();
		tac = tdc & 15; /*ac table*/
		tdc >>= 4;      /*dc table*/
		if (tdc > 1 || tac > 1)
		{
			err = E_QUANT_TBL_SEL_ERR;
			goto error;
		}
		for (j = 0; j < info.nc; j++)
			if (comps[j].cid == dscans[i].cid)
				break;
		if (j == info.nc)
		{
			err = E_UNKNOWN_CID_ERR;
			goto error;
		}
		dscans[i].hv = comps[j].hv;
		dscans[i].tq = comps[j].tq;
		dscans[i].hudc.dhuff = dec_huffdc + tdc;
		dscans[i].huac.dhuff = dec_huffac + tac;
	}

	i = getbyte(); /*0 */
	j = getbyte(); /*63*/
	m = getbyte(); /*0 */

	if (i != 0 || j != 63 || m != 0)
	{
		fprintf(stderr, "V4L2_CORE: (jpeg decoder) FW error,not seq DCT ??\n");
	}

	/*build huffman tables*/
	if(!isInitHuffman)
	{
		if(huffman_init() < 0)
			return E_BAD_TABLES_ERR;
	}
	/*
	if (dscans[0].cid != 1 || dscans[1].cid != 2 || dscans[2].cid != 3)
	{
		err = ERR_NOT_YCBCR_221111;
		goto error;
	}

	if (dscans[1].hv != 0x11 || dscans[2].hv != 0x11)
	{
		err = ERR_NOT_YCBCR_221111;
		goto error;
	}
	*/

	/*
	 * if internal width and external are not the same or heigth too
	 * and pic not allocated realloc the good size and mark the change
	 * need 1 macroblock line more ??
	 */
	//if (intwidth != width || intheight != height || pic == NULL)
	//{
	//	width = intwidth;
	//	height = intheight;
	//	// BytesperPixel 2 yuyv , 3 rgb24
	//	*pic = calloc( intwidth * (intheight + 8) * 2, sizeof(uint8_t));
	//	if(*pic == NULL)
	//	{
	//		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (alloc_v4l2_frames): %s\n", strerror(errno));
	//		exit(-1);
	//	}
	//}

	switch (dscans[0].hv)
	{
		case 0x22: // 411
			mb=6;
			mcusx = jpeg_ctx->width >> 4;
			mcusy = jpeg_ctx->height >> 4;
			bpp=2;
			xpitch = 16 * bpp;
			pitch = jpeg_ctx->width * bpp; // YUYV out
			ypitch = 16 * pitch;
			convert = yuv420pto422; //choose the right conversion function
			break;
		case 0x21: //422
			mb=4;
			mcusx = jpeg_ctx->width >> 4;
			mcusy = jpeg_ctx->height >> 3;
			bpp=2;
			xpitch = 16 * bpp;
			pitch = jpeg_ctx->width * bpp; // YUYV out
			ypitch = 8 * pitch;
			convert = yuv422pto422; //choose the right conversion function
			break;
		case 0x11: //444
			mcusx = jpeg_ctx->width >> 3;
			mcusy = jpeg_ctx->height >> 3;
			bpp=2;
			xpitch = 8 * bpp;
			pitch = jpeg_ctx->width * bpp; // YUYV out
			ypitch = 8 * pitch;
			if (info.ns==1)
			{
				mb = 1;
				convert = yuv400pto422; //choose the right conversion function
			}
			else
			{
				mb=3;
				convert = yuv444pto422; //choose the right conversion function
			}
			break;
		default:
			err = E_NOT_YCBCR_ERR;
			goto error;
			break;
	}

	idctqtab(quant[dscans[0].tq], decdata->dquant[0]);
	idctqtab(quant[dscans[1].tq], decdata->dquant[1]);
	idctqtab(quant[dscans[2].tq], decdata->dquant[2]);
	setinput(&inp, datap);
	dec_initscans();

	dscans[0].next = 2;
	dscans[1].next = 1;
	dscans[2].next = 0;	/* 4xx encoding */
	for (my = 0,y=0; my < mcusy; my++,y+=ypitch)
	{
		for (mx = 0,x=0; mx < mcusx; mx++,x+=xpitch)
		{
			if (info.dri && !--info.nm)
				if (dec_checkmarker())
				{
					err = E_WRONG_MARKER_ERR;
					goto error;
				}
			switch (mb)
			{
				case 6:
					decode_mcus(&inp, decdata->dcts, mb, dscans, max);
					idct(decdata->dcts, decdata->out, decdata->dquant[0],
						IFIX(128.5), max[0]);
					idct(decdata->dcts + 64, decdata->out + 64,
						decdata->dquant[0], IFIX(128.5), max[1]);
					idct(decdata->dcts + 128, decdata->out + 128,
						decdata->dquant[0], IFIX(128.5), max[2]);
					idct(decdata->dcts + 192, decdata->out + 192,
						decdata->dquant[0], IFIX(128.5), max[3]);
					idct(decdata->dcts + 256, decdata->out + 256,
						decdata->dquant[1], IFIX(0.5), max[4]);
					idct(decdata->dcts + 320, decdata->out + 320,
						decdata->dquant[2], IFIX(0.5), max[5]);
					break;

				case 4:
					decode_mcus(&inp, decdata->dcts, mb, dscans, max);
					idct(decdata->dcts, decdata->out, decdata->dquant[0],
						IFIX(128.5), max[0]);
					idct(decdata->dcts + 64, decdata->out + 64,
						decdata->dquant[0], IFIX(128.5), max[1]);
					idct(decdata->dcts + 128, decdata->out + 256,
							decdata->dquant[1], IFIX(0.5), max[4]);
					idct(decdata->dcts + 192, decdata->out + 320,
						decdata->dquant[2], IFIX(0.5), max[5]);
					break;

				case 3:
					decode_mcus(&inp, decdata->dcts, mb, dscans, max);
					idct(decdata->dcts, decdata->out, decdata->dquant[0],
						IFIX(128.5), max[0]);
					idct(decdata->dcts + 64, decdata->out + 256,
						decdata->dquant[1], IFIX(0.5), max[4]);
					idct(decdata->dcts + 128, decdata->out + 320,
						decdata->dquant[2], IFIX(0.5), max[5]);
					break;

				case 1:
					decode_mcus(&inp, decdata->dcts, mb, dscans, max);
					idct(decdata->dcts, decdata->out, decdata->dquant[0],
						IFIX(128.5), max[0]);
					break;
			} // switch enc411
			convert(decdata->out, out_buf+y+x, pitch); //convert to 422
		}
	}

	m = dec_readmarker(&inp);
	if (m != M_EOI)
	{
		err = E_NO_EOI_ERR;
		goto error;
	}
	free(decdata);
	return 0;
error:
	free(decdata);
	return err;
}

/*
 * close (m)jpeg decoder context
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void jpeg_close_decoder()
{
	if(jpeg_ctx == NULL)
		return;

	free(jpeg_ctx->tmp_frame);
	free(jpeg_ctx);

	jpeg_ctx = NULL;
}

#else  //use libavcodec to decode mjpeg data

typedef struct _codec_data_t
{
	AVCodec *codec;
	AVCodecContext *context;
	AVFrame *picture;
} codec_data_t;

/*
 * init (m)jpeg decoder context
 * args:
 *    width - image width
 *    height - image height
 *
 * asserts:
 *    none
 *
 * returns: error code (0 - E_OK)
 */
int jpeg_init_decoder(int width, int height)
{
#if !LIBAVCODEC_VER_AT_LEAST(53,34)
    getLoadLibsInstance()->m_avcodec_init();
#endif
#if !LIBAVCODEC_VER_AT_LEAST(58,9)
	/*
	 * register all the codecs (we can also register only the codec
	 * we wish to have smaller code)
	 */
    getLoadLibsInstance()->m_avcodec_register_all();
#endif
    getAvutil()->m_av_log_set_level(AV_LOG_PANIC);

	if(jpeg_ctx != NULL)
		jpeg_close_decoder();

	jpeg_ctx = calloc(1, sizeof(jpeg_decoder_context_t));
	if(jpeg_ctx == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (jpeg_init_decoder): %s\n", strerror(errno));
		exit(-1);
	}

	codec_data_t *codec_data = calloc(1, sizeof(codec_data_t));
	if(codec_data == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (jpeg_init_decoder): %s\n", strerror(errno));
		exit(-1);
	}

    codec_data->codec = getLoadLibsInstance()->m_avcodec_find_decoder(AV_CODEC_ID_MJPEG);
	if(!codec_data->codec)
	{
		fprintf(stderr, "V4L2_CORE: (mjpeg decoder) codec not found\n");
		free(jpeg_ctx);
		free(codec_data);
		jpeg_ctx = NULL;
		return E_NO_CODEC;
	}

#if LIBAVCODEC_VER_AT_LEAST(53,6)
    codec_data->context = getLoadLibsInstance()->m_avcodec_alloc_context3(codec_data->codec);
    #if !LIBAVCODEC_VER_AT_LEAST(60,0)
    getLoadLibsInstance()->m_avcodec_get_context_defaults3(codec_data->context, codec_data->codec);
    #endif
#else
    codec_data->context = getLoadLibsInstance()->m_avcodec_alloc_context();
    getLoadLibsInstance()->m_avcodec_get_context_defaults(codec_data->context);
#endif
	if(codec_data->context == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (h264_init_decoder): %s\n", strerror(errno));
		exit(-1);
	}

	codec_data->context->pix_fmt = AV_PIX_FMT_YUV422P;
	codec_data->context->width = width;
	codec_data->context->height = height;
	//jpeg_ctx->context->dsp_mask = (FF_MM_MMX | FF_MM_MMXEXT | FF_MM_SSE);

#if LIBAVCODEC_VER_AT_LEAST(53,6)
    if (getLoadLibsInstance()->m_avcodec_open2(codec_data->context, codec_data->codec, NULL) < 0)
#else
    if (getLoadLibsInstance()->m_avcodec_open(codec_data->context, codec_data->codec) < 0)
#endif
	{
		fprintf(stderr, "V4L2_CORE: (mjpeg decoder) couldn't open codec\n");
        getLoadLibsInstance()->m_avcodec_close(codec_data->context);
		free(codec_data->context);
		free(codec_data);
		free(jpeg_ctx);
		jpeg_ctx = NULL;
		return E_NO_CODEC;
	}

#if LIBAVCODEC_VER_AT_LEAST(55,28)
    codec_data->picture = getAvutil()->m_av_frame_alloc();
    getAvutil()->m_av_frame_unref(codec_data->picture);
#else
    codec_data->picture = getLoadLibsInstance()->m_avcodec_alloc_frame();
    getLoadLibsInstance()->m_avcodec_get_frame_defaults(codec_data->picture);
#endif

	/*alloc temp buffer*/
    jpeg_ctx->tmp_frame = calloc((size_t)(width*height*2), sizeof(uint8_t));
	if(jpeg_ctx->tmp_frame == NULL)
	{
		fprintf(stderr, "V4L2_CORE: FATAL memory allocation failure (jpeg_init_decoder): %s\n", strerror(errno));
		exit(-1);
	}
#if LIBAVUTIL_VER_AT_LEAST(54,6)
    jpeg_ctx->pic_size = getAvutil()->m_av_image_get_buffer_size(codec_data->context->pix_fmt, width, height, 1);
#else
	jpeg_ctx->pic_size = avpicture_get_size(codec_data->context->pix_fmt, width, height);
#endif
	jpeg_ctx->width = width;
	jpeg_ctx->height = height;
	jpeg_ctx->codec_data = codec_data;

	return E_OK;
}

/*
 * decode (m)jpeg frame
 * args:
 *    out_buf - pointer to decoded data
 *    in_buf - pointer to h264 data
 *    size - in_buf size
 *
 * asserts:
 *    jpeg_ctx is not null
 *    in_buf is not null
 *    out_buf is not null
 *
 * returns: decoded data size
 */
int jpeg_decode(uint8_t *out_buf, uint8_t *in_buf, int size)
{
	/*asserts*/
	assert(jpeg_ctx != NULL);
	assert(in_buf != NULL);
	assert(out_buf != NULL);

	AVPacket avpkt;

    getLoadLibsInstance()->m_av_init_packet(&avpkt);

	avpkt.size = size;
	avpkt.data = in_buf;

	codec_data_t *codec_data = (codec_data_t *) jpeg_ctx->codec_data;

	int got_frame = 0;
	int ret = libav_decode(codec_data->context, codec_data->picture, &got_frame, &avpkt);

	if(ret < 0)
	{
		fprintf(stderr, "V4L2_CORE: (jpeg decoder) error while decoding frame\n");
		return ret;
	}

	if(got_frame)
	{
#if LIBAVUTIL_VER_AT_LEAST(54,6)
        getAvutil()->m_av_image_copy_to_buffer(jpeg_ctx->tmp_frame, jpeg_ctx->pic_size,
                             (const uint8_t * const*) codec_data->picture->data, codec_data->picture->linesize,
                             codec_data->context->pix_fmt, jpeg_ctx->width, jpeg_ctx->height, 1);
#else
		avpicture_layout((AVPicture *) codec_data->picture, codec_data->context->pix_fmt,
			jpeg_ctx->width, jpeg_ctx->height, jpeg_ctx->tmp_frame, jpeg_ctx->pic_size);
#endif

        if (codec_data->context->pix_fmt == AV_PIX_FMT_YUVJ422P) {
            /* libavcodec output is in yuvj422p */

            yuv422p_to_yu12(out_buf, jpeg_ctx->tmp_frame, jpeg_ctx->width, jpeg_ctx->height);
            return jpeg_ctx->pic_size;
        } else if (codec_data->context->pix_fmt == AV_PIX_FMT_YUVJ420P) {
            /* libavcodec output is in yuvj420p */

            if (jpeg_ctx->pic_size > (size_t)(jpeg_ctx->width * jpeg_ctx->height * 3 / 2))
                jpeg_ctx->pic_size  = (size_t)(jpeg_ctx->width * jpeg_ctx->height * 3 / 2);

            memcpy(out_buf, jpeg_ctx->tmp_frame, jpeg_ctx->pic_size);

            return jpeg_ctx->pic_size;
        }else
            return 0;
	}
	else
		return 0;

}

/*
 * close (m)jpeg decoder context
 * args:
 *    none
 *
 * asserts:
 *    none
 *
 * returns: none
 */
void jpeg_close_decoder()
{
	if(jpeg_ctx == NULL)
		return;

	codec_data_t *codec_data = (codec_data_t *) jpeg_ctx->codec_data;

    getLoadLibsInstance()->m_avcodec_close(codec_data->context);

	free(codec_data->context);

#if LIBAVCODEC_VER_AT_LEAST(55,28)
    getAvutil()->m_av_frame_free(&codec_data->picture);
#else
	#if LIBAVCODEC_VER_AT_LEAST(54,28)
            getLoadLibsInstance()->m_avcodec_free_frame(&codec_data->picture);
	#else
			av_freep(&codec_data->picture);
	#endif
#endif

	if(jpeg_ctx->tmp_frame)
		free(jpeg_ctx->tmp_frame);

	free(codec_data);
	free(jpeg_ctx);

	jpeg_ctx = NULL;
}

#endif

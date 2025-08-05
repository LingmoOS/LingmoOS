/*
 * libdpkg - Debian packaging suite library routines
 * c-ctype.c - ASCII C locale-only functions
 *
 * Copyright © 2009-2014 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <dpkg/c-ctype.h>

#undef S
#define S C_CTYPE_SPACE
#undef W
#define W C_CTYPE_WHITE
#undef B
#define B C_CTYPE_BLANK
#undef U
#define U C_CTYPE_UPPER
#undef L
#define L C_CTYPE_LOWER
#undef D
#define D C_CTYPE_DIGIT

static unsigned short int c_ctype[256] = {
/** 0 **/
	/* \0 */ 0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* \a */ 0,
	/* \b */ 0,
	/* \t */ S | W | B,
	/* \n */ S | W,
	/* \v */ S,
	/* \f */ S,
	/* \r */ S,
	0,
	0,
/** 16 **/
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
/** 32 **/
	/*   */ S | W | B,
	/* ! */ 0,
	/* " */ 0,
	/* # */ 0,
	/* $ */ 0,
	/* % */ 0,
	/* & */ 0,
	/* ' */ 0,
	/* ( */ 0,
	/* ) */ 0,
	/* * */ 0,
	/* + */ 0,
	/* , */ 0,
	/* - */ 0,
	/* . */ 0,
	/* / */ 0,
/** 48 **/
	/* 0 */ D,
	/* 1 */ D,
	/* 2 */ D,
	/* 3 */ D,
	/* 4 */ D,
	/* 5 */ D,
	/* 6 */ D,
	/* 7 */ D,
	/* 8 */ D,
	/* 9 */ D,
	/* : */ 0,
	/* ; */ 0,
	/* < */ 0,
	/* = */ 0,
	/* > */ 0,
	/* ? */ 0,
/* 64 */
	/* @ */ 0,
	/* A */ U,
	/* B */ U,
	/* C */ U,
	/* D */ U,
	/* E */ U,
	/* F */ U,
	/* G */ U,
	/* H */ U,
	/* I */ U,
	/* J */ U,
	/* K */ U,
	/* L */ U,
	/* M */ U,
	/* N */ U,
	/* O */ U,
/* 80 */
	/* P */ U,
	/* Q */ U,
	/* R */ U,
	/* S */ U,
	/* T */ U,
	/* U */ U,
	/* V */ U,
	/* W */ U,
	/* X */ U,
	/* Y */ U,
	/* Z */ U,
	/* [ */ 0,
	/* \ */ 0,
	/* ] */ 0,
	/* ^ */ 0,
	/* _ */ 0,
/* 96 */
	/* ` */ 0,
	/* a */ L,
	/* b */ L,
	/* c */ L,
	/* d */ L,
	/* e */ L,
	/* f */ L,
	/* g */ L,
	/* h */ L,
	/* i */ L,
	/* j */ L,
	/* k */ L,
	/* l */ L,
	/* m */ L,
	/* n */ L,
	/* o */ L,
/* 112 */
	/* p */ L,
	/* q */ L,
	/* r */ L,
	/* s */ L,
	/* t */ L,
	/* u */ L,
	/* v */ L,
	/* w */ L,
	/* x */ L,
	/* y */ L,
	/* z */ L,
	/* { */ 0,
	/* | */ 0,
	/* } */ 0,
	/* ~ */ 0,
	0,
/* 128 */
};

/**
 * Check if the character is bits ctype.
 */
bool
c_isbits(int c, enum c_ctype_bit bits)
{
	return ((c_ctype[(unsigned char)c] & bits) != 0);
}

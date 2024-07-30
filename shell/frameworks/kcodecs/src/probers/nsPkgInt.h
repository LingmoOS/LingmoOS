/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsPkgInt_h__
#define nsPkgInt_h__

namespace kencodingprober
{
typedef enum {
    eIdxSft4bits = 3,
    eIdxSft8bits = 2,
    eIdxSft16bits = 1,
} nsIdxSft;

typedef enum {
    eSftMsk4bits = 7,
    eSftMsk8bits = 3,
    eSftMsk16bits = 1,
} nsSftMsk;

typedef enum {
    eBitSft4bits = 2,
    eBitSft8bits = 3,
    eBitSft16bits = 4,
} nsBitSft;

typedef enum {
    eUnitMsk4bits = 0x0000000FL,
    eUnitMsk8bits = 0x000000FFL,
    eUnitMsk16bits = 0x0000FFFFL,
} nsUnitMsk;

typedef struct nsPkgInt {
    nsIdxSft idxsft;
    nsSftMsk sftmsk;
    nsBitSft bitsft;
    nsUnitMsk unitmsk;
    const unsigned int *data;
} nsPkgInt;
}

#define PCK16BITS(a, b) ((unsigned int)(((b) << 16) | (a)))

#define PCK8BITS(a, b, c, d) PCK16BITS(((unsigned int)(((b) << 8) | (a))), ((unsigned int)(((d) << 8) | (c))))

#define PCK4BITS(a, b, c, d, e, f, g, h)                                                                                                                       \
    PCK8BITS(((unsigned int)(((b) << 4) | (a))), ((unsigned int)(((d) << 4) | (c))), ((unsigned int)(((f) << 4) | (e))), ((unsigned int)(((h) << 4) | (g))))

#define GETFROMPCK(i, c) (((((c).data)[(i) >> (c).idxsft]) >> (((i) & (c).sftmsk) << (c).bitsft)) & (c).unitmsk)

#endif /* nsPkgInt_h__ */

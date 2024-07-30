/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// FIXME: should we unroll some loops?  Optimization can be done here.

/* Implementation of 16 rounds blowfish as described in:
 * _Applied_Cryptography_ (c) Bruce Schneier, 1996.
 */

#include "blowfish.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "blowfishtables.h"

// DO NOT INCLUDE THIS. IT BREAKS KWALLET.
// We need to live with -Wundef until someone really figures out the problem.
//#include <QtCore/qglobal.h> // for Q_BYTE_ORDER and friends
// Workaround for -Wundef
#define Q_BIG_ENDIAN 1
#define Q_BYTE_ORDER Q_BIG_ENDIAN

BlowFish::BlowFish()
{
    _blksz = 8;
    m_key = nullptr;
    m_initialized = false;
}

bool BlowFish::init()
{
    // Initialize the sboxes
    for (int i = 0; i < 256; i++) {
        m_S[0][i] = ks0[i];
        m_S[1][i] = ks1[i];
        m_S[2][i] = ks2[i];
        m_S[3][i] = ks3[i];
    }

    uint32_t datal = 0;
    uint32_t datar = 0;
    uint32_t data = 0;
    int j = 0;

    // Update the sboxes and pbox.
    for (int i = 0; i < 18; i++) {
        data = 0;
        for (int k = 0; k < 4; ++k) {
            data = (data << 8) | ((unsigned char *)m_key)[j++];
            if (j >= m_keylen / 8) {
                j = 0;
            }
        }
        m_P[i] = P[i] ^ data;
    }

    for (int i = 0; i < 18; i += 2) {
        encipher(&datal, &datar);
        m_P[i] = datal;
        m_P[i + 1] = datar;
    }

    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 256; i += 2) {
            encipher(&datal, &datar);
            m_S[j][i] = datal;
            m_S[j][i + 1] = datar;
        }
    }

    // Nice code from gpg's implementation...
    //     check to see if the key is weak and return error if so
    for (int i = 0; i < 255; i++) {
        for (int j = i + 1; j < 256; j++) {
            if ((m_S[0][i] == m_S[0][j]) || (m_S[1][i] == m_S[1][j]) ||
                    (m_S[2][i] == m_S[2][j]) || (m_S[3][i] == m_S[3][j])) {
                return false;
            }
        }
    }

    m_initialized = true;

    return true;
}

BlowFish::~BlowFish()
{
    delete[](unsigned char *)m_key;
    m_key = nullptr;
}

int BlowFish::keyLen() const
{
    return 448;
}

bool BlowFish::variableKeyLen() const
{
    return true;
}

bool BlowFish::readyToGo() const
{
    return m_initialized;
}

bool BlowFish::setKey(void *key, int bitlength)
{
    if (bitlength <= 0 || bitlength > 448 || bitlength % 8 != 0) {
        return false;
    }

    delete[](unsigned char *)m_key;

    m_key = new unsigned char[bitlength / 8];
    memcpy(m_key, key, bitlength / 8);
    m_keylen = bitlength;

    return init();
}
// clang-format off
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#define shuffle(x) do {             \
        uint32_t r = x;             \
        x  = (r & 0xff000000) >> 24;    \
        x |= (r & 0x00ff0000) >>  8;    \
        x |= (r & 0x0000ff00) <<  8;    \
        x |= (r & 0x000000ff) << 24;    \
    } while (0)
#endif
// clang-format on

int BlowFish::encrypt(void *block, int len)
{
    uint32_t *d = (uint32_t *)block;

    if (!m_initialized || len % _blksz != 0) {
        return -1;
    }

    for (int i = 0; i < len / _blksz; i++) {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        shuffle(*d);
        shuffle(*(d + 1));
#endif
        encipher(d, d + 1);
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        shuffle(*d);
        shuffle(*(d + 1));
#endif
        d += 2;
    }

    return len;
}

int BlowFish::decrypt(void *block, int len)
{
    uint32_t *d = (uint32_t *)block;

    if (!m_initialized || len % _blksz != 0) {
        return -1;
    }

    for (int i = 0; i < len / _blksz; i++) {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        shuffle(*d);
        shuffle(*(d + 1));
#endif
        decipher(d, d + 1);
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        shuffle(*d);
        shuffle(*(d + 1));
#endif
        d += 2;
    }

    return len;
}

uint32_t BlowFish::F(uint32_t x)
{
    unsigned short a;
    unsigned short b;
    unsigned short c;
    unsigned short d;
    uint32_t y;

    d = x & 0x000000ff;
    x >>= 8;
    c = x & 0x000000ff;
    x >>= 8;
    b = x & 0x000000ff;
    x >>= 8;
    a = x & 0x000000ff;

    y = m_S[0][a] + m_S[1][b];
    y ^= m_S[2][c];
    y += m_S[3][d];

    return y;
}

void BlowFish::encipher(uint32_t *xl, uint32_t *xr)
{
    uint32_t Xl = *xl;
    uint32_t Xr = *xr;
    uint32_t temp;

    for (int i = 0; i < 16; ++i) {
        Xl ^= m_P[i];
        Xr ^= F(Xl);
        // Exchange
        temp = Xl; Xl = Xr; Xr = temp;
    }

    // Exchange
    temp = Xl; Xl = Xr; Xr = temp;

    Xr ^= m_P[16];
    Xl ^= m_P[17];

    *xl = Xl;
    *xr = Xr;
}

void BlowFish::decipher(uint32_t *xl, uint32_t *xr)
{
    uint32_t Xl = *xl;
    uint32_t Xr = *xr;
    uint32_t temp;

    for (int i = 17; i > 1; --i) {
        Xl ^= m_P[i];
        Xr ^= F(Xl);
        // Exchange
        temp = Xl; Xl = Xr; Xr = temp;
    }

    // Exchange
    temp = Xl; Xl = Xr; Xr = temp;

    Xr ^= m_P[1];
    Xl ^= m_P[0];

    *xl = Xl;
    *xr = Xr;
}

/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _BLOWFISH_H
#define _BLOWFISH_H

#include <config-kwalletbackend.h>

#if HAVE_STDINT_H
#include <stdint.h>
#endif
#include <sys/types.h>
#if HAVE_SYS_BITYPES_H
#include <sys/bitypes.h> /* For uintXX_t on Tru64 */
#endif

#include "blockcipher.h"
#include "kwalletbackend_export.h"

/* @internal
 */
class KWALLETBACKEND_EXPORT BlowFish : public BlockCipher
{
public:
    BlowFish();
    ~BlowFish() override;

    bool setKey(void *key, int bitlength) override;

    int keyLen() const override;

    bool variableKeyLen() const override;

    bool readyToGo() const override;

    int encrypt(void *block, int len) override;

    int decrypt(void *block, int len) override;

private:
    uint32_t m_S[4][256];
    uint32_t m_P[18];

    void *m_key;
    int m_keylen; // in bits

    bool m_initialized;

    bool init();
    uint32_t F(uint32_t x);
    void encipher(uint32_t *xl, uint32_t *xr);
    void decipher(uint32_t *xl, uint32_t *xr);
};

#endif

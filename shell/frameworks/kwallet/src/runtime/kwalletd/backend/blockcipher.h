/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BLOCKCIPHER_H
#define BLOCKCIPHER_H

#include "kwalletbackend_export.h"

/* @internal
 */
class KWALLETBACKEND_EXPORT BlockCipher
{
public:
    BlockCipher();
    virtual ~BlockCipher();

    /*
     *  Return the current blocksize in bytes.
     */
    int blockSize() const;

    /*
     *  Set the encryption key to key.  Return true on success.
     */
    virtual bool setKey(void *key, int bitlength) = 0;

    /*
     *  Get the required (or if it's variable, then the maximum) key
     *  length for this cipher in bits.
     */
    virtual int keyLen() const = 0;

    /*
     *  True if the key is of a variable length.  In this case,
     *  getKeyLen() will return the maximum length.
     */
    virtual bool variableKeyLen() const = 0;

    /*
     *  True if all settings are good and we are ready to encrypt.
     */
    virtual bool readyToGo() const = 0;

    /*
     *  Encrypt the block.  Returns the number of bytes successfully
     *  encrypted.  Can return -1 on error.
     */
    virtual int encrypt(void *block, int len) = 0;

    /*
     *  Decrypt the block.  Returns as does encrypt();
     */
    virtual int decrypt(void *block, int len) = 0;

protected:
    int _blksz;
    int _keylen; // in bits
};

#endif // BLOCKCIPHER_H

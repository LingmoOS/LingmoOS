/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __CBC__KO__H
#define __CBC__KO__H

#include "blockcipher.h"

/* @internal
 *   Initialize this class with a pointer to a valid, uninitialized BlockCipher
 *   and it will apply that cipher using CBC.  You may want to make the
 *   initial block a full block of random data.  Do not change the block size
 *   at any time!!  You must pad it yourself.  Also, you can only encrypt or
 *   decrypt.  You can't do both with a given instance.  After you call one,
 *   calls to the other will fail in this instance.
 */

class CipherBlockChain : public BlockCipher
{
public:
    CipherBlockChain(BlockCipher *cipher, bool useECBforReading = false);
    ~CipherBlockChain() override;

    bool setKey(void *key, int bitlength) override;

    int keyLen() const override;

    bool variableKeyLen() const override;

    bool readyToGo() const override;

    int encrypt(void *block, int len) override;

    int decrypt(void *block, int len) override;

private:
    void initRegister();
    int decryptECB(void *block, int len);

    BlockCipher *_cipher;
    void *_register;
    void *_next;
    int _len;
    int _reader, _writer;
    bool _useECBforReading;
};

#endif

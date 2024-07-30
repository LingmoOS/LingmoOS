/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 George Staikos <staikos@kde.org>

    Based heavily on SHA1 code from GPG 1.0.3:
    SPDX-FileCopyrightText: 1998 FSF

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __sha1__ko__h
#define __sha1__ko__h

#include "kwalletbackend_export.h"

/* @internal
 */
class KWALLETBACKEND_EXPORT SHA1
{
public:
    SHA1();
    ~SHA1();

    /*
     *  The number of bits in the hash generated.
     */
    int size() const;

    /*
     *  True if all settings are good and we are ready to hash.
     */
    bool readyToGo() const;

    /*
     *  Process a block of data for the hash function.
     */
    int process(const void *block, int len);

    /*
     *  Return the digest as a 20 byte array reference.
     *  Calling this makes readyToGo() == false.
     */
    const unsigned char *hash();

    /*
     *  Reset the digest so a new one can be calculated.
     */
    int reset();

protected:
    int _hashlen;
    bool _init;

    long _h0, _h1, _h2, _h3, _h4;
    long _nblocks;
    int _count;
    unsigned char _buf[64];
    void transform(void *data);
};

#endif

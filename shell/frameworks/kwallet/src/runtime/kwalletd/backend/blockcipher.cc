/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "blockcipher.h"

BlockCipher::BlockCipher()
{
    _blksz = -1;
}

BlockCipher::~BlockCipher()
{
}

int BlockCipher::blockSize() const
{
    return _blksz;
}


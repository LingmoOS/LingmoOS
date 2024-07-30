/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>
    SPDX-FileCopyrightText: 2008 Wang Kai <zealot.kai@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef nsUniversalDetector_h__
#define nsUniversalDetector_h__

#include "nsCharSetProber.h"

#define NUM_OF_CHARSET_PROBERS 3

namespace kencodingprober
{
typedef enum {
    ePureAscii = 0,
    eEscAscii = 1,
    eHighbyte = 2,
} nsInputState;

class KCODECS_NO_EXPORT nsUniversalDetector : public nsCharSetProber
{
public:
    nsUniversalDetector();
    ~nsUniversalDetector() override;
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override;
    void Reset(void) override;
    float GetConfidence(void) override;
    nsProbingState GetState() override;
    void SetOpion() override
    {
    }

protected:
    nsInputState mInputState;
    bool mDone;
    bool mInTag;
    bool mStart;
    bool mGotData;
    char mLastChar;
    const char *mDetectedCharset;
    int mBestGuess;

    nsCharSetProber *mCharSetProbers[NUM_OF_CHARSET_PROBERS];
    nsCharSetProber *mEscCharSetProber;
};
}

#endif

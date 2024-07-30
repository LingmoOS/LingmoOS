/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef CHINESEGROUPPROBER_H
#define CHINESEGROUPPROBER_H

#include "nsCharSetProber.h"

#define CN_NUM_OF_PROBERS 3
namespace kencodingprober
{
class KCODECS_NO_EXPORT ChineseGroupProber : public nsCharSetProber
{
public:
    ChineseGroupProber();
    ~ChineseGroupProber() override;
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override;
    nsProbingState GetState(void) override
    {
        return mState;
    }
    void Reset(void) override;
    float GetConfidence(void) override;
    void SetOpion() override
    {
    }

#ifdef DEBUG_PROBE
    void DumpStatus() override;
#endif

protected:
    nsProbingState mState;
    nsCharSetProber *mProbers[CN_NUM_OF_PROBERS];
    bool mIsActive[CN_NUM_OF_PROBERS];
    int mBestGuess;
    unsigned int mActiveNum;
};
}
#endif /* CHINESEGROUPPROBER_H */

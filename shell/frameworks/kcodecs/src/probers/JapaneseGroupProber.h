/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef JAPANESEGROUPPROBER_H
#define JAPANESEGROUPPROBER_H

#include "UnicodeGroupProber.h"
#include "nsCharSetProber.h"
#include "nsEUCJPProber.h"
#include "nsSJISProber.h"

#define JP_NUM_OF_PROBERS 3
namespace kencodingprober
{
class KCODECS_NO_EXPORT JapaneseGroupProber : public nsCharSetProber
{
public:
    JapaneseGroupProber();
    ~JapaneseGroupProber() override;
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
    nsCharSetProber *mProbers[JP_NUM_OF_PROBERS];
    bool mIsActive[JP_NUM_OF_PROBERS];
    int mBestGuess;
    unsigned int mActiveNum;
};
}
#endif /* JAPANESEGROUPPROBER_H */

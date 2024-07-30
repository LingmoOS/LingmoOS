/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsMBCSGroupProber_h__
#define nsMBCSGroupProber_h__

#include "UnicodeGroupProber.h"
#include "nsBig5Prober.h"
#include "nsEUCJPProber.h"
#include "nsEUCKRProber.h"
#include "nsGB2312Prober.h"
#include "nsSJISProber.h"

#define NUM_OF_PROBERS 6
namespace kencodingprober
{
class KCODECS_NO_EXPORT nsMBCSGroupProber : public nsCharSetProber
{
public:
    nsMBCSGroupProber();
    ~nsMBCSGroupProber() override;
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
    nsCharSetProber *mProbers[NUM_OF_PROBERS];
    bool mIsActive[NUM_OF_PROBERS];
    int mBestGuess;
    unsigned int mActiveNum;
};
}

#endif /* nsMBCSGroupProber_h__ */

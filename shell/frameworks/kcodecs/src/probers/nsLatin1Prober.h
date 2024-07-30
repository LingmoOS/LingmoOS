/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsLatin1Prober_h__
#define nsLatin1Prober_h__

#include "nsCharSetProber.h"

#define FREQ_CAT_NUM 4
namespace kencodingprober
{
class KCODECS_NO_EXPORT nsLatin1Prober : public nsCharSetProber
{
public:
    nsLatin1Prober(void)
    {
        Reset();
    }
    ~nsLatin1Prober(void) override
    {
    }
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return "windows-1252";
    }
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
    char mLastCharClass;
    unsigned int mFreqCounter[FREQ_CAT_NUM];
};
}

#endif /* nsLatin1Prober_h__ */

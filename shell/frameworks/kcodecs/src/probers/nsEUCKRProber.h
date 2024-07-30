/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsEUCKRProber_h__
#define nsEUCKRProber_h__

#include "CharDistribution.h"
#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"
namespace kencodingprober
{
class KCODECS_NO_EXPORT nsEUCKRProber : public nsCharSetProber
{
public:
    nsEUCKRProber(void)
    {
        mCodingSM = new nsCodingStateMachine(&EUCKRSMModel);
        Reset();
    }
    ~nsEUCKRProber(void) override
    {
        delete mCodingSM;
    }
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return "EUC-KR";
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

protected:
    void GetDistribution(unsigned int aCharLen, const char *aStr);

    nsCodingStateMachine *mCodingSM;
    nsProbingState mState;

    // EUCKRContextAnalysis mContextAnalyser;
    EUCKRDistributionAnalysis mDistributionAnalyser;
    char mLastChar[2];
};
}

#endif /* nsEUCKRProber_h__ */

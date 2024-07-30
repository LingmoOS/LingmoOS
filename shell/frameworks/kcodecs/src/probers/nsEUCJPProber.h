/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

// for S-JIS encoding, observe characteristic:
// 1, kana character (or hankaku?) often have high frequency of appearance
// 2, kana character often exist in group
// 3, certain combination of kana is never used in japanese language

#ifndef nsEUCJPProber_h__
#define nsEUCJPProber_h__

#include "CharDistribution.h"
#include "JpCntx.h"
#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"
namespace kencodingprober
{
class KCODECS_NO_EXPORT nsEUCJPProber : public nsCharSetProber
{
public:
    nsEUCJPProber(void)
    {
        mCodingSM = new nsCodingStateMachine(&EUCJPSMModel);
        Reset();
    }
    ~nsEUCJPProber(void) override
    {
        delete mCodingSM;
    }
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return "EUC-JP";
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
    nsCodingStateMachine *mCodingSM;
    nsProbingState mState;

    EUCJPContextAnalysis mContextAnalyser;
    EUCJPDistributionAnalysis mDistributionAnalyser;

    char mLastChar[2];
};
}

#endif /* nsEUCJPProber_h__ */

/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsEscCharSetProber_h__
#define nsEscCharSetProber_h__

#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"

#define NUM_OF_ESC_CHARSETS 4
namespace kencodingprober
{
class KCODECS_NO_EXPORT nsEscCharSetProber : public nsCharSetProber
{
public:
    nsEscCharSetProber(void);
    ~nsEscCharSetProber(void) override;
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return mDetectedCharset;
    }
    nsProbingState GetState(void) override
    {
        return mState;
    }
    void Reset(void) override;
    float GetConfidence(void) override
    {
        return (float)0.99;
    }
    void SetOpion() override
    {
    }

protected:
    void GetDistribution(unsigned int aCharLen, const char *aStr);

    nsCodingStateMachine *mCodingSM[NUM_OF_ESC_CHARSETS];
    unsigned int mActiveSM;
    nsProbingState mState;
    const char *mDetectedCharset;
};
}
#endif /* nsEscCharSetProber_h__ */

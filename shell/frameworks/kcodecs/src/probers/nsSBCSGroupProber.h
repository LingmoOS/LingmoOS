/*
    The Original Code is Mozilla Universal charset detector code.

    SPDX-FileCopyrightText: 2001 Netscape Communications Corporation
    SPDX-FileContributor: Shy Shalom <shooshX@gmail.com>

    SPDX-License-Identifier: MPL-1.1 OR GPL-2.0-or-later OR LGPL-2.1-or-later
*/

#ifndef nsSBCSGroupProber_h__
#define nsSBCSGroupProber_h__

#include "nsCharSetProber.h"

#define NUM_OF_SBCS_PROBERS 14

namespace kencodingprober
{
class KCODECS_NO_EXPORT nsSBCSGroupProber : public nsCharSetProber
{
public:
    nsSBCSGroupProber();
    ~nsSBCSGroupProber() override;
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
    nsCharSetProber *mProbers[NUM_OF_SBCS_PROBERS];
    bool mIsActive[NUM_OF_SBCS_PROBERS];
    int mBestGuess;
    unsigned int mActiveNum;
};
}

#endif /* nsSBCSGroupProber_h__ */

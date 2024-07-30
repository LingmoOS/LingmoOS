/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#include "nsEscCharsetProber.h"

namespace kencodingprober
{
nsEscCharSetProber::nsEscCharSetProber(void)
{
    mCodingSM[0] = new nsCodingStateMachine(&HZSMModel);
    mCodingSM[1] = new nsCodingStateMachine(&ISO2022CNSMModel);
    mCodingSM[2] = new nsCodingStateMachine(&ISO2022JPSMModel);
    mCodingSM[3] = new nsCodingStateMachine(&ISO2022KRSMModel);
    mActiveSM = NUM_OF_ESC_CHARSETS;
    mState = eDetecting;
    mDetectedCharset = nullptr;
}

nsEscCharSetProber::~nsEscCharSetProber(void)
{
    for (unsigned int i = 0; i < NUM_OF_ESC_CHARSETS; i++) {
        delete mCodingSM[i];
    }
}

void nsEscCharSetProber::Reset(void)
{
    mState = eDetecting;
    for (unsigned int i = 0; i < NUM_OF_ESC_CHARSETS; i++) {
        mCodingSM[i]->Reset();
    }
    mActiveSM = NUM_OF_ESC_CHARSETS;
    mDetectedCharset = nullptr;
}

nsProbingState nsEscCharSetProber::HandleData(const char *aBuf, unsigned int aLen)
{
    nsSMState codingState;
    int j;
    unsigned int i;

    for (i = 0; i < aLen && mState == eDetecting; i++) {
        for (j = mActiveSM - 1; j >= 0; j--) {
            // byte is feed to all active state machine
            codingState = mCodingSM[j]->NextState(aBuf[i]);
            if (codingState == eError) {
                // got negative answer for this state machine, make it inactive
                mActiveSM--;
                if (mActiveSM == 0) {
                    mState = eNotMe;
                    return mState;
                } else if (j != (int)mActiveSM) {
                    nsCodingStateMachine *t;
                    t = mCodingSM[mActiveSM];
                    mCodingSM[mActiveSM] = mCodingSM[j];
                    mCodingSM[j] = t;
                }
            } else if (codingState == eItsMe) {
                mState = eFoundIt;
                mDetectedCharset = mCodingSM[j]->GetCodingStateMachine();
                return mState;
            }
        }
    }

    return mState;
}
}

/*  -*- C++ -*-
    SPDX-FileCopyrightText: 2008 Wang Kai <wkai@gmail.com>

    SPDX-License-Identifier: MIT
*/

#include "UnicodeGroupProber.h"

#include <QChar>
#include <math.h>

namespace kencodingprober
{
UnicodeGroupProber::UnicodeGroupProber(void)
{
    mCodingSM[0] = new nsCodingStateMachine(&UTF8SMModel);
    mCodingSM[1] = new nsCodingStateMachine(&UCS2LESMModel);
    mCodingSM[2] = new nsCodingStateMachine(&UCS2BESMModel);
    mActiveSM = NUM_OF_UNICODE_CHARSETS;
    mState = eDetecting;
    mDetectedCharset = "UTF-8";
}

UnicodeGroupProber::~UnicodeGroupProber(void)
{
    for (unsigned int i = 0; i < NUM_OF_UNICODE_CHARSETS; i++) {
        delete mCodingSM[i];
    }
}

void UnicodeGroupProber::Reset(void)
{
    mState = eDetecting;
    for (unsigned int i = 0; i < NUM_OF_UNICODE_CHARSETS; i++) {
        mCodingSM[i]->Reset();
    }
    mActiveSM = NUM_OF_UNICODE_CHARSETS;
    mDetectedCharset = "UTF-8";
}

nsProbingState UnicodeGroupProber::HandleData(const char *aBuf, unsigned int aLen)
{
    nsSMState codingState;
    static bool disableUTF16LE = false;
    static bool disableUTF16BE = false;

    if (mActiveSM == 0 || aLen < 2) {
        mState = eNotMe;
        return mState;
    }

    if (!(disableUTF16LE || disableUTF16BE)) {
        if (aLen % 2 != 0) {
            disableUTF16LE = true;
            disableUTF16BE = true;
        }
        const uint weight_BOM = sqrt((double)aLen) + aLen / 10.0;
        uint counts[5] = {0, 0, 0, 0, 0};
        for (uint i = 0; i < 5; i++) {
            counts[i] = std::count(aBuf, aBuf + aLen, char(i));
        }
        const double weight_zero = (2.0 * (counts[0] + counts[1] + counts[2] + counts[3] + counts[4]) + weight_BOM) / aLen;
        if (weight_zero < log(1.4142)) {
            disableUTF16LE = true;
            disableUTF16BE = true;
        }
        if (4 >= aBuf[1] && aBuf[1] >= 0 && QChar::isPrint(static_cast<uint>(aBuf[0]))) {
            disableUTF16BE = true;
        } else {
            disableUTF16LE = true;
        }
        if (disableUTF16BE) {
            mActiveSM--;
        }
        if (disableUTF16LE) {
            nsCodingStateMachine *t;
            t = mCodingSM[1];
            mCodingSM[1] = mCodingSM[2];
            mCodingSM[2] = t;
            mActiveSM--;
        }
    }

    for (uint i = 0; i < aLen; ++i) {
        for (int j = mActiveSM - 1; j >= 0; --j) {
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
            } else if (mState == eDetecting) {
                mDetectedCharset = mCodingSM[j]->GetCodingStateMachine();
            };
        }
    }
    return mState;
}

float UnicodeGroupProber::GetConfidence()
{
    if (mState == eFoundIt) {
        return 0.99f;
    } else {
        return 0.0f;
    }
}

#ifdef DEBUG_PROBE
void UnicodeGroupProber::DumpStatus()
{
    GetConfidence();
    for (uint i = 0; i < mActiveSM; i++) {
        qDebug() << "Unicode group" << mCodingSM[i]->DumpCurrentState() << mCodingSM[i]->GetCodingStateMachine();
    }
}
#endif

}

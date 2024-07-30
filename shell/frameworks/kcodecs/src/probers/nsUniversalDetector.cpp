/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>
    SPDX-FileCopyrightText: 2008 Wang Kai <wkai@gmail.com>

    SPDX-License-Identifier: MIT
*/

#include "nsUniversalDetector.h"

#include "nsEscCharsetProber.h"
#include "nsLatin1Prober.h"
#include "nsMBCSGroupProber.h"
#include "nsSBCSGroupProber.h"

namespace kencodingprober
{
nsUniversalDetector::nsUniversalDetector()
{
    mDone = false;
    mBestGuess = -1; // illegal value as signal
    mInTag = false;
    mEscCharSetProber = nullptr;

    mStart = true;
    mDetectedCharset = nullptr;
    mGotData = false;
    mInputState = ePureAscii;
    mLastChar = '\0';

    unsigned int i;
    for (i = 0; i < NUM_OF_CHARSET_PROBERS; i++) {
        mCharSetProbers[i] = nullptr;
    }
}

nsUniversalDetector::~nsUniversalDetector()
{
    for (int i = 0; i < NUM_OF_CHARSET_PROBERS; i++) {
        delete mCharSetProbers[i];
    }
    delete mEscCharSetProber;
}

void nsUniversalDetector::Reset()
{
    mDone = false;
    mBestGuess = -1; // illegal value as signal
    mInTag = false;

    mStart = true;
    mDetectedCharset = nullptr;
    mGotData = false;
    mInputState = ePureAscii;
    mLastChar = '\0';

    if (mEscCharSetProber) {
        mEscCharSetProber->Reset();
    }

    unsigned int i;
    for (i = 0; i < NUM_OF_CHARSET_PROBERS; i++) {
        if (mCharSetProbers[i]) {
            mCharSetProbers[i]->Reset();
        }
    }
}

//---------------------------------------------------------------------
#define SHORTCUT_THRESHOLD (float)0.95
#define MINIMUM_THRESHOLD (float)0.20

nsProbingState nsUniversalDetector::HandleData(const char *aBuf, unsigned int aLen)
{
    if (mDone) {
        return eFoundIt;
    }

    if (aLen > 0) {
        mGotData = true;
    }

    unsigned int i;
    for (i = 0; i < aLen; i++) {
        // other than 0xa0, if every other character is ascii, the page is ascii
        if (aBuf[i] & '\x80' && aBuf[i] != '\xA0') { // Since many Ascii only page contains NBSP
            // we got a non-ascii byte (high-byte)
            if (mInputState != eHighbyte) {
                // adjust state
                mInputState = eHighbyte;

                // kill mEscCharSetProber if it is active
                delete mEscCharSetProber;
                mEscCharSetProber = nullptr;

                // start multibyte and singlebyte charset prober
                if (nullptr == mCharSetProbers[0]) {
                    mCharSetProbers[0] = new nsMBCSGroupProber;
                }
                if (nullptr == mCharSetProbers[1]) {
                    mCharSetProbers[1] = new nsSBCSGroupProber;
                }
                if (nullptr == mCharSetProbers[2]) {
                    mCharSetProbers[2] = new nsLatin1Prober;
                }
            }
        } else {
            // ok, just pure ascii so far
            if (ePureAscii == mInputState && (aBuf[i] == '\033' || (aBuf[i] == '{' && mLastChar == '~'))) {
                // found escape character or HZ "~{"
                mInputState = eEscAscii;
            }

            mLastChar = aBuf[i];
        }
    }

    nsProbingState st = eDetecting;
    switch (mInputState) {
    case eEscAscii:
        if (nullptr == mEscCharSetProber) {
            mEscCharSetProber = new nsEscCharSetProber;
        }
        st = mEscCharSetProber->HandleData(aBuf, aLen);
        if (st == eFoundIt) {
            mDone = true;
            mDetectedCharset = mEscCharSetProber->GetCharSetName();
        }
        break;
    case eHighbyte:
        for (i = 0; i < NUM_OF_CHARSET_PROBERS; ++i) {
            st = mCharSetProbers[i]->HandleData(aBuf, aLen);
            if (st == eFoundIt) {
                mDone = true;
                mDetectedCharset = mCharSetProbers[i]->GetCharSetName();
            }
        }
        break;

    default: // pure ascii
        mDetectedCharset = "UTF-8";
    }
    return st;
}

//---------------------------------------------------------------------
const char *nsUniversalDetector::GetCharSetName()
{
    if (mDetectedCharset) {
        return mDetectedCharset;
    }
    switch (mInputState) {
    case eHighbyte: {
        float proberConfidence;
        float maxProberConfidence = (float)0.0;
        int maxProber = 0;

        for (int i = 0; i < NUM_OF_CHARSET_PROBERS; i++) {
            proberConfidence = mCharSetProbers[i]->GetConfidence();
            if (proberConfidence > maxProberConfidence) {
                maxProberConfidence = proberConfidence;
                maxProber = i;
            }
        }
        // do not report anything because we are not confident of it, that's in fact a negative answer
        if (maxProberConfidence > MINIMUM_THRESHOLD) {
            return mCharSetProbers[maxProber]->GetCharSetName();
        }
    }
    case eEscAscii:
        break;
    default: // pure ascii
             ;
    }
    return "UTF-8";
}

//---------------------------------------------------------------------
float nsUniversalDetector::GetConfidence()
{
    if (!mGotData) {
        // we haven't got any data yet, return immediately
        // caller program sometimes call DataEnd before anything has been sent to detector
        return MINIMUM_THRESHOLD;
    }
    if (mDetectedCharset) {
        return 0.99f;
    }
    switch (mInputState) {
    case eHighbyte: {
        float proberConfidence;
        float maxProberConfidence = (float)0.0;
        int maxProber = 0;

        for (int i = 0; i < NUM_OF_CHARSET_PROBERS; i++) {
            proberConfidence = mCharSetProbers[i]->GetConfidence();
            if (proberConfidence > maxProberConfidence) {
                maxProberConfidence = proberConfidence;
                maxProber = i;
            }
        }
        // do not report anything because we are not confident of it, that's in fact a negative answer
        if (maxProberConfidence > MINIMUM_THRESHOLD) {
            return mCharSetProbers[maxProber]->GetConfidence();
        }
    }
    case eEscAscii:
        break;
    default: // pure ascii
             ;
    }
    return MINIMUM_THRESHOLD;
}

nsProbingState nsUniversalDetector::GetState()
{
    if (mDone) {
        return eFoundIt;
    } else {
        return eDetecting;
    }
}
}

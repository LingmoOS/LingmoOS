/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2008 Wang Hoi <zealot.hoi@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kencodingprober.h"

#include "probers/ChineseGroupProber.h"
#include "probers/JapaneseGroupProber.h"
#include "probers/UnicodeGroupProber.h"
#include "probers/nsCharSetProber.h"
#include "probers/nsMBCSGroupProber.h"
#include "probers/nsSBCSGroupProber.h"
#include "probers/nsUniversalDetector.h"

#include <string.h>

class KEncodingProberPrivate
{
public:
    KEncodingProberPrivate()
        : mProber(nullptr)
        , mStart(true)
    {
    }
    ~KEncodingProberPrivate()
    {
        delete mProber;
    }
    void setProberType(KEncodingProber::ProberType pType)
    {
        mProberType = pType;
        /* handle multi-byte encodings carefully , because they're hard to detect,
         *   and have to use some Stastics methods.
         * for single-byte encodings (most western encodings), nsSBCSGroupProber is ok,
         *   because encoding state machine can detect many such encodings.
         */

        delete mProber;

        switch (mProberType) {
        case KEncodingProber::None:
            mProber = nullptr;
            break;
        case KEncodingProber::Arabic:
        case KEncodingProber::Baltic:
        case KEncodingProber::CentralEuropean:
        case KEncodingProber::Cyrillic:
        case KEncodingProber::Greek:
        case KEncodingProber::Hebrew:
        case KEncodingProber::NorthernSaami:
        case KEncodingProber::Other:
        case KEncodingProber::SouthEasternEurope:
        case KEncodingProber::Thai:
        case KEncodingProber::Turkish:
        case KEncodingProber::WesternEuropean:
            mProber = new kencodingprober::nsSBCSGroupProber();
            break;
        case KEncodingProber::ChineseSimplified:
        case KEncodingProber::ChineseTraditional:
            mProber = new kencodingprober::ChineseGroupProber();
            break;
        case KEncodingProber::Japanese:
            mProber = new kencodingprober::JapaneseGroupProber();
            break;
        case KEncodingProber::Korean:
            mProber = new kencodingprober::nsMBCSGroupProber();
            break;
        case KEncodingProber::Unicode:
            mProber = new kencodingprober::UnicodeGroupProber();
            break;
        case KEncodingProber::Universal:
            mProber = new kencodingprober::nsUniversalDetector();
            break;
        default:
            mProber = nullptr;
        }
    }
    void unicodeTest(const char *aBuf, int aLen)
    {
        if (mStart) {
            mStart = false;
            if (aLen > 3) {
                switch (aBuf[0]) {
                case '\xEF':
                    if (('\xBB' == aBuf[1]) && ('\xBF' == aBuf[2]))
                    // EF BB BF  UTF-8 encoded BOM
                    {
                        mProberState = KEncodingProber::FoundIt;
                    }
                    break;
                case '\xFE':
                    if (('\xFF' == aBuf[1]) && ('\x00' == aBuf[2]) && ('\x00' == aBuf[3]))
                    // FE FF 00 00  UCS-4, unusual octet order BOM (3412)
                    {
                        mProberState = KEncodingProber::FoundIt;
                    } else if ('\xFF' == aBuf[1])
                    // FE FF  UTF-16, big endian BOM
                    {
                        mProberState = KEncodingProber::FoundIt;
                    }
                    break;
                case '\x00':
                    if (('\x00' == aBuf[1]) && ('\xFE' == aBuf[2]) && ('\xFF' == aBuf[3]))
                    // 00 00 FE FF  UTF-32, big-endian BOM
                    {
                        mProberState = KEncodingProber::FoundIt;
                    } else if (('\x00' == aBuf[1]) && ('\xFF' == aBuf[2]) && ('\xFE' == aBuf[3]))
                    // 00 00 FF FE  UCS-4, unusual octet order BOM (2143)
                    {
                        mProberState = KEncodingProber::FoundIt;
                    }
                    break;
                case '\xFF':
                    if (('\xFE' == aBuf[1]) && ('\x00' == aBuf[2]) && ('\x00' == aBuf[3]))
                    // FF FE 00 00  UTF-32, little-endian BOM
                    {
                        mProberState = KEncodingProber::FoundIt;
                    } else if ('\xFE' == aBuf[1])
                    // FF FE  UTF-16, little endian BOM
                    {
                        mProberState = KEncodingProber::FoundIt;
                    }
                    break;
                } // switch
            }
        }
    }
    KEncodingProber::ProberType mProberType;
    KEncodingProber::ProberState mProberState;
    kencodingprober::nsCharSetProber *mProber;
    bool mStart;
};

KEncodingProber::KEncodingProber(KEncodingProber::ProberType proberType)
    : d(new KEncodingProberPrivate())
{
    setProberType(proberType);
}

KEncodingProber::~KEncodingProber() = default;

void KEncodingProber::reset()
{
    d->mProberState = KEncodingProber::Probing;
    d->mStart = true;
}

KEncodingProber::ProberState KEncodingProber::feed(QByteArrayView data)
{
    if (!d->mProber) {
        return d->mProberState;
    }
    if (d->mProberState == Probing) {
        if (d->mStart) {
            d->unicodeTest(data.constData(), data.size());
            if (d->mProberState == FoundIt) {
                return d->mProberState;
            }
        }
        d->mProber->HandleData(data.constData(), data.size());
        switch (d->mProber->GetState()) {
        case kencodingprober::eNotMe:
            d->mProberState = NotMe;
            break;
        case kencodingprober::eFoundIt:
            d->mProberState = FoundIt;
            break;
        default:
            d->mProberState = Probing;
            break;
        }
    }
#ifdef DEBUG_PROBE
    d->mProber->DumpStatus();
#endif
    return d->mProberState;
}

KEncodingProber::ProberState KEncodingProber::state() const
{
    return d->mProberState;
}

QByteArray KEncodingProber::encoding() const
{
    if (!d->mProber) {
        return QByteArray("UTF-8");
    }

    return QByteArray(d->mProber->GetCharSetName());
}

float KEncodingProber::confidence() const
{
    if (!d->mProber) {
        return 0.0;
    }

    return d->mProber->GetConfidence();
}

KEncodingProber::ProberType KEncodingProber::proberType() const
{
    return d->mProberType;
}

void KEncodingProber::setProberType(KEncodingProber::ProberType proberType)
{
    d->setProberType(proberType);
    reset();
}

KEncodingProber::ProberType KEncodingProber::proberTypeForName(const QString &lang)
{
    if (lang.isEmpty()) {
        return KEncodingProber::Universal;
    } else if (lang == tr("Disabled", "@item Text character set")) {
        return KEncodingProber::None;
    } else if (lang == tr("Universal", "@item Text character set")) {
        return KEncodingProber::Universal;
    } else if (lang == tr("Unicode", "@item Text character set")) {
        return KEncodingProber::Unicode;
    } else if (lang == tr("Cyrillic", "@item Text character set")) {
        return KEncodingProber::Cyrillic;
    } else if (lang == tr("Western European", "@item Text character set")) {
        return KEncodingProber::WesternEuropean;
    } else if (lang == tr("Central European", "@item Text character set")) {
        return KEncodingProber::CentralEuropean;
    } else if (lang == tr("Greek", "@item Text character set")) {
        return KEncodingProber::Greek;
    } else if (lang == tr("Hebrew", "@item Text character set")) {
        return KEncodingProber::Hebrew;
    } else if (lang == tr("Turkish", "@item Text character set")) {
        return KEncodingProber::Turkish;
    } else if (lang == tr("Japanese", "@item Text character set")) {
        return KEncodingProber::Japanese;
    } else if (lang == tr("Baltic", "@item Text character set")) {
        return KEncodingProber::Baltic;
    } else if (lang == tr("Chinese Traditional", "@item Text character set")) {
        return KEncodingProber::ChineseTraditional;
    } else if (lang == tr("Chinese Simplified", "@item Text character set")) {
        return KEncodingProber::ChineseSimplified;
    } else if (lang == tr("Korean", "@item Text character set")) {
        return KEncodingProber::Korean;
    } else if (lang == tr("Thai", "@item Text character set")) {
        return KEncodingProber::Thai;
    } else if (lang == tr("Arabic", "@item Text character set")) {
        return KEncodingProber::Arabic;
    }

    return KEncodingProber::Universal;
}

QString KEncodingProber::nameForProberType(KEncodingProber::ProberType proberType)
{
    switch (proberType) {
    case KEncodingProber::None:
        return tr("Disabled", "@item Text character set");
        break;
    case KEncodingProber::Universal:
        return tr("Universal", "@item Text character set");
        break;
    case KEncodingProber::Arabic:
        return tr("Arabic", "@item Text character set");
        break;
    case KEncodingProber::Baltic:
        return tr("Baltic", "@item Text character set");
        break;
    case KEncodingProber::CentralEuropean:
        return tr("Central European", "@item Text character set");
        break;
    case KEncodingProber::Cyrillic:
        return tr("Cyrillic", "@item Text character set");
        break;
    case KEncodingProber::Greek:
        return tr("Greek", "@item Text character set");
        break;
    case KEncodingProber::Hebrew:
        return tr("Hebrew", "@item Text character set");
        break;
    case KEncodingProber::Japanese:
        return tr("Japanese", "@item Text character set");
        break;
    case KEncodingProber::Turkish:
        return tr("Turkish", "@item Text character set");
        break;
    case KEncodingProber::WesternEuropean:
        return tr("Western European", "@item Text character set");
        break;
    case KEncodingProber::ChineseTraditional:
        return tr("Chinese Traditional", "@item Text character set");
        break;
    case KEncodingProber::ChineseSimplified:
        return tr("Chinese Simplified", "@item Text character set");
        break;
    case KEncodingProber::Korean:
        return tr("Korean", "@item Text character set");
        break;
    case KEncodingProber::Thai:
        return tr("Thai", "@item Text character set");
        break;
    case KEncodingProber::Unicode:
        return tr("Unicode", "@item Text character set");
        break;
    default:
        return QString();
    }
}

// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiopreview_global.h"
#include "audiofileinfo.h"
#include "global/commontools.h"

#include <QDebug>
#include <QTextCodec>

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <tag.h>
#include <fileref.h>
#include <taglib.h>
#include <tpropertymap.h>

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::audio_preview;

AudioFileInfo::AudioFileInfo()
{
    m_localeCodeMap.insert("zh_CN", "GB18030");
}

AudioFileInfo::AudioMetaData AudioFileInfo::openAudioFile(const QString &file)
{
    TagLib::FileRef f(file.toLocal8Bit());
    if (!f.file()) {
        qWarning() << "TagLib: open file failed:" << file;
        return {};
    }

    TagLib::Tag *tag = f.tag();
    if (!tag) {
        qWarning() << "TagLib: no tag for media file" << file;
        return {};
    }

    AudioMetaData meta;
    characterEncodingTransform(meta, static_cast<void *>(tag));

    TagLib::AudioProperties *ap = f.audioProperties();
    if (ap) {
        auto len = ap->length() * 1000;
        meta.duration = CommonTools::durationString(len / 1000);
    }

    return meta;
}

void AudioFileInfo::characterEncodingTransform(AudioFileInfo::AudioMetaData &meta, void *obj)
{
    TagLib::Tag *tag = static_cast<TagLib::Tag *>(obj);
    bool encode = true;
    encode &= tag->title().isNull() ? true : tag->title().isLatin1();
    encode &= tag->artist().isNull() ? true : tag->artist().isLatin1();
    encode &= tag->album().isNull() ? true : tag->album().isLatin1();

    QByteArray detectByte;
    QByteArray detectCodec;
    if (encode) {
        if (detectCodec.isEmpty()) {
            detectByte += tag->title().toCString();
            detectByte += tag->artist().toCString();
            detectByte += tag->album().toCString();
            auto allDetectCodecs = detectEncodings(detectByte);
            auto localeCode = m_localeCodeMap.value(QLocale::system().name());

            auto iter = std::find_if(allDetectCodecs.begin(), allDetectCodecs.end(),
                                     [localeCode](const QByteArray & curDetext) {
                return (curDetext == "Big5" || curDetext == localeCode);
            });

            if (iter != allDetectCodecs.end())
                detectCodec = *iter;

            if (detectCodec.isEmpty())
                detectCodec = allDetectCodecs.value(0);

            QString curStr = QString::fromLocal8Bit(tag->title().toCString());
            if (curStr.isEmpty())
                curStr = QString::fromLocal8Bit(tag->artist().toCString());
            if (curStr.isEmpty())
                curStr = QString::fromLocal8Bit(tag->album().toCString());

            auto ret = std::any_of(curStr.begin(), curStr.end(), [this](const QChar & ch) {
                return isChinese(ch);
            });

            if (ret)
                detectCodec = "GB18030";
        }

        QString detectCodecStr(detectCodec);
        if (detectCodecStr.compare("utf-8", Qt::CaseInsensitive) == 0) {
            meta.album = TStringToQString(tag->album());
            meta.artist = TStringToQString(tag->artist());
            meta.title = TStringToQString(tag->title());
            meta.codec = "UTF-8";  //info codec
        } else {
            QTextCodec *codec = QTextCodec::codecForName(detectCodec);
            if (codec == nullptr) {
                meta.album = TStringToQString(tag->album());
                meta.artist = TStringToQString(tag->artist());
                meta.title = TStringToQString(tag->title());
            } else {
                meta.album = codec->toUnicode(tag->album().toCString());
                meta.artist = codec->toUnicode(tag->artist().toCString());
                meta.title = codec->toUnicode(tag->title().toCString());
            }
            meta.codec = detectCodec;
        }
    } else {
        meta.album = TStringToQString(tag->album());
        meta.artist = TStringToQString(tag->artist());
        meta.title = TStringToQString(tag->title());
        meta.codec = "UTF-8";
    }

    //empty str
    meta.album = meta.album.simplified();
    meta.artist = meta.artist.simplified();
    meta.title = meta.title.simplified();
}

QList<QByteArray> AudioFileInfo::detectEncodings(const QByteArray &rawData)
{
    QList<QByteArray> charsets;
    QByteArray charset = QTextCodec::codecForLocale()->name();
    charsets << charset;

    const char *data = rawData.data();
    int32_t len = rawData.size();

    UCharsetDetector *csd;
    const UCharsetMatch **csm;
    int32_t matchCount = 0;

    UErrorCode status = U_ZERO_ERROR;

    csd = ucsdet_open(&status);
    if (status != U_ZERO_ERROR) {
        ucsdet_close(csd);
        return charsets;
    }

    ucsdet_setText(csd, data, len, &status);
    if (status != U_ZERO_ERROR) {
        ucsdet_close(csd);
        return charsets;
    }

    csm = ucsdet_detectAll(csd, &matchCount, &status);
    if (status != U_ZERO_ERROR) {
        ucsdet_close(csd);
        return charsets;
    }

    if (matchCount > 0)
        charsets.clear();

    for (int32_t match = 0; match < matchCount; match += 1) {
        const char *name = ucsdet_getName(csm[match], &status);
        const char *lang = ucsdet_getLanguage(csm[match], &status);
        if (lang == nullptr || strlen(lang) == 0)
            lang = "**";
        charsets << name;
    }

    ucsdet_close(csd);
    return charsets;
}

bool AudioFileInfo::isChinese(const QChar &c)
{
    return c.unicode() <= 0x9FBF && c.unicode() >= 0x4E00;
}

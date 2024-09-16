// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUDIOINFO_H
#define AUDIOINFO_H

#include <QMap>

namespace GrandSearch {
namespace audio_preview {

class AudioFileInfo
{
public:
    struct AudioMetaData {
        QString title;
        QString artist;
        QString album;
        QString codec;
        QString duration;
    };

    AudioFileInfo();
    AudioMetaData openAudioFile(const QString &file);

private:
    void characterEncodingTransform(AudioMetaData &meta, void *obj);
    QList<QByteArray> detectEncodings(const QByteArray &rawData);
    bool isChinese(const QChar &c);

private:
    QMap<QString, QByteArray> m_localeCodeMap;  // 区域与编码
};

}}

#endif // AUDIOINFO_H

// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"

#include <QTextCodec>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QLocale>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <linux/limits.h>

#include <KEncodingProber>
#include "chardet.h"

//QByteArray m_codecStr;
/*static */float Common::codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country)
{
    qreal hep_count = 0;
    int non_base_latin_count = 0;
    qreal unidentification_count = 0;
    int replacement_count = 0;

    QTextDecoder decoder(codec);
    const QString &unicode_data = decoder.toUnicode(data);

    for (int i = 0; i < unicode_data.size(); ++i) {
        const QChar &ch = unicode_data.at(i);

        if (ch.unicode() > 0x7f)
            ++non_base_latin_count;

        switch (ch.script()) {
        case QChar::Script_Hiragana:
        case QChar::Script_Katakana:
            hep_count += (country == QLocale::Japan) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Japan) ? 0 : 0.3;
            break;
        case QChar::Script_Han:
            hep_count += (country == QLocale::China) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::China) ? 0 : 0.3;
            break;
        case QChar::Script_Hangul:
            hep_count += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 0 : 0.3;
            break;
        case QChar::Script_Cyrillic:
            hep_count += (country == QLocale::Russia) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Russia) ? 0 : 0.3;
            break;
        case QChar::Script_Devanagari:
            hep_count += (country == QLocale::Nepal || country == QLocale::India) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Nepal || country == QLocale::India) ? 0 : 0.3;
            break;
        default:
            // full-width character, emoji, 常用标点, 拉丁文补充1，天城文补充，CJK符号和标点符号（如：【】）
            if ((ch.unicode() >= 0xff00 && ch <= 0xffef)
                    || (ch.unicode() >= 0x2600 && ch.unicode() <= 0x27ff)
                    || (ch.unicode() >= 0x2000 && ch.unicode() <= 0x206f)
                    || (ch.unicode() >= 0x80 && ch.unicode() <= 0xff)
                    || (ch.unicode() >= 0xa8e0 && ch.unicode() <= 0xa8ff)
                    || (ch.unicode() >= 0x0900 && ch.unicode() <= 0x097f)
                    || (ch.unicode() >= 0x3000 && ch.unicode() <= 0x303f)) {
                ++hep_count;
            } else if (ch.isSurrogate() && ch.isHighSurrogate()) {
                ++i;

                if (i < unicode_data.size()) {
                    const QChar &next_ch = unicode_data.at(i);

                    if (!next_ch.isLowSurrogate()) {
                        --i;
                        break;
                    }

                    uint unicode = QChar::surrogateToUcs4(ch, next_ch);

                    // emoji
                    if (unicode >= 0x1f000 && unicode <= 0x1f6ff) {
                        hep_count += 2;
                    }
                }
            } else if (ch.unicode() == QChar::ReplacementCharacter) {
                ++replacement_count;
            } else if (ch.unicode() > 0x7f) {
                // 因为UTF-8编码的容错性很低，所以未识别的编码只需要判断是否为 QChar::ReplacementCharacter 就能排除
                if (codec->name() != "UTF-8")
                    ++unidentification_count;
            }
            break;
        }
    }

    float c = qreal(hep_count) / non_base_latin_count / 1.2;

    c -= qreal(replacement_count) / non_base_latin_count;
    c -= qreal(unidentification_count) / non_base_latin_count;

    return qMax(0.0f, c);
}

QString Common::trans2uft8(const char *str, QByteArray &strCode)
{
    if (strCode.isEmpty())
        strCode = detectEncode(str);

    if (strCode.isEmpty()) {
        return str;
    } else if ("gb18030" == strCode) {
        QTextCodec *codec = QTextCodec::codecForName(strCode);
        m_codecStr = strCode;
        return codec->toUnicode(str);
    } else if ((QString(strCode)).contains("windows", Qt::CaseInsensitive) || (QString(strCode)).contains("IBM", Qt::CaseInsensitive)
               || (QString(strCode)).contains("x-mac", Qt::CaseInsensitive) || (QString(strCode)).contains("Big5", Qt::CaseInsensitive)
               || (QString(strCode)).contains("iso", Qt::CaseInsensitive)) {
        QTextCodec *codec = QTextCodec::codecForName("GBK");
        m_codecStr = strCode;
        return codec->toUnicode(str);
    } else if (!(QString(strCode)).contains("UTF", Qt::CaseInsensitive)) {
        QTextCodec *codec = QTextCodec::codecForName(strCode);
        m_codecStr = strCode;
        return codec->toUnicode(str);
    } else {
        m_codecStr = "UTF-8";
        return QString(str);
    }
}

QByteArray Common::detectEncode(const QByteArray &data, const QString &fileName)
{
    QString detectedResult;
    float chardetconfidence = 0;
    QString str(data);
    bool bFlag = str.contains(QRegExp("[\\x4e00-\\x9fa5]+")); //匹配的是中文
    if (bFlag) {
        QByteArray newData = data;
        newData += "为增加探测率保留的中文";    //手动添加中文字符，避免字符长度太短而导致判断编码错误
        ChartDet_DetectingTextCoding(newData, detectedResult, chardetconfidence);
    } else {
        ChartDet_DetectingTextCoding(data, detectedResult, chardetconfidence);
    }
    //qInfo() << "chardet编码：" << detectedResult;
    m_codecStr = detectedResult.toLatin1();
    if (detectedResult.contains("UTF-8", Qt::CaseInsensitive) || detectedResult.contains("ASCII", Qt::CaseInsensitive)) {
        m_codecStr = "UTF-8";
        return  m_codecStr;
    } else {
        if (((QString)m_codecStr).contains("windows", Qt::CaseInsensitive) || ((QString)m_codecStr).contains("IBM", Qt::CaseInsensitive)
                || ((QString)m_codecStr).contains("x-mac", Qt::CaseInsensitive) || ((QString)m_codecStr).contains("Big5", Qt::CaseInsensitive)
                || ((QString)m_codecStr).contains("gb18030", Qt::CaseInsensitive)  || ((QString)m_codecStr).contains("iso", Qt::CaseInsensitive)) {
            return  m_codecStr;
        } else {
            m_codecStr = textCodecDetect(data, fileName);
        }
    }

    return  m_codecStr;
}

int Common::ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence)
{
    DetectObj *obj = detect_obj_init();

    if (obj == nullptr) {
        //qInfo() << "Memory Allocation failed\n";
        return CHARDET_MEM_ALLOCATED_FAIL;
    }

#ifndef CHARDET_BINARY_SAFE
    // before 1.0.5. This API is deprecated on 1.0.5
    switch (detect(str, &obj))
#else
    // from 1.0.5
    switch (detect_r(str, strlen(str), &obj))
#endif
    {
    case CHARDET_OUT_OF_MEMORY :
        qInfo() << "On handle processing, occured out of memory\n";
        detect_obj_free(&obj);
        return CHARDET_OUT_OF_MEMORY;
    case CHARDET_NULL_OBJECT :
        qInfo() << "2st argument of chardet() is must memory allocation with detect_obj_init API\n";
        return CHARDET_NULL_OBJECT;
    }

#ifndef CHARDET_BOM_CHECK
    //qInfo() << "encoding:" << obj->encoding << "; confidence: " << obj->confidence;
#else
    // from 1.0.6 support return whether exists BOM
    qInfo() << "encoding:" << obj->encoding << "; confidence: " << obj->confidence << "; bom: " << obj->bom;
#endif

    encoding = obj->encoding;
    confidence = obj->confidence;
    detect_obj_free(&obj);

    return CHARDET_SUCCESS ;
}

QByteArray Common::textCodecDetect(const QByteArray &data, const QString &fileName)
{
    // Return local encoding if nothing in file.
    if (data.isEmpty()) {
        return QTextCodec::codecForLocale()->name();
    }

    if (QTextCodec *c = QTextCodec::codecForUtfText(data, nullptr)) {
        return c->name();
    }

    QMimeDatabase mime_database;
    const QMimeType &mime_type = fileName.isEmpty() ? mime_database.mimeTypeForData(data) : mime_database.mimeTypeForFileNameAndData(fileName, data);
    const QString &mimetype_name = mime_type.name();
    KEncodingProber::ProberType proberType = KEncodingProber::Universal;

    if (mimetype_name == QStringLiteral("application/xml")
            || mimetype_name == QStringLiteral("text/html")
            || mimetype_name == QStringLiteral("application/xhtml+xml")) {
        const QString &_data = QString::fromLatin1(data);
        QRegularExpression pattern("<\\bmeta.+\\bcharset=(?'charset'\\S+?)\\s*['\"/>]");

        pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
        const QString &charset = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                               QRegularExpression::DontCheckSubjectStringMatchOption).captured("charset");

        if (!charset.isEmpty()) {
            return charset.toLatin1();
        }

        pattern.setPattern("<\\bmeta\\s+http-equiv=\"Content-Language\"\\s+content=\"(?'language'[a-zA-Z-]+)\"");

        const QString &language = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                                QRegularExpression::DontCheckSubjectStringMatchOption).captured("language");

        if (!language.isEmpty()) {
            QLocale l(language);

            switch (l.script()) {
            case QLocale::ArabicScript:
                proberType = KEncodingProber::Arabic;
                break;
            case QLocale::SimplifiedChineseScript:
                proberType = KEncodingProber::ChineseSimplified;
                break;
            case QLocale::TraditionalChineseScript:
                proberType = KEncodingProber::ChineseTraditional;
                break;
            case QLocale::CyrillicScript:
                proberType = KEncodingProber::Cyrillic;
                break;
            case QLocale::GreekScript:
                proberType = KEncodingProber::Greek;
                break;
            case QLocale::HebrewScript:
                proberType = KEncodingProber::Hebrew;
                break;
            case QLocale::JapaneseScript:
                proberType = KEncodingProber::Japanese;
                break;
            case QLocale::KoreanScript:
                proberType = KEncodingProber::Korean;
                break;
            case QLocale::ThaiScript:
                proberType = KEncodingProber::Thai;
                break;
            default:
                break;
            }
        }
    } else if (mimetype_name == "text/x-python") {
        QRegularExpression pattern("^#coding\\s*:\\s*(?'coding'\\S+)$");
        QTextStream stream(data);

        pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
        stream.setCodec("latin1");

        while (!stream.atEnd()) {
            const QString &_data = stream.readLine();
            const QString &coding = pattern.match(_data, 0).captured("coding");

            if (!coding.isEmpty()) {
                return coding.toLatin1();
            }
        }
    }

    // for CJK
    const QList<QPair<KEncodingProber::ProberType, QLocale::Country>> fallback_list {
        {KEncodingProber::ChineseSimplified, QLocale::China},
        {KEncodingProber::ChineseTraditional, QLocale::China},
        {KEncodingProber::Japanese, QLocale::Japan},
        {KEncodingProber::Korean, QLocale::NorthKorea},
        {KEncodingProber::Cyrillic, QLocale::Russia},
        {KEncodingProber::Greek, QLocale::Greece},
        {proberType, QLocale::system().country()}
    };

    KEncodingProber prober(proberType);
    prober.feed(data);
    float pre_confidence = prober.confidence();
    QByteArray pre_encoding = prober.encoding();

    QTextCodec *def_codec = QTextCodec::codecForLocale();
    QByteArray encoding;
    float confidence = 0;

    for (auto i : fallback_list) {
        prober.setProberType(i.first);
        prober.feed(data);

        float prober_confidence = prober.confidence();
        QByteArray prober_encoding = prober.encoding();

        if (i.first != proberType && qFuzzyIsNull(prober_confidence)) {
            prober_confidence = pre_confidence;
            prober_encoding = pre_encoding;
        }

    confidence:
        if (QTextCodec *codec = QTextCodec::codecForName(prober_encoding)) {
            if (def_codec == codec)
                def_codec = nullptr;

            float c = codecConfidenceForData(codec, data, i.second);

            if (prober_confidence > 0.5) {
                c = c / 2 + prober_confidence / 2;
            } else {
                c = c / 3 * 2 + prober_confidence / 3;
            }

            if (c > confidence) {
                confidence = c;
                encoding = prober_encoding;
            }

            if (i.first == KEncodingProber::ChineseTraditional && c < 0.5) {
                // test Big5
                c = codecConfidenceForData(QTextCodec::codecForName("Big5"), data, i.second);

                if (c > 0.5 && c > confidence) {
                    confidence = c;
                    encoding = "Big5";
                }
            }
        }

        if (i.first != proberType) {
            // 使用 proberType 类型探测出的结果结合此国家再次做编码检查
            i.first = proberType;
            prober_confidence = pre_confidence;
            prober_encoding = pre_encoding;
            goto confidence;
        }
    }

    if (def_codec && codecConfidenceForData(def_codec, data, QLocale::system().country()) > confidence) {
        return def_codec->name();
    }

    qInfo() << "QCodecs编码：" << encoding;
    return encoding;
}

QString Common::handleLongNameforPath(const QString &strFilePath, const QString &entryName, QMap<QString, int> &mapLongDirName, QMap<QString, int> &mapRealDirValue)
{
    bool bLongName = false;
    QString tempFilePathName;
    QFileInfo info(strFilePath);
     if(entryName.endsWith(QDir::separator())){
         //目录计数+1
        if(NAME_MAX < info.fileName().toLocal8Bit().length()) {
            QString strTemp = info.fileName().left(TRUNCATION_FILE_LONG);
            if(info.path() == ".") {
                tempFilePathName = strTemp;
            } else {
                tempFilePathName = info.path() + QDir::separator() + strTemp;
            }
            mapLongDirName[tempFilePathName]++;
            mapRealDirValue[info.filePath()] = mapLongDirName[tempFilePathName];
            bLongName = true;
        }
    }
    //目录拼接
     QString sDir;
     if(NAME_MAX < strFilePath.toLocal8Bit().length()) {
         QString strTempPath = strFilePath + QDir::separator();
         int nIndex = strTempPath.indexOf(QDir::separator(), 0);
         while (nIndex >= 0) {
            QString strPathName = strTempPath.left(nIndex);
            QFileInfo pathinfo(strPathName);
            if(NAME_MAX < pathinfo.fileName().toLocal8Bit().length()) {
                QString strTemp = pathinfo.fileName().left(TRUNCATION_FILE_LONG);
                if(pathinfo.path() == ".") {
                    tempFilePathName = strTemp;
                } else {
                    tempFilePathName = pathinfo.path() + QDir::separator() + strTemp;
                }
                if(mapLongDirName.contains(tempFilePathName)) {
                    sDir = sDir + strTemp + QString("(%1)").arg(mapRealDirValue[pathinfo.filePath()], LONGFILE_SUFFIX_FieldWidth, BINARY_NUM, QChar('0')) + QDir::separator();
                }else {
                    mapLongDirName[tempFilePathName]++;
                    mapRealDirValue[pathinfo.filePath()] = mapLongDirName[tempFilePathName];
                    bLongName = true;
                    sDir = sDir + strTemp + QString("(%1)").arg(mapRealDirValue[pathinfo.filePath()], 3, BINARY_NUM, QChar('0')) + QDir::separator();
                }
            } else {
                sDir = sDir + pathinfo.fileName() + QDir::separator();
            }
            nIndex = strTempPath.indexOf(QDir::separator(), nIndex+1);
         }
     }
     return sDir;
}

bool Common::isSubpathOfDlnfs(const QString &path)
{
    return findDlnfsPath(path, [](const QString &target, const QString &compare) {
        return target.startsWith(compare);
    });
}

bool Common::findDlnfsPath(const QString &target, Compare func)
{
    Q_ASSERT(func);
    libmnt_table *tab { mnt_new_table() };
    libmnt_iter *iter = mnt_new_iter(MNT_ITER_BACKWARD);

    auto unifyPath = [](const QString &path) {
        return path.endsWith("/") ? path : path + "/";
    };

    int ret = mnt_table_parse_mtab(tab, nullptr);
    if (ret != 0) {
        qWarning() << "device: cannot parse mtab" << ret;
        if (tab) mnt_free_table(tab);
        if (iter) mnt_free_iter(iter);
        return false;
    }

    libmnt_fs *fs = nullptr;
    while (mnt_table_next_fs(tab, iter, &fs) == 0) {
        if (!fs)
            continue;
        qInfo() << unifyPath(mnt_fs_get_target(fs));
        if (strcmp("dlnfs", mnt_fs_get_source(fs)) == 0) {
            QString mpt = unifyPath(mnt_fs_get_target(fs));
            if (func(unifyPath(target), mpt))
                if (tab) mnt_free_table(tab);
                if (iter) mnt_free_iter(iter);
                return true;
        }
    }

    if (tab) mnt_free_table(tab);
    if (iter) mnt_free_iter(iter);
    return false;
}

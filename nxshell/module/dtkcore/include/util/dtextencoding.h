// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTEXTENCODING_H
#define DTEXTENCODING_H

#include <dtkcore_global.h>

#include <QString>
#include <QByteArray>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DTextEncoding
{
public:
    static QByteArray detectTextEncoding(const QByteArray &content);
    static QByteArray detectFileEncoding(const QString &fileName, bool *isOk = nullptr);

    static bool convertTextEncoding(QByteArray &content,
                                    QByteArray &outContent,
                                    const QByteArray &toEncoding,
                                    const QByteArray &fromEncoding = QByteArray(),
                                    QString *errString = nullptr);
    static bool convertTextEncodingEx(QByteArray &content,
                                      QByteArray &outContent,
                                      const QByteArray &toEncoding,
                                      const QByteArray &fromEncoding = QByteArray(),
                                      QString *errString = nullptr,
                                      int *convertedBytes = nullptr);
    static bool convertFileEncoding(const QString &fileName,
                                    const QByteArray &toEncoding,
                                    const QByteArray &fromEncoding = QByteArray(),
                                    QString *errString = nullptr);
    static bool convertFileEncodingTo(const QString &fromFile,
                                      const QString &toFile,
                                      const QByteArray &toEncoding,
                                      const QByteArray &fromEncoding = QByteArray(),
                                      QString *errString = nullptr);
};

DCORE_END_NAMESPACE

#endif  // DTEXTENCODING_H

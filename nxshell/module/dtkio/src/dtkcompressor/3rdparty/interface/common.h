// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QLocale>
#include <QTextCodec>

class Common: public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr)
        : QObject(parent) {}

public:
    static float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country);
    QString  trans2uft8(const char *str, QByteArray &strCode);
    QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
    int ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence);
    QByteArray textCodecDetect(const QByteArray &data, const QString &fileName);

    QByteArray m_codecStr;
};

#endif

// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QObject>

class Utils : public QObject
{
    Q_OBJECT

public:
    Utils();
    ~Utils();

    static QString getConfigDir();
    static QString getQssContent(const QString &filePath);
    static QString formatThousandsSeparators(const QString &str);
    static bool stringIsDigit(const QString &str);
    static QString reformatSeparators(const QString &exp);

    //程序员计算器format
    static QString formatThousandsSeparatorsPro(const QString &str, const int Base);
    static bool stringIsDigitPro(const QString &str, const int Base);
    static QString reformatSeparatorsPro(const QString &exp, const int Base);

    //全角转半角符号
    static QString toHalfWidth(const QString &str);

private:
    static bool isAtoE(const QChar &num);
};

#endif

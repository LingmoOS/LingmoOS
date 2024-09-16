// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LANGUAGETRANSFORMATION_H
#define LANGUAGETRANSFORMATION_H

#include <QObject>

class LanguageTransformation : public QObject
{
    Q_OBJECT
public:
    explicit LanguageTransformation(QObject *parent = Q_NULLPTR);
    static LanguageTransformation *instance();

    QString zhToPinYin(const QString &chinese);
    QString zhToJianPin(const QString &chinese);
    void readConfigFile();

private:
    static LanguageTransformation *m_instance;

    QStringList m_pinyinStrList;
    QStringList m_jianpingStrList;
};

#endif

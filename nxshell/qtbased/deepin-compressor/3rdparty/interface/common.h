// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QLocale>
#include <libmount/libmount.h>

//文件大小10M
#define FILE_MAX_SIZE (10 * 1024 * 1024)
//截断文件长度
#define TRUNCATION_FILE_LONG (60)
//长文件后缀编码长度 001 002 ...
#define LONGFILE_SUFFIX_FieldWidth (3)
//相同目录下最大支持截断后同名文件数
#define LONGFILE_SAME_FILES (999)
//进制数
#define BINARY_NUM 10

typedef  bool (Compare )(const QString &target, const QString &compare);
class Common: public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr)
        : QObject(parent) {}

public:
    static float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country);
    QString trans2uft8(const char *str, QByteArray &strCode);
    QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
    int ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence);
    QByteArray textCodecDetect(const QByteArray &data, const QString &fileName);

    QByteArray m_codecStr;

    //长文件夹处理
    QString handleLongNameforPath(const QString &strFilePath, const QString &entryName, QMap<QString, int> &mapLongDirName, QMap<QString, int> &mapRealDirValue);
    //当前文件系统是否支持长文件
    bool isSubpathOfDlnfs(const QString &path);
private:
    //通过mount对应方法判断文件系统是否支持长文件
    bool findDlnfsPath(const QString &target, Compare func);
};

#endif

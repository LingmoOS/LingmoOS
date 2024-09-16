// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTSMANAGER_H
#define FONTSMANAGER_H
#include <QString>
#include <QStringList>
#include <QMap>
#include <QGSettings>
#include <fontconfig/fontconfig.h>
#include <QSharedPointer>

class FontsManager
{
public:
    struct Family{
        QString     id;
        QString     name;
        QStringList styles;
        bool        monospace;
        bool        show;
    };
    struct FcInfo{
        char *family;
        char *familylang;
        /* char *fullname; */
        /* char *fullnamelang; */
        char *style;
        char *lang;
        char *spacing;
    };
public:
    FontsManager();
    void initFamily();
    bool isFontFamily(QString value);
    bool isFontSizeValid(double size);
    bool setFamily(QString standard, QString monospace, double size);
    bool reset();
    QStringList listMonospace();
    QStringList listStandard();
    QSharedPointer<Family> getFamily(QString id);
    QVector<QSharedPointer<Family>> getFamilies(QStringList ids);
    double getFontSize();
    bool checkFontConfVersion();

private:
    QString fcFontMatch(QString family);
    static QString md5(QString src);
    static QString getStringFromUnsignedChar(unsigned char *str, unsigned int length);
    static QString fontMatch(QString family);
    static QString configContent(QString standard, QString monospace);
    bool   isFcCacheUpdate();
    FcInfo* listFontInfo (int *num);
    int appendFontinfo(FcInfo** list, FcPattern* pat, int idx);
    void freeFontInfoList(FcInfo *list, int num);
    QSharedPointer<Family> fcInfoToFamily(FcInfo* info);
    QString getCurLang();
    QString getLangFromLocale(QString locale);
    void fcInfosToFamilyTable();
    bool loadCacheFromFile(QString fileName);
    bool saveToFile();
private:
    QStringList                             virtualFonts;
    QString                                 filePath;
    QMap<QString,QSharedPointer<Family>>    familyMap;
    QSharedPointer<QGSettings>              xSetting;
    QStringList                             familyBlacklist;
    QString                                 fileName;
};

#endif // FONTSMANAGER_H

// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globaltool.h"

#include <DFontSizeManager>
#include <QFile>
#include <QFontDatabase>
#include <QDebug>

DWIDGET_USE_NAMESPACE

Globaltool::Globaltool()
{
}

Globaltool::~Globaltool()
{
}

/**
 * @brief Globaltool::instacne 获取单例
 * @return
 */
Globaltool *Globaltool::instacne()
{
    static Globaltool globaltool;
    return &globaltool;
}

QPixmap Globaltool::getDpiPixmap(QSize size, const QString &filename, QWidget *w)
{
    QPixmap pixmap;
    qreal devicePixelRatio = qApp->devicePixelRatio();
    if (w) {
        devicePixelRatio = w->devicePixelRatioF();
    }

    qreal ratio = 1.0;
    QImageReader reader;
    reader.setFileName(qt_findAtNxFile(filename, devicePixelRatio, &ratio));

    if (!qFuzzyCompare(ratio, devicePixelRatio)) {
        if (reader.canRead()) {
            QSize picSize;
            if (size != QSize(0, 0)) {
                picSize = size;
            } else {
                picSize = reader.size();
            }

            reader.setScaledSize(picSize * (devicePixelRatio / ratio));
            pixmap = QPixmap::fromImage(reader.read());
            pixmap.setDevicePixelRatio(devicePixelRatio);
        }
    } else {
        if (size != QSize(0, 0)) {
            reader.setScaledSize(size * (devicePixelRatio / ratio));
            pixmap = QPixmap::fromImage(reader.read());
        } else {
            pixmap.load(filename);
        }
    }
    return pixmap;
}

QString Globaltool::AutoFeed(const QString &text, const int fontSize, const int textWidth)
{
    QString strText = text;
    QString resultStr = nullptr;
    QFont labelF;
    labelF.setFamily(Globaltool::loadFontFamilyFromFiles(":/resources/font/ResourceHanRoundedCN-Bold.ttf"));
    labelF.setWeight(QFont::Black);
    labelF.setPixelSize(fontSize);
    QFontMetrics fm(labelF);
    int titlewidth = fm.width(strText);
    QStringList strList;
    QString str;
    strList.clear();

    if (titlewidth < textWidth) {
        strList.append(strText);
        resultStr += strText;
    } else {
        for (int i = 0; i < strText.count(); i++) {
            str += strText.at(i);

            if (fm.width(str) > textWidth) {
                str.remove(str.count() - 1, 1);
                strList.append(str);
                resultStr += str + "\n";
                str.clear();
                --i;
            }
        }
        strList.append(str);
        resultStr += str;
    }

    return resultStr;
}

QString Globaltool::loadFontFamilyFromFiles(const QString &fontFileName)
{
    static QHash<QString, QString> tmd;
    if (tmd.contains(fontFileName)) {
        return tmd.value(fontFileName);
    }
    QString font = "";
    QFile fontFile(fontFileName);
    if (!fontFile.open(QIODevice::ReadOnly)) {
        qDebug() << __FUNCTION__ << "Open font file error";
        return font;
    }

    int loadedFontID = QFontDatabase::addApplicationFontFromData(fontFile.readAll());
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
    if (!loadedFontFamilies.empty()) {
        font = loadedFontFamilies.at(0);
    }
    fontFile.close();

    if (!(font.isEmpty()))
        tmd.insert(fontFileName, font);
    return font;
}

void Globaltool::loadSystemLanguage()
{
    QLocale locale;
    if (locale.name() == "bo_CN") { //如果是藏语，
        gomokuFontManagement.dialogOffset = 3; //藏语时弹窗的偏移量为3
        gomokuFontManagement.welcomeText = 20; //欢迎字号变为20号
    } else if (locale.name() == "zh_CN" || locale.name() == "zh_HK"
               || locale.name() == "zh_TW") { //如果是中文功能按钮为23号字体
        gomokuFontManagement.functionButton = 23;
    }
    qInfo() << "当前系统的语言:" << locale.name();
}


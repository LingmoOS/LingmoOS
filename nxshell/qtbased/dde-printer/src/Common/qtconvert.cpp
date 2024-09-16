// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qtconvert.h"

#include <QDebug>
#include <QString>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CONVERT, "org.deepin.dde-printer.convert")

vector<string> qStringListStdVector(const QStringList &strList)
{
    vector<string> stdVector;

    foreach (QString str, strList) {
        stdVector.push_back(str.toStdString());
    }

    return stdVector;
}

QString attrValueToQString(const string &value)
{
    QString strValue;

    strValue = STQ(value);
    strValue.remove('`');
    strValue = strValue.right(strValue.length() - 1);

    return strValue;
}

map<string, string> mapValueByIndex(const map<int, map<string, string>> &mapData, int index, int order)
{
    if (ORDER_Forward == order) {
        map<int, map<string, string>>::const_iterator itmaps;
        for (itmaps = mapData.begin(); itmaps != mapData.end(); itmaps++) {
            if (0 == index--)
                return itmaps->second;
        }
    } else {
        map<int, map<string, string>>::const_reverse_iterator itmaps;
        for (itmaps = mapData.rbegin(); itmaps != mapData.rend(); itmaps++) {
            if (0 == index--)
                return itmaps->second;
        }
    }

    return map<string, string>();
}

int intMapKeyByIndex(const map<int, map<string, string>> &mapData, int index, int order)
{
    if (ORDER_Forward == order) {
        map<int, map<string, string>>::const_iterator itmaps;
        for (itmaps = mapData.begin(); itmaps != mapData.end(); itmaps++) {
            if (0 == index--)
                return itmaps->first;
        }
    } else {
        map<int, map<string, string>>::const_reverse_iterator itmaps;
        for (itmaps = mapData.rbegin(); itmaps != mapData.rend(); itmaps++) {
            if (0 == index--)
                return itmaps->first;
        }
    }

    return -1;
}

void dumpStdMapValue(const map<string, string> &mapValue)
{
    map<string, string>::const_iterator itinfo = mapValue.begin();

    for (; itinfo != mapValue.end(); ++itinfo) {
        qCDebug(CONVERT) << QString::fromStdString(itinfo->first) << ":" << attrValueToQString(itinfo->second);
    }
}

void geteElidedText(const QFont &font, QString &str, int maxWidth)
{
    QFontMetrics fontWidth(font);
    int width = fontWidth.width(str); //计算字符串宽度
    if (width >= maxWidth) { //当字符串宽度大于最大宽度时进行转换
        str = fontWidth.elidedText(str, Qt::ElideRight, maxWidth); //右部显示省略号
    }
}

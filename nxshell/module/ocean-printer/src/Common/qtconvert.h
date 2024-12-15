// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QTCONVERT_H
#define QTCONVERT_H

#include <QStringList>
#include <QFont>
#include <QFontMetrics>

#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>
using namespace std;

enum {
    ORDER_Forward,
    ORDER_Reverse
};

#define STQ(a) QString::fromUtf8((a).c_str())

vector<string> qStringListStdVector(const QStringList &strList);

QString attrValueToQString(const string &value);

map<string, string> mapValueByIndex(const map<int, map<string, string>> &mapData, int index, int order = ORDER_Forward);

int intMapKeyByIndex(const map<int, map<string, string>> &mapData, int index, int order = ORDER_Forward);

void dumpStdMapValue(const map<string, string> &mapValue);

void geteElidedText(const QFont &font, QString &str, int maxWidth);

#endif // QTCONVERT_H

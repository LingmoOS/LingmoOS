// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PINYINDICT_H
#define PINYINDICT_H

#include <QMap>
#include <QVector>

//获取拼音字典
const QVector<QMap<int,QString> > getPinYinDictVector();
// 带音标字符。
extern QMap<QString, QString> phoneticSymbol;
/* 合法拼音列表 */
extern QVector<QString> validPinyinList;

#endif // PINYINDICT_H

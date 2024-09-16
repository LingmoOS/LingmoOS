// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "dsettings.h"

#include <QRegularExpression>
#include <QStandardPaths>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QDebug>

const static QString ATOE = "ABCDEF";

Utils::Utils()
{

}

Utils::~Utils()
{
}

/**
 * @brief 返回程序文件路径
 */
QString Utils::getConfigDir()
{
    QDir dir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first())
             .filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

/**
 * @brief 获取文件内容（暂未使用）
 */
QString Utils::getQssContent(const QString &filePath)
{
    QFile file(filePath);
    QString qss = nullptr;

    if (file.open(QIODevice::ReadOnly)) {
        qss = file.readAll();
    }

    return qss;
}

/**
 * @brief 补充千位符(针对结果)
 */
QString Utils::formatThousandsSeparators(const QString &str)
{
    int separate = DSettingsAlt::instance()->getSeparate(); //数字分割位数

    QString result = str;
    int startPos = result.indexOf(QRegularExpression("[0-9]"));
    if (startPos >= 0) {
        int endPos = result.indexOf('.');

        if (endPos < 0) {
            endPos = result.indexOf('E');
            if (endPos < 0)
                endPos = result.length();
        }

        for (int i = endPos - separate; i >= startPos + 1; i -= separate) {
            result.insert(i, ",");
        }
    }

    return result;
}

/**
 * @brief 该string是否是数字
 */
bool Utils::stringIsDigit(const QString &str)
{
    bool isDigit = true;

    for (auto &ch : str) {
        if (!ch.isDigit() && ch != '.' && ch != ',' && ch != '-') { //非数字条件为非数字小数点分隔符负号
            isDigit = false;
            break;
        }
    }

    return isDigit;
}

/**
 * @brief 针对输入框内容重新设置千位符
 */
QString Utils::reformatSeparators(const QString &exp)
{
    QString seg;
    QStringList expList;

    int count = 0;
    for (auto ch : exp) {
        if (ch.isDigit() || ch == '.') {
            seg.append(ch);
        } else {
            expList << seg;
            seg.clear();
            seg.append(ch);
            expList << seg;
            seg.clear();
        }

        if (count == exp.count() - 1) {
            expList << seg;
        }

        ++count;
    }

    QString formatStr;
    for (auto item : expList) {
        if (stringIsDigit(item)) {
            item = formatThousandsSeparators(item);
        }

        formatStr.append(item);
    }

    return formatStr;
}

QString Utils::formatThousandsSeparatorsPro(const QString &str, const int Base)
{


    QString result = str;
    int startPos = result.indexOf(QRegularExpression("[0-9]"));
    int startPosHex = result.indexOf(QRegularExpression("[A-F0-9]"));
    switch (Base) {
    case 16:
    case 2:
        if (startPosHex >= 0) {
            int endPos = result.length();
            for (int i = endPos - 4; i >= startPosHex + 1; i -= 4) {
                result.insert(i, " ");
            }
        }
        break;
    case 10:
        if (startPos >= 0) {
            int endPos  = result.length();
            int separate = DSettingsAlt::instance()->getSeparate(); //数字分割位数
            for (int i = endPos - separate; i >= startPos + 1; i -= separate) {
                result.insert(i, ",");
            }
        }
        break;
    case 8:
        if (startPos >= 0) {
            int endPos  = result.length();
            for (int i = endPos - 3; i >= startPos + 1; i -= 3) {
                result.insert(i, " ");
            }
        }
        break;
    default:
        break;
    }

    return result;
}

/**
 * @brief Utils::stringIsDigitPro
 * @param str:待判断字符串
 * @param Base:进制
 * @return 是否是数字
 * 程序员模式下判断字符串是否为数字
 */
bool Utils::stringIsDigitPro(const QString &str, const int Base)
{
    bool isDigit = true;

    switch (Base) {
    case 16:
        for (auto &ch : str) {
            if (!ch.isDigit() && ch != ' ' && !isAtoE(ch)) {
                isDigit = false;
                break;
            }
        }
        break;
    case 10:
        for (auto &ch : str) {
            if (!ch.isDigit() && ch != ',' && ch != '-') {
                isDigit = false;
                break;
            }
        }
        break;
    case 8:
    case 2:
        for (auto &ch : str) {
            if (!ch.isDigit() && ch != ' ') {
                isDigit = false;
                break;
            }
        }
        break;
    default:
        for (auto &ch : str) {
            if (!ch.isDigit() && ch != ',' && ch != '-') {
                isDigit = false;
                break;
            }
        }
        break;
    }
    return isDigit;
}

QString Utils::reformatSeparatorsPro(const QString &exp, const int Base)
{
    QString seg;
    QStringList expList;

    int count = 0;
    for (auto ch : exp) {
        if (ch.isDigit() || isAtoE(ch)) {
            seg.append(ch);
        } else {
            expList << seg;
            seg.clear();
            seg.append(ch);
            expList << seg;
            seg.clear();
        }

        if (count == exp.count() - 1) {
            expList << seg;
        }

        ++count;
    }

    QString formatStr;
    for (auto item : expList) {
        if (stringIsDigitPro(item, Base)) {
            item = formatThousandsSeparatorsPro(item, Base);
        }
        formatStr.append(item);
    }

    for (int i = 0; i < formatStr.length();) {
        if (formatStr.at(i).isLower()) {
            if (formatStr.at(i) == 'n' && formatStr.at(i + 1) == 'a') {
                i += 4;
            } else if (formatStr.at(i) == 'o') {
                i += 2;
            } else {
                i += 3;
            }
            if (i <= formatStr.length() - 1) {
                formatStr.insert(i, " ");
                i++;
            }
        } else if (i < formatStr.length() - 1 && formatStr.at(i + 1).isLower()) {
            formatStr.insert(i + 1, " ");
            i += 2;
        } else {
            i++;
        }
    }

    return formatStr;
}

QString Utils::toHalfWidth(const QString &str)
{
    QString result = str;
    if (str.length() <= 0)
        return QString();
    for (int i = 0; i < result.length(); i++) {
        int charIntValue = result.at(i).unicode();
        if (charIntValue >= 65281 && charIntValue <= 65374) {
            result.replace(i, 1, QChar(charIntValue - 65248));
        } else if (charIntValue == 12288) {
            result.replace(i, 1, QChar(32));
        }
    }
    return result;
}

bool Utils::isAtoE(const QChar &num)
{
    return ATOE.contains(num);
}

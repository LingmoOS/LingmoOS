// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"

#include <QFontMetrics>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDBusVariant>

/*!
 * \brief wordwrapText，根据指定换行长度向字符串中添加换行符
 * \param font 当前使用的字体
 * \param text　带换行的字符串
 * \param nLabelLenth　指定换行长度
 */
QString Utils::wordwrapText(const QFontMetrics &font, const QString &text, int nLabelLenth)
{
    int textLenth = font.width(text);
    if (textLenth <= nLabelLenth)
        return text;

    QString retStr;
    QString lineString;
    long lenthTmp = 0; //积累的字符串长度，到达标签长度时重置，并向返回的字符串retStr后添加"\n"
    for (int i = 0; i < text.size(); i++) {
        lineString.append(text.at(i));
        lenthTmp = font.width(lineString);
        if (nLabelLenth < lenthTmp) {
            lineString = lineString.left(lineString.count() - 1);
            i--;
            lineString.append("\n");
            retStr.append(lineString);
            lineString.clear();
        }
    }
    retStr.append(lineString);

    return retStr;
}

// 根据流量数据大小改变单位
QString Utils::changeFlowValueUnit(double input, int prec)
{
    QString flowValueStr;
    if (GB_COUNT < input) {
        //单位为GB
        flowValueStr = QString::number(input / GB_COUNT, 'd', prec) + "GB";
    } else if (MB_COUNT < input) {
        //单位为MB
        flowValueStr = QString::number(input / MB_COUNT, 'd', prec) + "MB";
    } else {
        //单位为KB
        flowValueStr = QString::number(input / KB_COUNT, 'd', prec) + "KB";
    }
    return flowValueStr;
}

// 根据网速大小改变单位
QString Utils::changeFlowSpeedValueUnit(double input, int prec)
{
    QString flowSpeedValueStr;
    if (GB_COUNT < input) {
        //单位为GB
        flowSpeedValueStr = QString::number(input / GB_COUNT, 'd', prec) + "GB/s";
    } else if (MB_COUNT < input) {
        //单位为MB
        flowSpeedValueStr = QString::number(input / MB_COUNT, 'd', prec) + "MB/s";
    } else {
        //单位为KB
        flowSpeedValueStr = QString::number(input / KB_COUNT, 'd', prec) + "KB/s";
    }
    return flowSpeedValueStr;
}

QString Utils::formatBytes(double input, int prec)
{
    QString flowValueStr;
    if (KB_COUNT > input) {
        flowValueStr = QString::number(input / 1, 'd', prec) + "B";
    } else if (MB_COUNT > input) {
        flowValueStr = QString::number(input / KB_COUNT, 'd', prec) + "KB";
    } else if (GB_COUNT > input) {
        flowValueStr = QString::number(input / MB_COUNT, 'd', prec) + "MB";
    } else if (TB_COUNT > input) {
        flowValueStr = QString::number(input / GB_COUNT, 'd', prec) + "GB";
    } else if (TB_COUNT >= input) {
        // 大于TB单位
        flowValueStr = QString::number(input / TB_COUNT, 'd', prec) + "TB";
    }
    return flowValueStr;
}

QString Utils::getPkgNameByFilePathFromDb(const QString &sExecPath, const QString &connectionName)
{
    QSqlDatabase db;

    if (QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }

    db.setDatabaseName(DEFENDER_LOCAL_CACHE_DB_PATH); // /usr/share/deepin-app-store/cache.db

    if (!db.open()) {
        qDebug() << Q_FUNC_INFO << "db open error";
    }

    QString strPackageName;
    QString cmd = QString("select * from package_binary_file where binaryPath = '%1'").arg(sExecPath);
    QSqlQuery query(db);
    bool bRet = query.exec(cmd);
    if (bRet) {
        while (query.next()) {
            strPackageName = query.value("packageName").toString();
        }
    }

    db.close();

    return strPackageName;
}

// 检查并转换QDbusVariant为QVariant
QVariant Utils::checkAndTransQDbusVarIntoQVar(const QVariant &var)
{
    QDBusVariant qdbusVar = var.value<QDBusVariant>();
    QVariant retVar = qdbusVar.variant();
    if (!retVar.isNull()) {
        qDebug() << Q_FUNC_INFO << "returned value is QDBusVariant" << retVar;
        return retVar;
    }
    return var;
}

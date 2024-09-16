// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSQLITEUTIL_H
#define DSQLITEUTIL_H

#include "dfontpreviewitemdef.h"

#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QMutexLocker>

/*************************************************************************
 <Class>         DSqliteUtil
 <Description>   数据库处理类
 <Author>
 <Note>          null
*************************************************************************/
class DSqliteUtil
{
public:
    explicit DSqliteUtil(const QString &strDatabase = "sqlite3db.db");
    ~DSqliteUtil();

    //创建数据库连接
    bool createConnection(const QString &database);

    //创建数据库表
    bool createTable();

    //保存旧数据
    int saveRecord();

    //更新旧数据
    bool updateOld2Record();

    //增加数据
    bool addRecord(QMap<QString, QString> data, const QString &table_name = "t_fontmanager");

    //增加表t_fontmanagerinfo数据
    bool addFontManagerInfoRecord(const QString &table_name = "t_fontmanagerinfo");

    //删除一条记录
    bool delRecord(QMap<QString, QString> where, const QString &table_name = "t_fontmanager");

    //删除所有记录
    bool delAllRecords(const QString &table_name = "t_fontmanager");

    //更新数据
    bool updateRecord(QMap<QString, QString> where, QMap<QString, QString> data,
                      const QString &table_name = "t_fontmanager");

    //更新表t_fontmanagerinfo数据
    bool updateFontManagerInfoRecord(const QString &table_name = "t_fontmanagerinfo");

    //查找所有记录
    bool findAllRecords(const QList<QString> &key, QList<QMap<QString, QString>> &row,
                        const QString &table_name = "t_fontmanager");

    //查找所有记录
    bool findRecords(const QList<QString> &key, QList<QMap<QString, QString>> *row,
                     const QString &table_name = "t_fontmanager");

    //按条件查找
    bool findRecords(const QList<QString> &key, const QMap<QString, QString> &where, QList<QMap<QString, QString>> *row,
                     const QString &table_name = "t_fontmanager");

    //查找表t_fontmanagerinfo所有记录
    bool findFontManagerInfoRecords(const QString &table_name = "t_fontmanagerinfo");

    //获取记录总数
    int getRecordCount(const QString &table_name = "t_fontmanager");
    //获取最大字体id
    int getMaxFontId(const QString &table_name = "t_fontmanager");
    //批量添加字体信息
    void addFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name = "t_fontmanager");
    //批量删除字体信息
    void deleteFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name = "t_fontmanager");
    //批量更新字体信息
    void updateFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &key, const QString &table_name = "t_fontmanager");
    //字符串处理
    QString escapeString(const QString &str);
    //更新sp2的sp3familyName
    void updateSP3FamilyName(const QList<DFontInfo> &fontList, bool inFontList = false);
    //去除非法记录
    void checkIfEmpty();
    //内联函数-执行结束处理函数
    inline void finish()
    {
        if (m_query == nullptr)
            return;
        m_query->finish();
    }
    bool isDBDeleted(){return m_bDbDeleted;}
    void setDBDeleted(bool isDBDeleted)
    {
        m_bDbDeleted = isDBDeleted;
    }

    QSqlDatabase m_db;

    //获取已安装字体路径
    QStringList getInstalledFontsPath();
protected:

private:
    QString m_strDatabase;
    QList<QMap<QString, QString> > m_lstFontRecord;
    bool m_bDbDeleted = false;

    QSqlQuery *m_query;
    mutable QMutex mutex;
};

#endif  // DSQLITEUTIL_H

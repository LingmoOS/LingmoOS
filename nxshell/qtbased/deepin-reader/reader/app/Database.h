// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSet>

class DocSheet;
class Sheet;
class QDateTime;
class DocumentView;

class Transaction
{
public:
    explicit Transaction(QSqlDatabase &database);
    ~Transaction();
    void commit();

private:
    bool m_committed;
    QSqlDatabase &m_database;

};

class Database : public QObject
{
    Q_OBJECT

public:
    static Database *instance();

    ~Database();

    /**
     * @brief readOperation
     * 读取操作
     * @param sheet 哪个文档
     * @return
     */
    bool readOperation(DocSheet *sheet);

    /**
     * @brief saveOperation
     * 保存操作
     * @param sheet 哪个文档
     * @return
     */
    bool saveOperation(DocSheet *sheet);

    /**
     * @brief readBookmarks
     * 读取书签
     * @param filePath 文件名(唯一标识)
     * @param bookmarks 书签列表
     * @return
     */
    bool readBookmarks(const QString &filePath, QSet<int> &bookmarks);

    /**
     * @brief saveBookmarks
     * 保存书签
     * @param filePath 文件名(唯一标识)
     * @param bookmarks 书签列表
     * @return
     */
    bool saveBookmarks(const QString &filePath, const QSet<int> bookmarks);

private:
    Q_DISABLE_COPY(Database)

    static Database *s_instance;

    explicit Database(QObject *parent = nullptr);

    /**
     * @brief prepareOperation
     *准备操作记录表(文档操作)
     * @return
     */
    bool prepareOperation();

    /**
     * @brief prepareBookmark
     * 准备书签表
     * @return
     */
    bool prepareBookmark();

    QSqlDatabase m_database;

};

#endif // DATABASE_H

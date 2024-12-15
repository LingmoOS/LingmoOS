// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QList>

class ItemsPage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    Q_PROPERTY(int maxItemCountPerPage READ maxItemCountPerPage CONSTANT)

public:
    explicit ItemsPage(int maxItemCountPerPage, QObject * parent = nullptr);
    explicit ItemsPage(const QString & name, int maxItemCountPerPage, QObject * parent = nullptr);
    ~ItemsPage();

    QString name() const;
    void setName(const QString & name);

    int maxItemCountPerPage() const;

    int pageCount() const;
    QStringList items(int page = 0) const;
    QStringList firstNItems(int count = 4);
    QStringList allArrangedItems() const;
    QList<QStringList> allPagedItems() const;
    int itemCount() const;
    int itemCount(int page) const;

    void appendEmptyPage();
    void appendPage(const QStringList items);
    void appendItem(const QString id, int page = -1);
    void insertItem(const QString id, int page, int pos = 0);
    void insertItemToPage(const QString &id, int toPage);
    void moveItemPosition(int fromPage, int fromIndex, int toPage, int toIndex, bool appendToIndexItem);
    bool removeItem(const QString id, bool removePageIfPageIsEmpty = true);
    void removeItemsNotIn(const QSet<QString> & itemSet);
    void removeEmptyPages();

    std::tuple<int, int> findItem(const QString & id) const;
    bool contains(const QString & id) const;

private:
    void moveItem(int fromPage, int fromIndex, int toPage, int toIndex);

signals:
    void nameChanged();
    void pageCountChanged();
    void sigPageAoceand(int first, int last);
    void sigPageRemoved(int first, int last);

private:
    int m_maxItemCountPerPage;
    QString m_displayName;
    QList<QStringList> m_pages;
};

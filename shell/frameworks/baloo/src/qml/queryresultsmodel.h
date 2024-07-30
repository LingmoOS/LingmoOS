/*
    SPDX-FileCopyrightText: 2014 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_QUERYRESULTSMODEL_H
#define BALOO_QUERYRESULTSMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <qqmlregistration.h>

class Query : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)

public:
    explicit Query(QObject *parent = nullptr);
    ~Query();

    void setSearchString(const QString &searchString);
    QString searchString() const;

    void setLimit(const int &limit);
    int limit() const;

Q_SIGNALS:
    void searchStringChanged();
    void limitChanged();

private:
    QString m_searchString;
    int m_limit;

};

class QueryResultsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(Query* query READ query WRITE setQuery NOTIFY queryChanged)

public:
    explicit QueryResultsModel(QObject *parent = nullptr);
    ~QueryResultsModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        UrlRole = Qt::UserRole + 1,
    };

    void setQuery(Query *query);
    Query* query() const;

Q_SIGNALS:
    void queryChanged();

private Q_SLOTS:
    void populateModel();

private:
    QStringList m_balooEntryList;
    Query *m_query;
};

#endif

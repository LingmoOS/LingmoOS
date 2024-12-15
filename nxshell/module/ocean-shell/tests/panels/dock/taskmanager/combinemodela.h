// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QHash>
#include <QAbstractListModel>

class TestModelA;

class DataA
{
public:
    DataA(int id, TestModelA* model);
    DataA(int id, const QString &data, TestModelA* model);

    int id();
    QString data();
    void setData(const QString &data);

private:
    TestModelA* m_model;
    int m_id;
    QString m_data;
};

class TestModelA : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        idRole = Qt::UserRole + 1,
        dataRole
    };
    Q_ENUM(Roles)
    TestModelA(QObject *parent = nullptr);
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void addData(DataA *data);
    void removeData(DataA *data);

private:
    QList<DataA*> m_list;
};

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QHash>
#include <QAbstractListModel>

class TestModelB;
class DataB
{
public:
    DataB(int id, TestModelB* model);
    DataB(int id, const QString &data, TestModelB* model);

    int id();
    QString data();
    void setData(const QString &data);

private:
    TestModelB* m_model;
    int m_id;
    QString m_data;
};

class TestModelB : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        idRole = Qt::UserRole + 1,
        dataRole
    };
    Q_ENUM(Roles)
    TestModelB(QObject *parent = nullptr);
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void addData(DataB *data);
    void removeData(DataB *data);


private:
    QList<DataB*> m_list;
};

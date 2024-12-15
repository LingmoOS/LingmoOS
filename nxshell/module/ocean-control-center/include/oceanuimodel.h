// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DCCMODEL_H
#define DCCMODEL_H

#include <QAbstractItemModel>

namespace oceanuiV25 {
class OceanUIObject;

class OceanUIModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(OceanUIObject * root READ root WRITE setRoot NOTIFY rootChanged)
public:
    explicit OceanUIModel(QObject *parent = nullptr);
    ~OceanUIModel() override;

    OceanUIObject *root() const;
    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(const OceanUIObject *object);
    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parentIndex = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
public Q_SLOTS:
    void setRoot(OceanUIObject *root);
    OceanUIObject *getObject(int row);

private Q_SLOTS:
    void updateObject();
    void AboutToAddObject(const OceanUIObject *parent, int pos);
    void addObject(const OceanUIObject *child);
    void AboutToRemoveObject(const OceanUIObject *parent, int pos);
    void removeObject(const OceanUIObject *child);
    void AboutToMoveObject(const OceanUIObject *parent, int pos, int oldPos);
    void moveObject(const OceanUIObject *child);

Q_SIGNALS:
    void rootChanged(OceanUIObject *root);

private:
    void connectObject(const OceanUIObject *obj);
    void disconnectObject(const OceanUIObject *obj);

private:
    OceanUIObject *m_root;
};
} // namespace oceanuiV25
#endif // DCCMODEL_H

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef WORKSPACEMODEL_H
#define WORKSPACEMODEL_H

#include <QAbstractListModel>
#include <QPointer>
namespace dock {

class WorkSpaceData
{
public:
    WorkSpaceData(QString name, QString image)
        : m_name(name)
        , m_image(image)
    {
        
    }
    QString name() {
        return m_name;
    }
    QString image() {
        return m_image;
    }
private:
    QString m_name;
    QString m_image;
};

class WorkspaceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ImageRole,
    };
    Q_ENUM(Roles)

    static WorkspaceModel* instance();

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = ImageRole) const Q_DECL_OVERRIDE;

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    Q_PROPERTY(QList<WorkSpaceData*> items READ items WRITE setItems NOTIFY itemsChanged FINAL)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)
    Q_INVOKABLE void preview(bool show);

    QList<WorkSpaceData *> items() const;
    void setItems(const QList<WorkSpaceData *> &newItems);

    int currentIndex() const;
    void setCurrentIndex(int newCurrentIndex);

signals:
    void itemsChanged();
    void currentIndexChanged(int index);

private:
    explicit WorkspaceModel(QObject *parent = nullptr);

    QList<WorkSpaceData*> m_items;
    QObject *m_worker;
    int m_currentIndex;
};
}
#endif // WORKSPACEMODEL_H

// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef IMLISTMODEL_H
#define IMLISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>

namespace fcitx {
namespace kcm {
class FilteredIMModel;
}
}

namespace lingmo {
namespace fcitx5configtool {

struct IMItem
{
    QString name;
    QString uniqueName;
    // add more ..
};

class IMListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum IMRoles {
        NameRole = Qt::UserRole + 1,
        UniqueName
    };

    explicit IMListModel(QObject *parent = nullptr);
    ~IMListModel() override;

    void resetData(fcitx::kcm::FilteredIMModel *model);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int count() const;
    Q_INVOKABLE bool canMoveUp(int index) const;
    Q_INVOKABLE bool canMoveDown(int index) const;
    Q_INVOKABLE void removeItem(int index);
    Q_INVOKABLE void moveItem(int from, int to);
    Q_INVOKABLE bool canRemove() const;

Q_SIGNALS:
    void requestRemove(int);
    void requestMove(int, int);

private:
    QList<IMItem> m_items;
    fcitx::kcm::FilteredIMModel *m_imModel;
};

}   // namespace fcitx5configtool
}   // namespace lingmo

#endif   // IMLISTMODEL_H

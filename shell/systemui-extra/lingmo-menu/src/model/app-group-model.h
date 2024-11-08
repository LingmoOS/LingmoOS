/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LINGMO_MENU_APP_GROUP_MODEL_H
#define LINGMO_MENU_APP_GROUP_MODEL_H

#include <QAbstractListModel>

#include "commons.h"

namespace LingmoUIMenu {

class AppModel;
class LabelGroupModelPrivate;

class AppGroupModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool containLabel READ containLabel NOTIFY containLabelChanged)
public:
    explicit AppGroupModel(AppModel *appModel, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool containLabel();

    Q_INVOKABLE void openMenu(int labelIndex, int appIndex);
    Q_INVOKABLE void openApp(int labelIndex, int appIndex);
    Q_INVOKABLE int getLabelIndex(const QString &labelId);

Q_SIGNALS:
    void containLabelChanged(bool containLabel);

private Q_SLOTS:
    void reloadLabels();

private:
    void connectSignals();
    inline int getLabelIndex(int index) const;
    inline void resetModel(QVector<LabelItem> &labels);

private:
    LabelGroupModelPrivate *d{nullptr};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_GROUP_MODEL_H

/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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

#ifndef LINGMO_MENU_LABEL_MODEL_H
#define LINGMO_MENU_LABEL_MODEL_H

#include <QAbstractListModel>

#include "commons.h"

namespace LingmoUIMenu {

class LabelModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit LabelModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private Q_SLOTS:
    void reloadLabelData();

private:
    QVector<LabelItem> m_labels;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_LABEL_MODEL_H

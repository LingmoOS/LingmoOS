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

#ifndef LINGMO_MENU_WIDGET_EXTENSION_MODEL_H
#define LINGMO_MENU_WIDGET_EXTENSION_MODEL_H

#include <QAbstractListModel>

namespace LingmoUIMenu {

class WidgetExtension;

class WidgetExtensionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    static WidgetExtensionModel *instance();
    explicit WidgetExtensionModel(QObject *parent = nullptr);

    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE LingmoUIMenu::WidgetExtension *widgetAt(int index) const;
    Q_INVOKABLE void notify(int index, const QVariantMap &data) const;

private:
    QList<WidgetExtension*> m_widgets;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_WIDGET_EXTENSION_MODEL_H

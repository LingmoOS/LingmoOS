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

#ifndef LINGMO_MENU_WIDGET_MODEL_H
#define LINGMO_MENU_WIDGET_MODEL_H

#include <QSortFilterProxyModel>
#include <QVariant>
#include "widget-extension.h"

namespace LingmoUIMenu {

class WidgetModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(LingmoUIMenu::WidgetMetadata::Types types READ types WRITE setTypes)
    Q_PROPERTY(LingmoUIMenu::WidgetMetadata::Flags flags READ flags WRITE setFlags)
public:
    explicit WidgetModel(QObject *parent = nullptr);

    Q_INVOKABLE void init();
    Q_INVOKABLE void send(int index, const QVariantMap &data);

    WidgetMetadata::Types types() const;
    void setTypes(WidgetMetadata::Types types);
    WidgetMetadata::Flags flags() const;
    void setFlags(WidgetMetadata::Flags flags);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    WidgetMetadata::Types m_types = WidgetMetadata::Widget;
    WidgetMetadata::Flags m_flags = WidgetMetadata::Normal;
};

}

#endif //LINGMO_MENU_WIDGET_MODEL_H

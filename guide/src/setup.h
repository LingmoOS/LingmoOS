/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     LingmoOS Team <team@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FIRSTSETUPUI_H
#define FIRSTSETUPUI_H

#include <QObject>
#include <QSettings>

class FirstSetupUI : public QObject
{
    Q_OBJECT

public:
    static FirstSetupUI *self();
    explicit FirstSetupUI(QObject *parent = nullptr);

    Q_INVOKABLE void configInstall();
    
    // QHash<int, QByteArray> roleNames() const override;

    // int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    // QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // FIRSTSETUPUI_H

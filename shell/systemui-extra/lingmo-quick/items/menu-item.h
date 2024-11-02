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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_ITEMS_MENU_ITEM_H
#define LINGMO_QUICK_ITEMS_MENU_ITEM_H

#include <QAction>
#include <QObject>
namespace LingmoUIQuick {

class MenuItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *parent READ parent WRITE setParent)
    Q_PROPERTY(QAction *action READ action WRITE setAction NOTIFY actionChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable NOTIFY checkableChanged)
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY toggled)

public:
    explicit MenuItem(QObject *parent = nullptr);
    QAction *action() const;
    void setAction(QAction *action);

    QString icon() const;
    void setIcon(const QString &icon);

    QString text() const;
    void setText(const QString &text);

    bool checkable() const;
    void setCheckable(bool checkable);

    bool checked() const;
    void setChecked(bool checked);

    bool isEnabled() const;
    void setEnabled(bool enabled);

Q_SIGNALS:
    void clicked();
    void toggled(bool checked);

    void actionChanged();
    void iconChanged();
    void textChanged();
    void checkableChanged();
    void enabledChanged();

private:
    QAction *m_action = nullptr;
};

}

#endif //LINGMO_QUICK_ITEMS_MENU_ITEM_H

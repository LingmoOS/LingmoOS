/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_ACTION_EXTENSION_H
#define LINGMO_QUICK_ACTION_EXTENSION_H

#include <QAction>
#include <QMenu>
#include <QQmlListProperty>

namespace LingmoUIQuick {

class ActionExtensionPrivate;

/**
 * @class ActionExtension
 * QAction只能通过指定QICon设置图标
 * 扩展指定图标名称设置图标功能，增加指定为分割线功能，增加设置子菜单功能
 * @see https://doc.qt.io/qt-5/qtqml-referenceexamples-extended-example.html
 * Usage:
        Action {
            iconName: "file-manager"
            text: "菜单"

            subActions: [
                Action {
                    iconName: "file-manager"
                    text: "子菜单 1"
                    onTriggered: function (checked) {
                        console.log("onTriggered 子菜单 1", checked);
                    }
                },
                Action {
                    // 分割线
                    isSeparator: true
                },
                Action {
                    iconName: "file-manager"
                    text: "子菜单 2"
                    onTriggered: function (checked) {
                        console.log("onTriggered 子菜单 2");
                    }
                }
            ]
        }
 *
 *
 */
class ActionExtension : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(bool isSeparator READ isSeparator WRITE setSeparator NOTIFY isSeparatorChanged)
    Q_PROPERTY(QMenu *menu READ menu WRITE setMenu NOTIFY menuChanged)
    Q_PROPERTY(QQmlListProperty<QAction> subActions READ subActions)
    Q_CLASSINFO("DefaultProperty", "subActions")
public:
    explicit ActionExtension(QObject *parent = nullptr);
    ~ActionExtension() override;

    QString iconName() const;
    void setIconName(const QString &name);

    bool isSeparator() const;
    void setSeparator(bool isSeparator);

    // actions
    QQmlListProperty<QAction> subActions();

    QMenu *menu() const;
    void setMenu(QMenu *menu);

Q_SIGNALS:
    void iconNameChanged();
    void isSeparatorChanged();
    void menuChanged();

private:
    ActionExtensionPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_ACTION_EXTENSION_H

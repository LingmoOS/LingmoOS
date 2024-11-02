/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yan Wang <wangyan@kylinos.cn>
 *
 */

#ifndef KYICON_H
#define KYICON_H
#include <QQuickPaintedItem>
#include <QIcon>
#include <QStyle>
#include <QPointer>

class QStyle;
class KyIcon : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY( bool hover READ hover WRITE setHover NOTIFY hoverChanged)
    Q_PROPERTY( bool selected READ selected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY( bool hasFocus READ hasFocus WRITE sethasFocus NOTIFY hasFocusChanged)
    Q_PROPERTY( bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY( bool sunken READ sunken WRITE setSunken NOTIFY sunkenChanged)
    Q_PROPERTY( bool on READ on WRITE setOn NOTIFY onChanged)
    Q_PROPERTY( QString icontype READ icontype WRITE seticontype NOTIFY icontypeChanged)


public:
    KyIcon(QQuickPaintedItem *parent = nullptr);

    QIcon icon() { return m_icon; }
    void setIcon(const QIcon &icon);
    QString iconName(){ return m_iconName; }
    void setIconName(const QString &iconName);

    bool hover() const { return m_hover; }
    void setHover(bool hover) { if (m_hover != hover) {m_hover = hover ; emit hoverChanged();}}

    bool selected() const { return m_selected; }
    void setSelected(bool selected) {
        if (m_selected!= selected) {
            m_selected = selected;
            emit selectedChanged();
        }
    }

    bool hasFocus() const { return m_focus; }
    void sethasFocus(bool focus) { if (m_focus != focus) {m_focus = focus; emit hasFocusChanged();}}

    bool active() const { return m_active; }
    void setActive(bool active) { if (m_active!= active) {m_active = active; emit activeChanged();}}

    bool sunken() const { return m_sunken; }
    void setSunken(bool sunken) { if (m_sunken != sunken) {m_sunken = sunken; emit sunkenChanged();}}

    bool on() const { return m_on; }
    void setOn(bool on) { if (m_on != on) {m_on = on ; emit onChanged();}}

    QString icontype() const { return m_icontype;}
    void seticontype(QString icontype) {
            m_icontype = icontype ;
            emit icontypeChanged();       
    }

    void paint(QPainter *painter);

    static QStyle *style();

public Q_SLOTS:
    void updateItem(){update();}

Q_SIGNALS:
    void hoverChanged();
    void selectedChanged();
    void hasFocusChanged();
    void activeChanged();
    void sunkenChanged();
    void onChanged();
    void icontypeChanged();
    void iconNameChanged();

protected:
    bool m_hover;
    bool m_selected;
    bool m_focus;
    bool m_active;
    bool m_sunken;
    bool m_on;
    QString m_icontype;

private:
    QIcon m_icon;
    QString m_iconName;

};

#endif // KYICON_H

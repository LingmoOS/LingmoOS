/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QObject>
#include <QRect>

class QAbstractItemModel;

namespace KWin
{
namespace TabBox
{

class SwitcherItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model NOTIFY modelChanged)
    Q_PROPERTY(QRect screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)
    Q_PROPERTY(bool visible READ isVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool allDesktops READ isAllDesktops NOTIFY allDesktopsChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(bool noModifierGrab READ noModifierGrab NOTIFY noModifierGrabChanged)
    Q_PROPERTY(bool compositing READ compositing NOTIFY compositingChanged)

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QObject *item READ item WRITE setItem NOTIFY itemChanged)
    Q_PROPERTY(QString windowColor READ windowColor WRITE setWindowColor NOTIFY windowColorChanged)
    Q_PROPERTY(float windowRadius READ windowRadius WRITE setWindowRadius NOTIFY windowRadiusChanged)
    Q_PROPERTY(float winHoverRadius READ winHoverRadius WRITE setWinHoverRadius NOTIFY winHoverRadiusChanged)
    Q_PROPERTY(QString winHoverColor READ winHoverColor WRITE setWinHoverColor NOTIFY winHoverColorChanged)
    Q_PROPERTY(QRect viewRect READ viewRect NOTIFY viewRectChanged)

    Q_CLASSINFO("DefaultProperty", "item")
public:
    SwitcherItem(QObject *parent = nullptr);
    ~SwitcherItem() override;

    QAbstractItemModel *model() const;
    QRect screenGeometry() const;
    bool isVisible() const;
    bool isAllDesktops() const;
    int currentIndex() const;
    void setCurrentIndex(int index);
    QObject *item() const;
    void setItem(QObject *item);
    QString windowColor() const;
    void setWindowColor(QString windowColor);
    bool noModifierGrab() const
    {
        return m_noModifierGrab;
    }
    bool compositing();
    float windowRadius();
    void setWindowRadius(float);
    float winHoverRadius();
    void setWinHoverRadius(float);
    QString winHoverColor();
    void setWinHoverColor(QString color);
    QRect viewRect() const;
    void setViewRect(const QRect &rect);

    // for usage from outside
    void setModel(QAbstractItemModel *model);
    void setAllDesktops(bool all);
    void setVisible(bool visible);
    void setNoModifierGrab(bool set);

Q_SIGNALS:
    void visibleChanged();
    void currentIndexChanged(int index);
    void modelChanged();
    void allDesktopsChanged();
    void screenGeometryChanged();
    void itemChanged();
    void noModifierGrabChanged();
    void compositingChanged();
    void windowColorChanged(QString windowColor);
    void windowRadiusChanged();
    void winHoverRadiusChanged();
    void winHoverColorChanged();
    void viewRectChanged();

private Q_SLOTS:
    void updateWindowColor(bool active);
    void updateWindowRadius();
    void updateOsTheme();

private:
    QAbstractItemModel *m_model;
    QObject *m_item;
    bool m_visible;
    bool m_allDesktops;
    int m_currentIndex;
    QMetaObject::Connection m_selectedIndexConnection;
    bool m_noModifierGrab = false;
    QString m_windowColor;
    QString m_winHoverColor;
    float m_winRadius;
    float m_winHoverRadius;
    QRect m_viewRect;
};

inline QAbstractItemModel *SwitcherItem::model() const
{
    return m_model;
}

inline bool SwitcherItem::isVisible() const
{
    return m_visible;
}

inline bool SwitcherItem::isAllDesktops() const
{
    return m_allDesktops;
}

inline int SwitcherItem::currentIndex() const
{
    return m_currentIndex;
}

inline QObject *SwitcherItem::item() const
{
    return m_item;
}

} // TabBox
} // KWin

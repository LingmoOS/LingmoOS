// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPoint>
#include <QQmlEngine>
#include <QSize>

namespace docktray {

struct DropIndex {
    Q_GADGET
    Q_PROPERTY(int index MEMBER index)
    Q_PROPERTY(bool isOnItem MEMBER isOnItem)
    Q_PROPERTY(bool isBefore MEMBER isBefore)
    QML_ELEMENT
public:
    int index;
    bool isOnItem = true;
    bool isBefore = false;
};

class TrayItemPositionManager : public QObject
{
    Q_OBJECT
    // dock properties, to notify tray items its property has been changed
    Q_PROPERTY(Qt::Orientation orientation MEMBER m_orientation NOTIFY orientationChanged)
    Q_PROPERTY(int dockHeight MEMBER m_dockHeight NOTIFY dockHeightChanged)
    // position manager properties, when tray items reports visualPositionChanged, re-calculate this
    Q_PROPERTY(QSize visualSize MEMBER m_visualSize NOTIFY visualSizeChanged)
    // position manager properties, use to know how to calculate the actual width of visualSize
    Q_PROPERTY(int visualItemCount MEMBER m_visualItemCount NOTIFY visualItemCountChanged)
    Q_PROPERTY(QSize itemVisualSize MEMBER m_itemVisualSize CONSTANT FINAL)
    Q_PROPERTY(int itemSpacing MEMBER m_itemSpacing CONSTANT FINAL)
    Q_PROPERTY(int itemPadding MEMBER m_itemPadding CONSTANT FINAL)
    QML_ELEMENT
    QML_SINGLETON
public:
    static TrayItemPositionManager &instance()
    {
        static TrayItemPositionManager _instance;
        return _instance;
    }

    static TrayItemPositionManager *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
    {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)
        return &instance();
    }

    void registerVisualItemSize(int index, const QSize & size);
    QSize visualItemSize(int index) const;
    QSize visualSize(int index, bool includeLastSpacing = true) const;
    Q_INVOKABLE DropIndex itemIndexByPoint(const QPoint point) const;
    Qt::Orientation orientation() const;
    int dockHeight() const;
    Q_INVOKABLE void layoutHealthCheck(int delayMs = 200);

signals:
    void orientationChanged(Qt::Orientation);
    void dockHeightChanged(int);
    void visualSizeChanged(QSize);
    void visualItemCountChanged(int);
    void visualItemSizeChanged();

private:
    explicit TrayItemPositionManager(QObject *parent = nullptr);

    void updateVisualSize();

    Qt::Orientation m_orientation;
    QSize m_visualSize;
    int m_dockHeight;
    int m_visualItemCount;
    QList<QSize> m_registeredItemsSize;
    QSize m_itemVisualSize;
    int m_itemSpacing;
    int m_itemPadding;
};

}

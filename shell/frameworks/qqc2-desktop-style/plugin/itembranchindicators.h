/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>
*/

#ifndef ITEMBRANCHINDICATORS_H
#define ITEMBRANCHINDICATORS_H

#include <QModelIndex>
#include <QPalette>
#include <QQuickPaintedItem>

struct PaintData {
    bool hasSibling : 1 = false;
    bool isItem : 1 = false;
};

class ItemBranchIndicators : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QModelIndex modelIndex MEMBER m_index WRITE setModelIndex NOTIFY modelIndexChanged)
    Q_PROPERTY(QModelIndex rootIndex MEMBER m_rootIndex WRITE setRootIndex NOTIFY rootIndexChanged)
    Q_PROPERTY(bool selected MEMBER m_selected WRITE setSelected NOTIFY selectedChanged)
public:
    explicit ItemBranchIndicators(QQuickItem *parent = nullptr);
    void setModelIndex(const QModelIndex &index);
    void setRootIndex(const QModelIndex &index);
    void setSelected(bool selected);
    void paint(QPainter *painter) override;

Q_SIGNALS:
    void modelIndexChanged();
    void rootIndexChanged();
    void selectedChanged();

private:
    void updateParentChain();

    std::vector<PaintData> m_parentChain;

    QPersistentModelIndex m_index;
    QPersistentModelIndex m_rootIndex;
    bool m_selected;
    QPalette m_palette;
};

#endif

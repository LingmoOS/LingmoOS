/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LEGENDLAYOUT_H
#define LEGENDLAYOUT_H

#include <QQuickItem>
#include <qqmlregistration.h>

class LegendLayoutAttached : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    explicit LegendLayoutAttached(QObject *parent = nullptr);

    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    qreal minimumWidth() const;
    void setMinimumWidth(qreal newMinimumWidth);
    Q_SIGNAL void minimumWidthChanged();
    bool isMinimumWidthValid() const;

    Q_PROPERTY(qreal preferredWidth READ preferredWidth WRITE setPreferredWidth NOTIFY preferredWidthChanged)
    qreal preferredWidth() const;
    void setPreferredWidth(qreal newPreferredWidth);
    Q_SIGNAL void preferredWidthChanged();
    bool isPreferredWidthValid() const;

    Q_PROPERTY(qreal maximumWidth READ maximumWidth WRITE setMaximumWidth NOTIFY maximumWidthChanged)
    qreal maximumWidth() const;
    void setMaximumWidth(qreal newMaximumWidth);
    Q_SIGNAL void maximumWidthChanged();
    bool isMaximumWidthValid() const;

private:
    std::optional<qreal> m_minimumWidth;
    std::optional<qreal> m_preferredWidth;
    std::optional<qreal> m_maximumWidth;
};

class LegendLayout : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(LegendLayoutAttached)

public:
    explicit LegendLayout(QQuickItem *parent = nullptr);

    Q_PROPERTY(qreal horizontalSpacing READ horizontalSpacing WRITE setHorizontalSpacing NOTIFY horizontalSpacingChanged)
    qreal horizontalSpacing() const;
    void setHorizontalSpacing(qreal newHorizontalSpacing);
    Q_SIGNAL void horizontalSpacingChanged();

    Q_PROPERTY(qreal verticalSpacing READ verticalSpacing WRITE setVerticalSpacing NOTIFY verticalSpacingChanged)
    qreal verticalSpacing() const;
    void setVerticalSpacing(qreal newVerticalSpacing);
    Q_SIGNAL void verticalSpacingChanged();

    Q_PROPERTY(qreal preferredWidth READ preferredWidth NOTIFY preferredWidthChanged)
    qreal preferredWidth() const;
    Q_SIGNAL void preferredWidthChanged();

    static LegendLayoutAttached *qmlAttachedProperties(QObject *object)
    {
        return new LegendLayoutAttached(object);
    }

protected:
    void componentComplete() override;
    void updatePolish() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data) override;

private:
    void updateItem(int index, QQuickItem *item);
    std::tuple<int, int, qreal, qreal> determineColumns();

    qreal m_horizontalSpacing = 0.0;
    qreal m_verticalSpacing = 0.0;
    qreal m_preferredWidth = 0.0;

    bool m_completed = false;
};

#endif // LEGENDLAYOUT_H

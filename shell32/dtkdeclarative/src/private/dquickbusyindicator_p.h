// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKBUSYINDICATOR_P_H
#define DQUICKBUSYINDICATOR_P_H

#include <dtkdeclarative_global.h>

#include <QtGui/qcolor.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/private/qsgadaptationlayer_p.h>

DQUICK_BEGIN_NAMESPACE

class DQuickBusyIndicator;
class DQuickBusyIndicatorNode : public QObject, public QSGTransformNode
{
    Q_OBJECT

public:
    DQuickBusyIndicatorNode(DQuickBusyIndicator *item);

    void setSpinning(bool spinning);
    void sync(QQuickItem *item);

public Q_SLOTS:
    void maybeRotate();
    void maybeUpdate();
    void setFillColor(const QColor &color);

private:
    int updateIndicatorColors(const QColor &fill);

private:
    bool m_spinning;
    float m_rotation;
    QPointF m_center;
    QQuickWindow *m_window;
    QList<QList<QColor>> m_indicatorColors;
};

class DQuickBusyIndicator : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning)

public:
    explicit DQuickBusyIndicator(QQuickItem *parent = nullptr);

    QColor fillColor() const;
    bool isRunning() const;

public Q_SLOTS:
    void setFillColor(const QColor &color);
    void setRunning(bool running);

protected:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

Q_SIGNALS:
    void fillColorChanged();

private:
    QColor m_fillColor;
    bool m_isRunning;
    bool m_fillColorIsChanged;
};

DQUICK_END_NAMESPACE

QML_DECLARE_TYPE(DTK_QUICK_NAMESPACE::DQuickBusyIndicator)

#endif // DQUICKBUSYINDICATOR_P_H

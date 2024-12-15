// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PATHVIEWRANGEHANDLER_H
#define PATHVIEWRANGEHANDLER_H

#include <QObject>
#include <QPointF>
#include <QQuickItem>

class PathViewRangeHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(bool enableForward READ enableForward WRITE setEnableForward NOTIFY enableForwardChanged FINAL)
    Q_PROPERTY(bool enableBackward READ enableBackward WRITE setEnableBackward NOTIFY enableBackwardChanged FINAL)

public:
    explicit PathViewRangeHandler(QObject *parent = nullptr);
    ~PathViewRangeHandler() = default;

    QQuickItem *target() const;
    void setTarget(QQuickItem *view);
    Q_SIGNAL void targetChanged();

    bool enableForward() const;
    void setEnableForward(bool b);
    Q_SIGNAL void enableForwardChanged();

    bool enableBackward() const;
    void setEnableBackward(bool b);
    Q_SIGNAL void enableBackwardChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QQuickItem *targetView { nullptr };
    bool enableForwardFlag { true };   // 允许向前
    bool enableBackwardFlag { true };  // 允许向后
    QPointF basePoint;
};

#endif  // PATHVIEWRANGEHANDLER_H

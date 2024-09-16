// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUSETRACKITEM_H
#define MOUSETRACKITEM_H

#include <QObject>
#include <QQuickItem>

class MouseTrackItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool pressed READ pressed NOTIFY pressedChanged)

public:
    explicit MouseTrackItem(QQuickItem *parent = nullptr);

    void setPressed(bool press);
    bool pressed() const;
    Q_SIGNAL void pressedChanged();

    Q_SIGNAL void doubleClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    bool isPressed = false;
};

#endif  // MOUSETRACKITEM_H

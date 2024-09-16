// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETICONBUTTON_H
#define NETICONBUTTON_H

#include <QIcon>
#include <QWidget>

namespace dde {
namespace network {

class NetIconButton : public QWidget
{
    Q_OBJECT
public:
    explicit NetIconButton(QWidget *parent = nullptr);

public Q_SLOTS:
    void setIcon(const QIcon &icon);
    void setHoverIcon(const QIcon &icon);

    void setClickable(bool clickable);
    void setRotatable(bool rotatable);

signals:
    void clicked();

protected:
    bool event(QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void startRotate();
    void stopRotate();

private:
    QTimer *m_refreshTimer;
    QIcon m_icon;
    QIcon m_hoverIcon;
    QPoint m_pressPos;
    int m_rotateAngle;
    bool m_clickable;
    bool m_rotatable;
    bool m_hover;
};

} // namespace network
} // namespace dde
#endif // NETICONBUTTON_H

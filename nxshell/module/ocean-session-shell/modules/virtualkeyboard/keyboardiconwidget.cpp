// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyboardiconwidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QSvgRenderer>

const static QSize iconSize = QSize(30, 30);

KeyboardIconWidget::KeyboardIconWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(iconSize);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void KeyboardIconWidget::setIconPath(const QString &iconPath)
{
    m_iconPath = iconPath;
    update();
}

bool KeyboardIconWidget::event(QEvent *event) {
  if (event->type() == QEvent::User + 10) {
    QWidget *topLevelWidget = this->topLevelWidget();
    if (topLevelWidget) {
      // 获取顶层窗口，虚拟键盘窗口中需要知道顶层窗口以便移动位置
      Q_EMIT clicked(topLevelWidget);
    }
  }

  return QWidget::event(event);
}


void KeyboardIconWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPixmap pixmap(iconSize);
    pixmap.load(m_iconPath);
    pixmap.fill(Qt::transparent);

    QSvgRenderer renderer(m_iconPath);
    renderer.render(&painter);

    int x = (rect().width() - iconSize.width()) / 2;
    int y = (rect().height() - iconSize.height()) / 2;
    painter.drawPixmap(x, y, iconSize.width(), iconSize.height(), pixmap);

    QWidget::paintEvent(event);
}

void KeyboardIconWidget::hideEvent(QHideEvent *event)
{
    Q_EMIT iconWidgetHided();

    if (this->topLevelWidget() && !topLevelWidget()->isVisible()) {
        // 通过判断顶层窗口隐藏来触发
        Q_EMIT topLevelWidgetHided();
    }

    QWidget::hideEvent(event);
}

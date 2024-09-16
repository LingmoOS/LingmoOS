// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEMORYLISTWIDGET_H
#define MEMORYLISTWIDGET_H

#include <QListWidget>

/**
 * @brief 内存ListWidget
 */
class MemoryListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit MemoryListWidget(QWidget *parent = nullptr, bool isarrowlist = false);
    ~MemoryListWidget() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void cleanState(bool isfocus = false);

signals:
    void itempressed(int row);
    void itemselected(int row, bool isselect);
    void focus(int direction); //direction 0-上　1-下　2-左　3-右
    void space();
    void altAndM();
    void mousemoving();

private:
    QPoint m_mousepoint;
    QPoint m_mousemovepoint;//鼠标的实时全局位置
    int m_clickrow = -1;
    bool m_isarrowlist = false;

public slots:
//    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
};

#endif // MEMORYLISTWIDGET_H

// SPDX-FileCopyrightText: 2017-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>

class LeftAreaTextEdit;
class TextEdit;

class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    explicit LineNumberArea(LeftAreaTextEdit *leftAreaWidget);
    ~LineNumberArea() override;
    QPoint getPressPoint();

protected:
    void paintEvent(QPaintEvent *e) override;
    QSize sizeHint() const override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    LeftAreaTextEdit *m_leftAreaWidget;
    QPoint m_pressPoint;
};

#endif

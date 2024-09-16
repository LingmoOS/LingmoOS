// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPLICABLELABEL_H
#define REPLICABLELABEL_H

#include <QLineEdit>

namespace GrandSearch {

class ReplicableLabel : public QLineEdit
{
    Q_OBJECT
public:
    explicit ReplicableLabel(QWidget *parent = nullptr);

    void setText(const QString &text);

    void setElideMode(Qt::TextElideMode elideMode);
    Qt::TextElideMode elideMode() const;

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;


private:
    Qt::TextElideMode m_elideMode;      // 截断显示模式
    QString m_fullText;                 // 文本-用于复制
    QString m_elideText;                // 截断文本-用于显示
    bool m_pressed = false;             // 记录是否按照鼠标左键
};

}

#endif // REPLICABLELABEL_H

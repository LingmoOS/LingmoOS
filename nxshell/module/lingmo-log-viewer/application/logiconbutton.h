// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MYICONBUTTON_H
#define MYICONBUTTON_H

#include <QPushButton>
/**
 * @brief The LogIconButton class 详细信息部分显示等级图标和文字内容的无边框button
 */
class LogIconButton : public QPushButton
{
public:
    explicit LogIconButton(QWidget *parent = nullptr);
    explicit LogIconButton(const QString &text, QWidget *parent = nullptr);
protected:
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent *e)override;
    void mousePressEvent(QMouseEvent *e) override;
    bool event(QEvent *e) override;
};

#endif  // MYICONBUTTON_H

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOPROCESSWIDGET_H
#define NOPROCESSWIDGET_H

#include <QWidget>

namespace Dtk {
namespace Widget {
    class DSpinner;
    class DLabel;
}  // namespace Widget
}  // namespace Dtk

class NoProcessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NoProcessWidget(QWidget *parent = nullptr);

    void setActionText(const QString &text);
    void start();
    void stop();

protected:
    bool event(QEvent *e);

private:
    Dtk::Widget::DSpinner *spinner;
    Dtk::Widget::DLabel *actionTextLabel;
};

#endif  // NOPROCESSWIDGET_H

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSWIDGET_H
#define PROCESSWIDGET_H

#include <QWidget>
#include <QIcon>

namespace Dtk {
namespace Widget {
    class DLabel;
    class DProgressBar;
}  // namespace Widget
}  // namespace Dtk

class ProcessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProcessWidget(QWidget *parent = nullptr);

    void setIcon(const QIcon &icon);
    void setMainText(const QString &text);
    void setProcessText(const QString &text);
    void setProgress(int current, int all);

private:
    Dtk::Widget::DLabel *mainIcon;
    Dtk::Widget::DLabel *mainLabel;
    Dtk::Widget::DLabel *processTextLabel;
    Dtk::Widget::DProgressBar *processBar;

    QString processText;
};

#endif  // PROCESSWIDGET_H

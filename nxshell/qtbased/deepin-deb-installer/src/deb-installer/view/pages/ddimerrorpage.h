// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDIMERRORPAGE_H
#define DDIMERRORPAGE_H

#include <QWidget>

namespace Dtk {
namespace Widget {
    class DLabel;
}
}  // namespace Dtk

class QPushButton;

class DdimErrorPage : public QWidget
{
    Q_OBJECT
public:
    explicit DdimErrorPage(QWidget *parent = nullptr);
    void setErrorMessage(const QString &message);

signals:
    void comfimPressed();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Dtk::Widget::DLabel *errorMessageLabel;
    Dtk::Widget::DLabel *errorPicLabel;
    QPushButton *confimButton;
};

#endif  // DDIMERRORPAGE_H

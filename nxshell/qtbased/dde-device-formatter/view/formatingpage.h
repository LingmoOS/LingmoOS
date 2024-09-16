// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FORMATINGPAGE_H
#define FORMATINGPAGE_H

#include <QFrame>

namespace Dtk {
namespace Widget {
class DWaterProgress;
}
}

class FormatingPage : public QFrame
{
    Q_OBJECT
public:
    explicit FormatingPage(QWidget *parent = 0);
    void initUI();
    void setProgress(double p);

public slots:
private:
    Dtk::Widget::DWaterProgress* m_progressBox = NULL;
};

#endif // FORMATINGPAGE_H

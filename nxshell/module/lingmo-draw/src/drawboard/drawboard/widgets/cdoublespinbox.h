// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDOUBLESPINBOX_H
#define CDOUBLESPINBOX_H

#include <QWidget>
#include "globaldefine.h"

#ifdef USE_DTK
#include <DDoubleSpinBox>
#define CDOUBLESPINBOXFATHERCLASS DDoubleSpinBox
DWIDGET_USE_NAMESPACE
#else
#include <QDoubleSpinBox>
#define CDOUBLESPINBOXFATHERCLASS QDoubleSpinBox
#endif

/**
 * @brief 用于过滤部分信号，当输入值完成输入及按步变更时
 *      发送数值变更信号。
 */
class CDoubleSpinBox : public CDOUBLESPINBOXFATHERCLASS
{
    Q_OBJECT

public:
    explicit CDoubleSpinBox(QWidget *parent = nullptr);

    // 数值按步变更时触发，判断是否向外抛出信号
    virtual void stepBy(int steps) override;
    // 数值变更时触发信号
    Q_SIGNAL void phaseValueChanged(double value, EChangedPhase phase);

private:
    PRIVATESAFECLASS(CDoubleSpinBox)
};

#endif // CDOUBLESPINBOX_H

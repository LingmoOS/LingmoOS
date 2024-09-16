// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cdoublespinbox.h"

#include <QLineEdit>
#include <QTimer>

/**
 * @brief 内部私有类
 */
class CDoubleSpinBox::CDoubleSpinBox_private
{
public:
    explicit CDoubleSpinBox_private(CDoubleSpinBox *q)
        : q_ptr(q)
    {
        m_finishedTimer.setSingleShot(true);
    }

    CDoubleSpinBox *q_ptr;
    QTimer          m_finishedTimer;    // 超时更新定时
};

/**
 * @brief CDoubleSpinBox::CDoubleSpinBox
 * @param parent
 */
CDoubleSpinBox::CDoubleSpinBox(QWidget *parent)
    : CDOUBLESPINBOXFATHERCLASS(parent)
    , CDoubleSpinBox_d(new CDoubleSpinBox_private(this))
{
    // 当前控件输入框完成编辑时触发
    connect(this, &QDoubleSpinBox::editingFinished, this, [ this ]() {
        Q_EMIT phaseValueChanged(value(), EChangedFinished);
    });

    // 超过50ms的按步数值变更，才会更新变更完成信号
    connect(&d_CDoubleSpinBox()->m_finishedTimer, &QTimer::timeout, this, [ this ]() {
        Q_EMIT phaseValueChanged(value(), EChangedFinished);
    });
}

/**
 * @brief 输入框按步 \a steps 变更时，判断数据是否变化，
 *      若变更则发送数据更新信号。
 * @param steps 输入框数据变化的步数
 */
void CDoubleSpinBox::stepBy(int steps)
{
    double old = value();
    DDoubleSpinBox::stepBy(steps);
    if (!qFuzzyCompare(old, value())) {
        Q_EMIT phaseValueChanged(value(), EChangedUpdate);

        // 启动超时判断定时，超时50ms后的按步数值变更才会被认为完成
        d_CDoubleSpinBox()->m_finishedTimer.start(50);
    }
}

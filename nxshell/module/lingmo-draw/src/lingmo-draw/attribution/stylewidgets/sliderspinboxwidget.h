// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SLIDERSPINBOXWIDGET_H
#define SLIDERSPINBOXWIDGET_H
#include <DSlider>

#include "attributewidget.h"
DWIDGET_USE_NAMESPACE

class QLabel;
class CSpinBox;

/**
 * @brief 滑动条和数值输入框的组合控件，用于属性值展示及设置
 */
class SliderSpinBoxWidget : public AttributeWgt
{
    Q_OBJECT

public:
    /**
     * @brief 用于区分显示不同的输入框风格
     */
    enum BoxStyle {
        EIntegerStyle,      ///< 整数输入框风格
        EPercentStyle,      ///< 百分数输入框风格
    };

    explicit SliderSpinBoxWidget(int attri, BoxStyle style = EIntegerStyle, QWidget *parent = nullptr);

    // 更新设置当前显示的侧边数
    virtual void setVar(const QVariant &var) override;

    // 设置数值滑动和输入的范围
    void setRange(int min, int max);
    // 设置属性控件的标题
    void setTitle(const QString &title);

    // 设置混合态时用于显示的特殊文本
    void setSpecialText(QString text = QString("..."));

    QLineEdit *lineEdit();
Q_SIGNALS:
    // 值变更信号
    void sigValueChanged(int value, EChangedPhase phase);

private:
    // 初始化布局，添加滑动条和数字输入框
    void initUi(BoxStyle style);
    // 初始化信号连接
    void initConnection();

private:
    QLabel      *m_title = nullptr;
    DSlider     *m_slider = nullptr;
    CSpinBox    *m_spinBox = nullptr;
};

#endif // SLIDERSPINBOXWIDGET_H

// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QLabel>
#include <QVBoxLayout>

#include "sliderspinboxwidget.h"
#include "boxlayoutwidget.h"
#include "cspinbox.h"

/**
 * @class SliderSpinboxwidget
 * @brief 滑动条和数值输入框的组合控件，用于属性值展示及设置
 */

/**
 * @brief 构造函数，根据 \a style 初始化不同的控件，\a attri 用于基类标识当前属性控件类型
 * @param attri     控件控制的属性类型
 * @param style     输入框风格
 * @param parent    父窗口指针
 */
SliderSpinBoxWidget::SliderSpinBoxWidget(int attri, BoxStyle style, QWidget *parent)
    : AttributeWgt(attri, parent)
{
    initUi(style);
    initConnection();
}

/**
 * @param var 外部设置更新的属性值 \a var , 数值一般来源drawboard，
 *      根据传入的 \a var 是否有效切换混合态，混合态下输入框显示 '...' 文本，
 *      滑动条为最小值
 */
void SliderSpinBoxWidget::setVar(const QVariant &var)
{
    // 判断数值是否有效，无效数据来自不同属性值的图元
    if (var.isValid()) {
        int value = var.toInt();

        // 来自drawborad的更新，不向外发送更新信号
        QSignalBlocker sliderLocker(m_slider);
        m_slider->setValue(value);
        QSignalBlocker bokLocker(m_spinBox);
        m_spinBox->setSpinValue(value);
    } else {
        // 设置当前控件为混合状态，显示 '...' 特殊文本
        setSpecialText();
    }
}

/**
 * @brief 设置滑动条和输入框允许的数值变化范围为 \a min ~ \a max
 * @param min 最小允许数值
 * @param max 最大允许数值
 */
void SliderSpinBoxWidget::setRange(int min, int max)
{
    m_slider->slider()->setRange(min, max);
    m_spinBox->setSpinRange(min, max);
}

/**
 * @param title 属性标题，当前所用于的属性名称
 */
void SliderSpinBoxWidget::setTitle(const QString &title)
{
    m_title->setText(title);
}

/**
 * @param text 设置混合态时输入框显示 '...' 特殊文本，同时将滑动条置为最小值
 */
void SliderSpinBoxWidget::setSpecialText(QString text)
{
    // CSpinbox已处理不向外发送信号
    m_spinBox->setSpecialText(text);

    // 混合态时，滑块置于最小值位置
    QSignalBlocker sliderLocker(m_slider);
    m_slider->setValue(m_slider->minimum());

    //! \todo 特殊情况：如果选中单个文本框，且该文本框中，如果文本框内文字大小不同，滑条取文本框内字号的最大值。
    //!     根据风格 BoxStyle 切换？
}

QLineEdit *SliderSpinBoxWidget::lineEdit()
{
    return m_spinBox->lineEdit();
}


/**
 * @brief 根据不同的输入框显示风格 \a style 初始化界面布局
 * @param style 输入框显示风格，包括整数和百分数
 */
void SliderSpinBoxWidget::initUi(SliderSpinBoxWidget::BoxStyle style)
{
    this->setFocusPolicy(Qt::NoFocus);
    m_title = new QLabel(this);
    m_slider = new DSlider(Qt::Horizontal, this);
    m_spinBox = new CSpinBox(this);

    // 根据不同风格切换数值显示框
    switch (style) {
    case EPercentStyle:
        m_spinBox->setSuffix(QString("%"));
        break;
    // 默认使用整数输入框
    default:
        break;
    }
    m_spinBox->setEnabledEmbedStyle(true);


    BoxLayoutWidget *contextWid = new BoxLayoutWidget(this);
    contextWid->setMargins(8);
    contextWid->setBackgroundRect(true);
    contextWid->setFrameRect(false);
    contextWid->addWidget(m_slider, 2);
    contextWid->addWidget(m_spinBox, 1);

    QVBoxLayout *lay = new QVBoxLayout;
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_title);
    lay->addWidget(contextWid);
    setLayout(lay);
}

/**
 * @brief 初始化信号连接，关联滑动条和输入框数值变更时发送更新信号
 */
void SliderSpinBoxWidget::initConnection()
{
    // 不同的滑动条阶段使用发送不同更新指令
    connect(m_slider, &DSlider::sliderPressed, this, [ = ]() {
        Q_EMIT sigValueChanged(m_slider->value(), EChangedBegin);
    });
    // 滑动过程中更新
    connect(m_slider, &DSlider::valueChanged, this, [ = ](int value) {
        Q_EMIT sigValueChanged(value, EChangedUpdate);

        // 同步更新输入框
        QSignalBlocker locker(m_spinBox);
        m_spinBox->setValue(value);
    });
    connect(m_slider, &DSlider::sliderReleased, this, [ = ]() {
        Q_EMIT sigValueChanged(m_slider->value(), EChangedFinished);
    });

    // CSpinBox已实现阶段信号
    connect(m_spinBox, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
        Q_EMIT sigValueChanged(value, phase);

        // 同步更新滑动条
        QSignalBlocker locker(m_slider);
        m_slider->setValue(value);
    });
}

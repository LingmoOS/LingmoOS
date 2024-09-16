// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBINFOLABEL_H
#define DEBINFOLABEL_H

#include <DLabel>

DWIDGET_USE_NAMESPACE

/**
 * @brief The DebInfoLabel class
 *  singleInstallPage 中 package name、 package version 与 tipsLabel 的控件
 *  基于DLabel 对不同情况下的Dlabel进行界面上的风格样式修改，保证界面上风格的统一
 *
 */
class DebInfoLabel : public DLabel
{
    Q_OBJECT
public:
    explicit DebInfoLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /**
     * @brief setCustomQPalette 设置自适应的QPalette
     * @param colorRole  字体颜色角色
     * 目前使用到此类型的有 singleInstallPage的packgeName packageVersion 以及他们提示的label
     */
    void setCustomQPalette(QPalette::ColorRole colorRole);

    /**
     * @brief setCustomDPalette 设置自适应的DPalette
     * @param colorType 字体颜色的类型
     * 目前使用到此类型的有 singleInstallPage的m_tipsLabel(单包安装时状态[依赖状态][安装状态][版本状态]提示)
     */
    void setCustomDPalette(DPalette::ColorType colorType);

    /**
     * @brief setCustomDPalette 设置默认的DPalette
     * 目前没有函数使用到此类型
     */
    void setCustomDPalette();

    void paintEvent(QPaintEvent *event) override;

private:
    QPalette::ColorRole m_colorRole;  // 当前label的字体颜色角色（QPalette）
    DPalette::ColorType m_colorType;  // 当前Label的字体颜色类型（DPalette）

    bool m_bUserColorType = false;  // 是否是使用的DPalette
    bool m_bMultiIns = false;       // 是否是使用的自定义DPalette风格
};

#endif  // DEBINFOLABEL_H

// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLURWIDGET_H
#define BLURWIDGET_H

#include "globaldefine.h"
#include "cattributeitemwidget.h"

#include <DWidget>
#include <DSlider>
#include <DLabel>
#include <DToolButton>

class CSpinBox;

DWIDGET_USE_NAMESPACE

struct SBLurEffect {
    int width = 20;
    int type = 1;
};

Q_DECLARE_METATYPE(SBLurEffect)

class BlurWidget : public DrawAttribution::CAttriBaseOverallWgt
{
    Q_OBJECT

public:
    explicit BlurWidget(DWidget *parent = nullptr);
    ~BlurWidget();

    /**
     * @brief setBlurWidth 设置模糊宽度
     */
    void setBlurWidth(const int &width, bool emitSig = true);

    /**
     * @brief setBlurType 设置模糊类型
     */
    void setBlurType(const EBlurEffect &blurEffect, bool emitSig = true);

    /**
     * @brief getEffect 获取属性值
     */
    SBLurEffect getEffect() const;

signals:
//    void blurTypeChanged(EBlurEffect);
//    void blurWidthChanged(int);
    void blurEffectChanged(const SBLurEffect &effect);

private:
    /**
     * @brief initUI 初始化ＵＩ
     */
    void initUI();
    /**
     * @brief initConnection 初始化连接
     */
    void initConnection();

private:
    QButtonGroup *m_TypeButtons = nullptr;

    DLabel   *m_pLineWidthLabel;
    CSpinBox *m_spinboxForLineWidth = nullptr;
};

#endif // BLURWIDGET_H

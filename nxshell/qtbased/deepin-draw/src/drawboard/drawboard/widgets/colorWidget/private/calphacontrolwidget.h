// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CALPHACONTROLWIDGET_H
#define CALPHACONTROLWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>

#include "globaldefine.h"
#include "adaptivedef.h"

#ifdef USE_DTK
#include <DLineEdit>
#include <DSlider>
DWIDGET_USE_NAMESPACE
#endif

class CAlphaControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CAlphaControlWidget(QWidget *parent = nullptr);

    /**
     * @brief setAlpha　通过颜色setAlpha值更新控件
     * @param internalChanged　由内发生的变化
     */
    void setAlpha(int alpha, bool internalChanged = true);

    /**
     * @brief alphaValue　当前的alpha值
     */
    int alpha();

signals:
    /**
     * @brief alphaChanged　alpha值改变信号
     */
    void alphaChanged(int apl, EChangedPhase phase);

private:
    /**
     * @brief initUI　初始化ＵＩ
     */
    void initUI();
    /**
     * @brief initConnection　初始化连接
     */
    void initConnection();

private:
    DSlider *m_alphaSlider;
    DLineEdit *m_alphaLabel;
};

#endif // CALPHACONTROLWIDGET_H

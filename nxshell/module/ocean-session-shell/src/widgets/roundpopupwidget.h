// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROUND_POPUP_WIDGET_H
#define ROUND_POPUP_WIDGET_H

#include <DBlurEffectWidget>

class QVBoxLayout;

using DTK_WIDGET_NAMESPACE::DBlurEffectWidget;
/*!
 * \brief The RoundPopupWidget class
 * 圆角弹窗页面
 */
class RoundPopupWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit RoundPopupWidget(QWidget *parent = nullptr);

public:
    void setContent(QWidget *widget);
    QWidget *getContent() const;

protected:
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();

private:
    QWidget *m_pContentWidget;
    QVBoxLayout *m_mainLayout;
    QWidget *m_savedParent;
};


#endif // ROUND_POPUP_WIDGET_H

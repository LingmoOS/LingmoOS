// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADJUSTMENTATRRIWIDGET_H
#define ADJUSTMENTATRRIWIDGET_H

#include <QWidget>
#include <DToolButton>

#include "attributewidget.h"

DWIDGET_USE_NAMESPACE
class DPushButton;

class AdjustmentAtrriWidget : public AttributeWgt
{
    Q_OBJECT
public:
    explicit AdjustmentAtrriWidget(QWidget *parent = nullptr);
    DToolButton *button();
    virtual void setVar(const QVariant &var) override;
private:
    void initUi();
signals:

private:
    DToolButton *m_adjustmentBtn = nullptr;
    QLabel *m_titleLabel = nullptr;
};

#endif // ADJUSTMENTATRRIWIDGET_H

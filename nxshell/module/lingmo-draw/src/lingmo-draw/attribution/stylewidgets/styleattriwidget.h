// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STYLEATTRIWIDGET_H
#define STYLEATTRIWIDGET_H
#include <DLineEdit>
#include <DLabel>

#include "attributewidget.h"

DWIDGET_USE_NAMESPACE
class QLabel;
class StyleAttriWidget : public AttributeWgt
{
    Q_OBJECT
public:
    explicit StyleAttriWidget(int attri, QWidget *parent = nullptr);
    QVBoxLayout *getLayout();
    void addChildAtrri(AttributeWgt *attri, bool bAddLayout = true);
    void removeChildAtrri(AttributeWgt *attri);
    void setVar(const QVariant &var) override;

private:
    void initUi();
public:
    StyleAttriWidget   *m_extend = nullptr;
private:
    DLabel             *m_label = nullptr;
    QVBoxLayout        *m_layout = nullptr;
    QList<AttributeWgt *>         m_childAtrri;
};

#endif // STYLEATTRIWIDGET_H

// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILLSTYLEWIDGET_H
#define FILLSTYLEWIDGET_H
#include "styleattriwidget.h"
#include <DCheckBox>
DWIDGET_USE_NAMESPACE

class CColorSettingButton;
class ColorStyleWidget : public AttributeWgt
{
    Q_OBJECT
public:
    explicit ColorStyleWidget(QWidget *parent = nullptr);
    virtual void setVar(const QVariant &var) override;
    void setTitleText(QString text);
    void setColorFill(int fillStyle);//0 fill 1 border
    void setColorTextVisible(bool bVisible);
    void addWidget(QWidget *w, int stretch = 0);
    void addTitleWidget(QWidget *w, Qt::Alignment alignment);
    void setContentEnable(bool enable);
    DLineEdit *lineEdit();
signals:
    void colorChanged(const QColor &color, int phase);
private:
    void initUi();
    void enableColorEdit(bool bEnable);
private:
    CColorSettingButton *m_fillColor = nullptr;
    DLineEdit           *m_fillColorEdit = nullptr;
    DLabel              *m_titleLabel = nullptr;
    QHBoxLayout         *m_lFillColor = nullptr;
    QHBoxLayout         *m_titleLayout = nullptr;
    QList<QWidget *>     m_addWidgets;
};

#endif // FILLSTYLEWIDGET_H

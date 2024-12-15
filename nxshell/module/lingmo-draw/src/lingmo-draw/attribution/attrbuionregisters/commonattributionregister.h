// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ATTRIBUTIONREGISTER_H
#define ATTRIBUTIONREGISTER_H
#include "iattributionregister.h"

class GroupButtonWidget;
class RotateAttriWidget;
class OrderWidget;

class ColorStyleWidget;
class RectRadiusStyleWidget;
class CSpinBox;

class SliderSpinBoxWidget;
class ComboBoxSettingWgt;
class CheckBoxSettingWgt;

class GroupAttriRegister : public IAttributionRegister
{
    Q_OBJECT
public:
    using IAttributionRegister::IAttributionRegister;
    virtual void registe() override;
private:
    void updateGroupStatus();
private:
    GroupButtonWidget   *m_groupWidget = nullptr;
};

class RotateAttriRegister : public IAttributionRegister
{
    Q_OBJECT
public:
    using IAttributionRegister::IAttributionRegister;
    virtual void registe() override;
private:
    RotateAttriWidget   *m_rotateAttri = nullptr;
};

class OrderAttriRegister : public IAttributionRegister
{
    Q_OBJECT
public:
    using IAttributionRegister::IAttributionRegister;
    virtual void registe() override;
private:
    OrderWidget *m_orderAttri = nullptr;
};

class CommonAttributionRegister : public IAttributionRegister
{
    Q_OBJECT
public:
    using IAttributionRegister::IAttributionRegister;
    virtual void registe() override;
    QList<QWidget *> getStyleAttriWidgets();
private:
    void registeGroupAttri();
    void resgisteRotateAttri();
    void registeBaseStyleAttrri();
    void registeOrderAttri();
private:
    bool m_isInit = false;      // 判断是否已调用函数

    ColorStyleWidget    *m_fillBrushStyle = nullptr;
    ColorStyleWidget    *m_borderPenStyle = nullptr;
    CSpinBox            *m_penWidth = nullptr;

    CheckBoxSettingWgt  *m_enablePenStyle = nullptr;
    CheckBoxSettingWgt  *m_enableBrushStyle = nullptr;
};

#endif // ATTRIBUTIONREGISTER_H

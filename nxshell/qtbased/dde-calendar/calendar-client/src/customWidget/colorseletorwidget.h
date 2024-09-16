// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COLORSELETOR_H
#define COLORSELETOR_H

#include "cradiobutton.h"
#include "colorWidget/colorpickerWidget.h"
#include "scheduledatamanage.h"
#include "dschedule.h"
#include "dtypecolor.h"
#include "accountmanager.h"

#include <QWidget>
#include <QColor>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMap>

//色彩控件选择类
class ColorSeletorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorSeletorWidget(QWidget *parent = nullptr);

    /**
     * @brief reset
     * 重置
     */
    void reset();
    /**
     * @brief getSelectColor
     * @return 当前已选在的色彩
     */
    DTypeColor::Ptr getSelectedColorInfo();
    /**
     * @brief setUserColor 设置用户自定义的色彩
     */
    void setUserColor(const DTypeColor::Ptr &);
    /**
     * @brief setSelectedColorByIndex 设置选择的色彩控件
     * @param index 色彩控件位置
     */
    void setSelectedColorByIndex(int index);
    /**
     * @brief setSelectedColorById 设置选择的色彩控件
     * @param colorId 色彩id
     */
    void setSelectedColorById(int colorId);
    /**
     * @brief setSelectedColor 设置选择的色彩控件
     * @param color 色彩实例
     */
    void setSelectedColor(const DTypeColor&);
    /**
     * @brief resetColorButton
     * 重置色彩控件
     * @param account 帐户
     */
    void resetColorButton(const AccountItem::Ptr& account);

signals:
    //选择的色彩改变信号
    void signalColorChange(DTypeColor::Ptr);

public slots:
    //色彩控件点击信号
    void slotButtonClicked(int butId);
    //添加自定义色彩控件点击信号
    void slotAddColorButClicked();

private:
    void init();
    void initView();

    //添加色彩控件
    void addColor(const DTypeColor::Ptr &);

private:
    QMap<int, DTypeColor::Ptr> m_colorEntityMap; //所有色彩实体
    QHBoxLayout *m_colorLayout = nullptr;     //色彩控件布局类
    QButtonGroup *m_colorGroup = nullptr;     //所有色彩控件
    DTypeColor::Ptr m_colorInfo; //当前已选择的色彩
    CRadioButton *m_userColorBtn = nullptr; //用户自定义的色彩控件
    const int m_userColorBtnId = 999; //用户自定义的色彩控件id
};

#endif // COLORSELETOR_H

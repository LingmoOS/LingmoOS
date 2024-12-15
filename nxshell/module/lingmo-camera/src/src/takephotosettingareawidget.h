// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAKEPHOTOSETTINGAREAWIDGET_H
#define TAKEPHOTOSETTINGAREAWIDGET_H

#include "filterpreviewbutton.h"
#include "exposureslider.h"

#include <QWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEvent>
#include <QKeyEvent>

#include "exposureslider.h"

class QPalette;
class circlePushButton;
class CMainWindow;

class takePhotoSettingAreaWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int opacity READ getOpacity WRITE setOpacity)//自定义不透明度属性
public:
    explicit takePhotoSettingAreaWidget(QWidget *parent = nullptr);
    /**
    * @brief init 初始化
    */
    void init();
    /**
    * @brief showFold 显示收起按钮
    * @param bShow 是否显示
    */
    void showFold(bool bShow, bool isShortcut = false);
    /**
    * @brief showUnfold 显示展开
    * @param bShow 是否显示
    * @param btn 返回焦点的按钮
    * @param isShortcut 是否通过快捷键触发
    */
    void showUnfold(bool bShow, circlePushButton *btn, bool isShortcut = false);
    /**
    * @brief showDelayButtons 显示延时拍照按钮
    * @param bShow 是否显示
    */
    void showDelayButtons(bool bShow, bool isShortcut = false);
    /**
    * @brief showFlashlights 显示闪光灯按钮
    * @param bShow 是否显示
    * @param isShortcut 是否通过快捷键触发
    */
    void showFlashlights(bool bShow, bool isShortcut = false);

    /**
    * @brief showFilters 显示滤镜按钮
    * @param bShow 是否显示
    * @param isShortcut 是否通过快捷键触发
    */
    void showFilters(bool bShow, bool isShortcut = false);

    /**
    * @brief setDelayTime 设置延时拍照时间
    * @param delayTime 延迟的时间
    */
    void setDelayTime(int delayTime);
    /**
    * @brief setDelayTime 设置是否打开了闪光灯
    * @param bFlashOn 是否打开闪光
    */
    void setFlashlight(bool bFlashOn);

    /**
    * @brief setDelayTime 获取闪光灯状态
    */
    bool flashLight();

    /**
    * @brief setFilterType 设置滤镜类型
    * @param type 滤镜类型
    */
    void setFilterType(efilterType type);

    /**
    * @brief filterType 获取滤镜类型
    */
    efilterType getFilterType();

    /**
    * @brief moveToParentLeft 移动到父窗口左边
    */
    void moveToParentLeft();

    /**
    * @brief closeAllGroup 关闭所有的按钮弹出组
    */
    void closeAllGroup();

    /**
    * @brief getOpacity 获取当前不透明度
    */
    int getOpacity() { return m_opacity; };

    /**
    * @brief setOpacity 设置不透明度 动画会通过这个函数输入插值，修改透明度。
    * @param  opacity 不透明度 0～255
    */
    void setOpacity(int opacity);

    /**
     * @brief setState 设置拍照，录像状态
     * @param bPhoto  true 拍照状态， false 录像状态
     */
    void setState(bool bPhoto);

    /**
     * @brief resizeScrollHeight 更新滤镜窗口高度
     * @param height 主窗口提升高度
     */
    void resizeScrollHeight(int height);

private:
    /**
    * @brief initButtons 初始化所有的按钮
    */
    void initButtons();
    /**
    * @brief initLayout 按钮位置布局
    */
    void initLayout();
    /**
    * @brief hideAll 隐藏所有按钮
    */
    void hideAll();

    void keyDownClick();
    void keyUpClick();
    void keyEnterClick();

protected:
    /**
    * @brief paintEvent 绘制事件
    * @param  event 事件参数
    */
    void paintEvent(QPaintEvent *event) override;

    /**
    * @brief resizeEvent 尺寸大小改变事件
    * @param  event 事件参数
    */
    void resizeEvent(QResizeEvent *event) override;

    /**
    * @brief keyReleaseEvent 尺寸大小改变事件
    * @param  event 事件参数
    */
    void keyReleaseEvent(QKeyEvent *event) override;

    /**
    * @brief focusInEvent 焦点进入事件
    * @param  event 事件参数
    */
    void focusInEvent(QFocusEvent *event) override;

signals:
    void sngSetDelayTakePhoto(int delayTime);
    void sngSetFlashlight(bool bFlashlightOn);
    void sngFilterChanged(efilterType efilterType);
    void sngShowFilterName(bool bShow);
    void sngSetFilterName(const QString& name);
    void sigExposureChanged(int exposure);
    void sigFilterGroupDisplay(bool bDisplay);

public slots:
    /**
    * @brief foldBtnClicked 折叠按钮点击槽函数
    * @param isShortcut 是否通过键盘触发
    */
    void foldBtnClicked(bool isShortcut = false);

    /**
    * @brief unfoldBtnClicked 展开按钮点击槽函数
    * @param isShortcut 是否通过键盘触发
    */
    void unfoldBtnClicked(bool isShortcut = false);

    /**
    * @brief flashlightFoldBtnClicked 闪光灯折叠按钮点击槽函数
    * @param isShortcut 是否通过键盘触发
    */
    void flashlightFoldBtnClicked(bool isShortcut = false);

    /**
    * @brief flashlightFoldBtnClicked 闪光灯展开按钮点击槽函数
    * @param isShortcut 是否通过键盘触发
    */
    void flashlightUnfoldBtnClicked(bool isShortcut = false);

    /**
    * @brief delayUnfoldBtnClicked 延时展开按钮点击槽函数
    * @param isShortcut 是否通过键盘触发
    */
    void delayUnfoldBtnClicked(bool isShortcut = false);

    /**
    * @brief delayfoldBtnClicked 延时折叠按钮点击槽函数
    * @param isShortcut 是否通过键盘触发
    */
    void delayfoldBtnClicked(bool isShortcut = false);

    /**
    * @brief filtersUnfoldBtnsClicked 滤镜展开按钮点击槽函数
    */
    void filtersUnfoldBtnClicked(bool isShortcut = false);

    /**
    * @brief filtersFoldBtnsClicked 滤镜折叠按钮点击槽函数
    */
    void filtersFoldBtnClicked(bool isShortcut = false);

    /**
    * @brief onDelayBtnsClicked 延时拍照按钮点击槽函数
    */
    void onDelayBtnsClicked(bool isShortcut = false);

    /**
    * @brief onDelayBtnsClicked 闪光开关按钮点击槽函数
    */
    void onFlashlightBtnsClicked(bool isShortcut = false);

    /**
    * @brief onFilterPreviewBtnsClicked 滤镜预览选择按钮点击槽函数
    */
    void onFilterBtnsClicked(bool isShortcut = false);

    /**
    * @brief onUpdateFilterImage 更新预览帧图片
    * @param img 预览帧图片
    */
    void onUpdateFilterImage(QImage* img);

    /**
    * @brief onExposureValueChanged 曝光值改变槽函数
    * @param value 曝光值
    */
    void onExposureValueChanged(int value);

    /**
    * @brief exposureBtnClicked 曝光按钮点击槽函数
    * @param isShortcut 是否通过键盘触发
    */
    void exposureBtnClicked(bool isShortcut = false);

private:
    filterPreviewButton* getFilterPreviewFocusBtn();

private:
    circlePushButton        *m_foldBtn;//折叠按钮
    circlePushButton        *m_unfoldBtn;//展开按钮

    circlePushButton        *m_flashlightFoldBtn;//折叠闪光按钮
    circlePushButton        *m_flashlightUnfoldBtn;//展开闪光
    circlePushButton        *m_flashlightOnBtn;//闪光开启按钮
    circlePushButton        *m_flashlightOffBtn;//闪光关闭按钮

    circlePushButton        *m_delayFoldBtn;//折叠延迟拍照
    circlePushButton        *m_delayUnfoldBtn;//展开延迟拍照
    circlePushButton        *m_noDelayBtn;//无延迟
    circlePushButton        *m_delay3SecondBtn;//延迟3s
    circlePushButton        *m_delay6SecondBtn;//延迟6s

    circlePushButton        *m_filtersFoldBtn = nullptr;//折叠滤镜按钮
    circlePushButton        *m_filtersUnfoldBtn = nullptr;//展开滤镜按钮

    filterPreviewBtnList     m_filterPreviewBtnList;//滤镜预览按钮列表
    circlePushButton        *m_filtersCloseBtn = nullptr;//关闭滤镜界面按钮

    circlePushButton        *m_exposureBtn = nullptr;//曝光按钮
    ExposureSlider          *m_exposureSlider;

    QScrollArea             *m_scrollArea;//滤镜窗口
    QWidget                 *m_scrollAreaWidget;//滤镜滚动条窗口

    int                      m_btnHeightOffset;//按钮之间的间隔
    int                      m_threeBtnOffset;//三级菜单按钮之间的间隔
    int                      m_scrollHeight;//滚动窗口默认高度

    QColor                   m_buttonGroupColor;//按钮组展开后背景颜色

    bool                     m_delayGroupDisplay;//延迟按钮组是否显示
    bool                     m_flashGroupDisplay;//闪光灯按钮组是否显示
    bool                     m_filtersGroupDislay = false;//滤镜按钮组是否显示
    bool                     m_exposureSliderDisplay = false;//曝光按钮组是否显示

    bool                     m_bPhotoToVideState; //当前是否由拍照状态切换到录像状态

    bool                     m_isBtnsFold;  //按钮组是否收起
    int                      m_opacity = 102;    //不透明度 默认值0.4 * 255

    bool                     m_bPhoto;          //true 拍照状态， false 录像状态

    efilterType               m_filterType;      //当前选中的滤镜类型

    qint64                  m_animationTime = 0;    //记录动画执行时间
};

#endif // TAKEPHOTOSETTINGAREAWIDGET_H

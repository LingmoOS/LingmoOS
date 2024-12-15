// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUBTOOLWIDGET_H
#define SUBTOOLWIDGET_H

#include "toolbutton.h"
#include "../utils/saveutils.h"
#include "filter.h"

#include <DLabel>
#include <DStackedWidget>

#include <QObject>

DWIDGET_USE_NAMESPACE
class MainWindow;
class SubToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    explicit SubToolWidget(MainWindow *pmainwindow, DWidget *parent = nullptr);
    ~SubToolWidget();

    void initWidget();
    /**
     * @brief 初始化录屏工具栏
     */
    void initRecordLabel();
    /**
     * @brief 初始化截图工具栏
     */
    void initShotLabel();
    /**
     * @brief 初始化滚动截图工具栏
     */
    void initScrollLabel();
    /**
     * @brief 安装工具栏的提示
     * @param w : 工具按钮
     * @param hintstr : 提示信息
     */
    void installTipHint(QWidget *w, const QString &hintstr);
    /**
     * @brief 安装工具栏提示的触发
     * @param w : 工具按钮
     * @param hint : 提示信息
     */
    void installHint(QWidget *w, QWidget *hint);

    /**
     * @brief 设置禁止滚动截图
     */
    void setScrollShotDisabled(const bool state);

    /**
     * @brief 设置贴图的激活状态
     */
    void setPinScreenshotsEnable(const bool &state);

    void setOcrScreenshotEnable(const bool &state);

    void setButEnableOnLockScreen(const bool &state);

    /**
     * @brief 根据当前传入的按钮形状获取当前按钮坐标的x值
     * @param func:当前按钮的形状（目前：矩形、圆形、直线等）
     * @return 当前按钮坐标的x值
     */
    int getFuncSubToolX(QString &func);

    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void keyBoardButtonClicked(bool checked);
    void mouseBoardButtonClicked(bool checked);
    void mouseShowButtonClicked(bool checked);
    void cameraActionChecked(bool checked);
    /**
     * @brief 截图功能选择的工具被改变
     * @param 工具的名称
     */
    void changeShotToolFunc(const QString &func);
    void saveMethodChanged(SaveAction saveact, const QString path);
    void saveToClipBoard(bool isClip);
public slots:
    /**
     * @brief 切换截图功能或者录屏功能
     * @param shapeType : "record" or "shot"
     */
    void switchContent(QString shapeType);
    void setRecordButtonDisable();
    void setRecordLaunchMode(const unsigned int funType);
    void setVideoButtonInitFromSub();
    void shapeClickedFromWidget(QString shape);
    void setMicroPhoneEnable(bool status);
    void setCameraDeviceEnable(bool status);
    void setSystemAudioEnable(bool status);
    /**
     * @brief 当m_microphoneAction或m_systemAudioAction被点击或者程序主动调用trigg()时，会触发工具栏音频
     * 采集图标的改变及发射实际需要录制的音频
     * @param checked
     */
    void onChangeAudioType(bool checked);
private:
    void initShotOption();
    void initRecordOption();
    /**
     * @brief 录屏功能工具栏
     */
    DLabel *m_recordSubTool = nullptr;
    /**
     * @brief 截屏功能工具栏
     */
    DLabel *m_shotSubTool = nullptr;
    /**
     * @brief 滚动截图功能工具栏
     */
    DLabel *m_scrollSubTool = nullptr;
    QString m_currentType;
    /**
     * @brief 滚动截图工具栏按钮
     */
    ToolButton *m_scrollShotButton = nullptr;
    /**
     * @brief ocr识别工具栏按钮
     */
    ToolButton *m_ocrButton = nullptr;
    ToolButton *m_ocrScrollButton = nullptr;
    /**
     * @brief 贴图工具栏按钮
     */
    ToolButton *m_pinButton = nullptr;
    ToolButton *m_cancelButton = nullptr;
    ToolButton *m_recorderButton = nullptr;
    /**
     * @brief 截图功能中矩形工具按钮
     */
    ToolButton *m_rectButton = nullptr;
    /**
     * @brief 截图功能中圆形工具按钮
     */
    ToolButton *m_circleButton = nullptr;
    /**
     * @brief 截图功能中直线工具按钮
     */
    ToolButton *m_lineButton = nullptr;
    /**
     * @brief 截图功能中箭头工具按钮
     */
    ToolButton *m_arrowButton = nullptr;
    /**
     * @brief 截图功能中画笔工具按钮
     */
    ToolButton *m_penButton = nullptr;
    /**
     * @brief 截图功能中模糊工具按钮
     */
    ToolButton *m_mosaicButton = nullptr;
    /**
     * @brief 截图功能中文本编辑工具按钮
     */
    ToolButton *m_textButton = nullptr;
    /**
     * @brief 录屏功能中摄像头工具按钮
     */
    ToolButton *m_cameraButton = nullptr;
    /**
     * @brief 录屏功能中键盘工具按钮
     */
    ToolButton *m_keyBoardButton = nullptr;
    /**
     * @brief 录屏功能中选项按钮
     */
    ToolButton *m_optionButton = nullptr;
    ToolButton *m_scrollOptionButton = nullptr;
    ToolButton *m_shotButton = nullptr;
    /**
     * @brief 截图功能中选项按钮
     */
    ToolButton *m_shotOptionButton = nullptr;
    HintFilter *hintFilter = nullptr;
    DMenu *m_optionMenu = nullptr;
    DMenu *m_scrollOptionMenu = nullptr;
    DMenu *m_recordOptionMenu = nullptr;
    QAction *m_microphoneAction = nullptr;
    QButtonGroup *m_shotBtnGroup = nullptr;

    /**
     * @brief m_saveToSpecialPathMenu 截图选项: 保存到指定位置的菜单
     */
    DMenu *m_saveToSpecialPathMenu = nullptr;
    /**
     * @brief m_saveToSpecialPathAction 截图选项: 历史保存路径
     */
    QAction *m_saveToSpecialPathAction = nullptr;
    /**
     * @brief m_changeSaveToSpecialPath 截图选项: 设置或更新指定位置
     */
    QAction *m_changeSaveToSpecialPath = nullptr;

    /**
     * @brief m_scrollSaveToSpecialPathMenu 滚动截图选项: 保存到指定位置的菜单
     */
    DMenu *m_scrollSaveToSpecialPathMenu = nullptr;
    /**
     * @brief m_scrollSaveToSpecialPathAction 滚动截图选项: 历史保存路径
     */
    QAction *m_scrollSaveToSpecialPathAction = nullptr;
    /**
     * @brief m_scrollChangeSaveToSpecialPath 滚动截图选项: 设置或更新指定位置
     */
    QAction *m_scrollChangeSaveToSpecialPath = nullptr;
    MainWindow *m_pMainWindow = nullptr;
};

#endif // SUBTOOLWIDGET_H

// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subtoolwidget.h"
#include "../camera_process.h"
#include "../utils/configsettings.h"
#include "../settings.h"
#include "tooltips.h"
#include "../utils.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"
#include "imagemenu.h"

#include <DSlider>
#include <DLineEdit>
#include <DMenu>
#include <DInputDialog>
#include <DFontSizeManager>
#include <DComboBox>
#include <DListWidget>

#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QDebug>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <QThread>

#include <unistd.h>


DWIDGET_USE_NAMESPACE

namespace {
const QSize TOOL_ICON_SIZE = QSize(36, 36);
const QSize TOOL_BUTTON_SIZE = QSize(36, 36);
const QSize MEDIUM_TOOL_BUTTON_SIZE = QSize(56, 36);
}

SubToolWidget::SubToolWidget(MainWindow *pmainwindow, DWidget *parent) : DStackedWidget(parent)
{
    m_pMainWindow = pmainwindow;
    initWidget();
    connect(m_pMainWindow, SIGNAL(microPhoneEnable(bool)), this, SLOT(setMicroPhoneEnable(bool)));
}

void SubToolWidget::initWidget()
{
    hintFilter = new HintFilter(this);
    initRecordLabel();
    initShotLabel();
    setCurrentWidget(m_shotSubTool);
}

void SubToolWidget::initRecordLabel()
{
    qDebug() << "正在初始化录屏工具栏UI...";
    m_recordSubTool = new DLabel(this);
    QList<ToolButton *> btnList;

    //添加显示键盘按钮
    m_keyBoardButton = new ToolButton();
    Utils::setAccessibility(m_keyBoardButton, AC_SUBTOOLWIDGET_KEYBOARD_BUTTON);
    m_keyBoardButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_keyBoardButton, tr("Show keystroke (K)"));
    m_keyBoardButton->setIcon(QIcon::fromTheme("key_mormal"));
    m_keyBoardButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_keyBoardButton);
    //发送键盘按键按钮状态信号
    connect(m_keyBoardButton, &DPushButton::clicked, this, [ = ] {
        if (m_keyBoardButton->isChecked())
        {
            installTipHint(m_keyBoardButton, tr("Hide Keystroke (K)"));
        } else
        {
            installTipHint(m_keyBoardButton, tr("Show Keystroke (K)"));
        }
        emit keyBoardButtonClicked(m_keyBoardButton->isChecked());
    });

    //添加摄像头显示按钮
    m_cameraButton = new ToolButton();
    m_cameraButton->setDisabled((QCameraInfo::availableCameras().count() <= 0));
    Utils::setAccessibility(m_cameraButton, AC_SUBTOOLWIDGET_CAMERA_BUTTON);
    m_cameraButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_cameraButton, tr("Turn on camera (C)"));
    m_cameraButton->setIcon(QIcon::fromTheme("webcam_normal"));
    m_cameraButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_cameraButton);
    connect(m_cameraButton, &DPushButton::clicked, this, [ = ] {
        qDebug() << "点击摄像头开启/关闭按钮！";
        if (m_cameraButton->isChecked())
        {
            installTipHint(m_cameraButton, tr("Turn off camera (C)"));
        } else
        {
            installTipHint(m_cameraButton, tr("Turn on camera (C)"));
        }
        qDebug() << "正在发射摄像头点击信号...";
        emit cameraActionChecked(m_cameraButton->isChecked());
        qDebug() << "已发射摄像头点击信号";
    });


    m_shotButton = new ToolButton();
    m_shotButton->setCheckable(false);
    Utils::setAccessibility(m_shotButton, AC_SUBTOOLWIDGET_SHOT_BUTTON);
    m_shotButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_shotButton, tr("Screenshot"));
    m_shotButton->setIcon(QIcon::fromTheme("shot"));
    m_shotButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_shotButton);
    connect(m_shotButton, &DPushButton::clicked, this, [ = ] {
        m_pMainWindow->getToolBarPoint();
        qInfo() << "shotbutton is clicked";
        switchContent("shot");
        qInfo() << "emit changeShotToolFunc(shot)";
        emit changeShotToolFunc("shot");
    });

    //添加录屏选项按钮
    m_optionButton = new ToolButton();
    m_optionButton->setOptionButtonFlag(true);
    DPalette pa = m_optionButton->palette();
    if (Utils::themeType == 1) {
        pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
        pa.setColor(DPalette::Dark, QColor(192, 192, 192, 255));
        pa.setColor(DPalette::Light, QColor(192, 192, 192, 255));
    } else {
        pa.setColor(DPalette::ButtonText, QColor(228, 228, 228, 255));
        pa.setColor(DPalette::Dark, QColor(64, 64, 64, 255));
        pa.setColor(DPalette::Light, QColor(64, 64, 64, 255));
    }
    m_optionButton->setPalette(pa);
    m_optionButton->setCheckable(false);
    m_optionButton->setFlat(false);
    m_optionButton->setHoverState(false);
    DFontSizeManager::instance()->bind(m_optionButton, DFontSizeManager::T6);
    Utils::setAccessibility(m_optionButton, AC_SUBTOOLWIDGET_RECORD_OPTION_BUT);
    m_optionButton->setText(tr("Settings"));
    m_optionButton->setMinimumSize(MEDIUM_TOOL_BUTTON_SIZE);
    installTipHint(m_optionButton, tr("Settings (F3)"));
    btnList.append(m_optionButton);

    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setSizeConstraint(QLayout::SetFixedSize);
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(10);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        if (btnList[i] == m_shotButton) {
            rectLayout->addSpacing(10);
        }
    }
    m_recordSubTool->setLayout(rectLayout);
    addWidget(m_recordSubTool);

    initRecordOption();

    qDebug() << "录屏工具栏UI已初始化";
}
void SubToolWidget::initRecordOption()
{
    qDebug() << "正在初始化录屏工具栏选项UI...";
    QActionGroup *t_formatGroup = new QActionGroup(this);
    QActionGroup *t_fpsGroup = new QActionGroup(this);
    QActionGroup *t_audioGroup = new QActionGroup(this);
    QActionGroup *t_mouseInfoGroup = new QActionGroup(this);
    //录屏保存位置的组
    QActionGroup *t_saveGroup = new QActionGroup(this);
    t_formatGroup->setExclusive(true);
    t_fpsGroup->setExclusive(true);
    t_audioGroup->setExclusive(false);
    t_mouseInfoGroup->setExclusive(false);
    t_saveGroup->setExclusive(true);
    m_recordOptionMenu = new DMenu(this);
    m_recordOptionMenu->installEventFilter(this);
    DFontSizeManager::instance()->bind(m_recordOptionMenu, DFontSizeManager::T8);
    // 保存格式
    QAction *formatTitleAction = new QAction(tr("Format:"), m_recordOptionMenu);
    QAction *gifAction = new QAction(tr("GIF"));
    QAction *mp4Action = new QAction(tr("MP4"), m_recordOptionMenu);
    QAction *mkvAction = new QAction(tr("MKV"), m_recordOptionMenu);
    if (!Utils::isFFmpegEnv) {
        mp4Action->setText(tr("webm"));
    }
    // 帧数
    QAction *fpsTitleAction = new QAction(tr("FPS:"), m_recordOptionMenu);
    QAction *fps5Action = new QAction(tr("5 fps"), m_recordOptionMenu);
    QAction *fps10Action = new QAction(tr("10 fps"), m_recordOptionMenu);
    QAction *fps20Action = new QAction(tr("20 fps"), m_recordOptionMenu);
    QAction *fps24Action = new QAction(tr("24 fps"), m_recordOptionMenu);
    QAction *fps30Action = new QAction(tr("30 fps"), m_recordOptionMenu);
    // 声音
    QAction *audio = new QAction(tr("Sound"), m_recordOptionMenu);
    //QAction *notAudio = new QAction(tr("Not Audio"), m_recordOptionMenu);
    m_microphoneAction = new QAction(tr("Microphone"), m_recordOptionMenu);
    QAction *sysAudio = new QAction(tr("System audio"), m_recordOptionMenu);
    // 选项
    QAction *mouseInfo = new QAction(tr("Options"), m_recordOptionMenu);
    //QAction *notMouse = new QAction(tr("Not Mouse"), m_recordOptionMenu);
    QAction *showPointer = new QAction(tr("Show pointer"), m_recordOptionMenu);
    QAction *showClick = new QAction(tr("Show click"), m_recordOptionMenu);
    //保存位置
    QAction *saveTitleAction = new QAction(m_recordOptionMenu);
    QAction *saveToDesktopAction = new QAction(m_recordOptionMenu);
    QAction *saveToVideoAction = new QAction(m_recordOptionMenu);

    Utils::setAccessibility(gifAction, "gifAction");
    Utils::setAccessibility(mp4Action, "mp4Action");
    Utils::setAccessibility(mkvAction, "mkvAction");
    Utils::setAccessibility(fps5Action, "fps5Action");
    Utils::setAccessibility(fps10Action, "fps10Action");
    Utils::setAccessibility(fps20Action, "fps20Action");
    Utils::setAccessibility(fps24Action, "fps24Action");
    Utils::setAccessibility(fps30Action, "fps30Action");

    formatTitleAction->setDisabled(true);
    gifAction->setCheckable(true);
    mp4Action->setCheckable(true);
    mkvAction->setCheckable(true);
    t_formatGroup->addAction(gifAction);
    t_formatGroup->addAction(mp4Action);
    t_formatGroup->addAction(mkvAction);

    fpsTitleAction->setDisabled(true);
    fps5Action->setCheckable(true);
    fps10Action->setCheckable(true);
    fps20Action->setCheckable(true);
    fps24Action->setCheckable(true);
    fps30Action->setCheckable(true);
    t_fpsGroup->addAction(fps5Action);
    t_fpsGroup->addAction(fps10Action);
    t_fpsGroup->addAction(fps20Action);
    t_fpsGroup->addAction(fps24Action);
    t_fpsGroup->addAction(fps30Action);

    audio->setDisabled(true);
    //notAudio->setCheckable(true);
    m_microphoneAction->setCheckable(true);
    sysAudio->setCheckable(true);
    sysAudio->setChecked(true);
    //t_audioGroup->addAction(notAudio);
    t_audioGroup->addAction(m_microphoneAction);
    t_audioGroup->addAction(sysAudio);

    mouseInfo->setDisabled(true);
    //notMouse->setCheckable(true);
    showPointer->setCheckable(true);
    showClick->setCheckable(true);
    //t_mouseInfoGroup->addAction(notMouse);
    t_mouseInfoGroup->addAction(showPointer);
    t_mouseInfoGroup->addAction(showClick);

    saveTitleAction->setDisabled(true);
    saveTitleAction->setText(tr("Save to"));
    saveToDesktopAction->setText(tr("Desktop"));
    saveToDesktopAction->setCheckable(true);
    saveToVideoAction->setText(tr("Videos"));
    saveToVideoAction->setCheckable(true);
    t_saveGroup->addAction(saveToDesktopAction);
    t_saveGroup->addAction(saveToVideoAction);

    m_recordOptionMenu->addAction(formatTitleAction);
#if !(defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__))
    if (Utils::isFFmpegEnv) {
        m_recordOptionMenu->addAction(gifAction);
    }
#endif
    m_recordOptionMenu->addAction(mp4Action);
    if (Utils::isFFmpegEnv) {
        m_recordOptionMenu->addAction(mkvAction);
    }
    //m_recordOptionMenu->addSeparator();

    m_recordOptionMenu->addAction(fpsTitleAction);
    m_recordOptionMenu->addAction(fps5Action);
    m_recordOptionMenu->addAction(fps10Action);
    m_recordOptionMenu->addAction(fps20Action);
    m_recordOptionMenu->addAction(fps24Action);
    m_recordOptionMenu->addAction(fps30Action);
    //m_recordOptionMenu->addSeparator();

    m_recordOptionMenu->addAction(audio);
    //m_recordOptionMenu->addAction(notAudio);
    m_recordOptionMenu->addAction(m_microphoneAction);
    m_recordOptionMenu->addAction(sysAudio);
    //m_recordOptionMenu->addSeparator();

    m_recordOptionMenu->addAction(mouseInfo);
    //m_recordOptionMenu->addAction(notMouse);
    m_recordOptionMenu->addAction(showPointer);
    m_recordOptionMenu->addAction(showClick);

    m_recordOptionMenu->addAction(saveTitleAction);
    m_recordOptionMenu->addAction(saveToDesktopAction);
    m_recordOptionMenu->addAction(saveToVideoAction);

    m_recordOptionMenu->hide();
    m_optionButton->setMenu(m_recordOptionMenu);

    ConfigSettings *t_settings = ConfigSettings::instance();
    int save_format = t_settings->getValue("recorder", "format").toInt();
    int frame_rate = t_settings->getValue("recorder", "frame_rate").toInt();
    //在GStreamer环境下，录屏格式只有webm，因此录屏格式webm一定会被选中
    if (!Utils::isFFmpegEnv) {
        if (save_format != 1) {
            save_format = 1;
        }
    }

    //0 1 2; GIF,MP4,MKV
    if (save_format == 0) {
        gifAction->setChecked(true);
        gifAction->trigger();
        fps5Action->setEnabled(false);
        fps10Action->setEnabled(false);
        fps20Action->setEnabled(false);
        fps24Action->setEnabled(false);
        fps30Action->setEnabled(false);
    } else if (save_format == 1) {
        mp4Action->setChecked(true);
        mp4Action->trigger();
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
    } else {
        mkvAction->setChecked(true);
        mkvAction->trigger();
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
    }

    connect(gifAction, &QAction::triggered, this, [ = ](bool checked) {
        Q_UNUSED(checked);
        t_settings->setValue("recorder", "format", 0);
        fps5Action->setEnabled(false);
        fps10Action->setEnabled(false);
        fps20Action->setEnabled(false);
        fps24Action->setEnabled(false);
        fps30Action->setEnabled(false);
    });

    connect(mp4Action, &QAction::triggered, this, [ = ](bool checked) {
        Q_UNUSED(checked);
        t_settings->setValue("recorder", "format", 1);
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
    });

    connect(mkvAction, &QAction::triggered, this, [ = ](bool checked) {
        Q_UNUSED(checked);
        t_settings->setValue("recorder", "format", 2);
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
    });

    connect(t_fpsGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        int t_frameRate = 24;
        if (t_act == fps5Action) {
            t_frameRate = 5;
        } else if (t_act == fps10Action) {
            t_frameRate = 10;
        } else if (t_act == fps20Action) {
            t_frameRate = 20;
        } else if (t_act == fps24Action) {
            t_frameRate = 24;
        } else if (t_act == fps30Action) {
            t_frameRate = 30;
        }
        t_settings->setValue("recorder", "frame_rate", t_frameRate);
    });
    switch (frame_rate) {
    case 5:
        fps5Action->triggered();
        fps5Action->setChecked(true);
        break;
    case 10:
        fps10Action->triggered();
        fps10Action->setChecked(true);
        break;
    case 20:
        fps20Action->triggered();
        fps20Action->setChecked(true);
        break;
    case 24:
        fps24Action->triggered();
        fps24Action->setChecked(true);
        break;
    case 30:
        fps30Action->triggered();
        fps30Action->setChecked(true);
        break;
    default:
        fps24Action->triggered();
        fps24Action->setChecked(true);
        break;
    }

    t_settings->setValue("recorder", "audio", 0);
//    t_settings->setValue("recorder", "cursor", 1);
    //notAudio->setChecked(true);
    //notMouse->setChecked(true);

    connect(t_audioGroup, QOverload<QAction *>::of(&QActionGroup::triggered), [ = ](QAction * t_act) {
        Q_UNUSED(t_act);
        int configValue = 0;
        if (sysAudio->isChecked() && m_microphoneAction->isChecked()) {
            configValue = 3;
        } else if (sysAudio->isChecked()) {
            configValue = 2;
        } else if (m_microphoneAction->isChecked()) {
            configValue = 1;
        }
        t_settings->setValue("recorder", "audio", configValue);
    });

    connect(t_mouseInfoGroup, QOverload<QAction *>::of(&QActionGroup::triggered), [ = ](QAction * t_act) {
        Q_UNUSED(t_act);
        int configValue = 0;
        if (showClick->isChecked() && showPointer->isChecked()) {
            configValue = 3;
        } else if (showClick->isChecked()) {
            configValue = 2;
        } else if (showPointer->isChecked()) {
            configValue = 1;
        }
        t_settings->setValue("recorder", "cursor", configValue);
        emit mouseBoardButtonClicked(showClick->isChecked());
    });
    int cursor = t_settings->getValue("recorder", "cursor").toInt();
    qDebug() << "默认是否录制鼠标？(0 不录制鼠标，及不录制鼠标点击,1 录制鼠标,2 录制鼠标点击,3 录制鼠标，及录制鼠标点击)" << cursor;
    if (cursor == 3) {
        showClick->setChecked(true);
        showPointer->setChecked(true);
    } else if (cursor == 2) {
        showClick->setChecked(true);
        showPointer->setChecked(false);
    } else if (cursor == 1) {
        showPointer->setChecked(true);
        showClick->setChecked(false);
    }else{
        showPointer->setChecked(false);
        showClick->setChecked(false);
    }

    int save_opt = t_settings->getValue("recorder", "save_op").toInt();
    qDebug() << "录屏默认保存到" << (save_opt ? "视频" : "桌面");
    if (save_opt == 0) {
        saveToVideoAction->setChecked(true);
        saveToVideoAction->trigger();
    } else {
        saveToDesktopAction->setChecked(true);
        saveToDesktopAction->trigger();
    }
    connect(t_saveGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == saveToDesktopAction) {
            qInfo() << "save to desktop";
            t_settings->setValue("recorder", "save_op", 1);
            t_settings->setValue("recorder", "save_dir", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
        } else {
            qInfo() << "save to video";
            t_settings->setValue("recorder", "save_op", 0);
            t_settings->setValue("recorder", "save_dir", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
        }
    });
    qDebug() << "录屏工具栏选项UI已初始化";
}

void SubToolWidget::initShotLabel()
{
    qDebug() << "正在初始化截图工具栏UI...";
    m_shotSubTool = new DLabel(this);
    m_shotBtnGroup = new QButtonGroup(this);
    m_shotBtnGroup->setExclusive(true);

    QList<ToolButton *> btnList;

    //添加矩形按钮
    m_rectButton = new ToolButton();
    m_rectButton->setIconSize(TOOL_ICON_SIZE);
    m_rectButton->setIcon(QIcon::fromTheme("rectangle-normal"));
    Utils::setAccessibility(m_rectButton, AC_SUBTOOLWIDGET_RECT_BUTTON);
    m_shotBtnGroup->addButton(m_rectButton);
    m_rectButton->setFixedSize(TOOL_BUTTON_SIZE);
    installTipHint(m_rectButton, tr("Rectangle (R)\nPress and hold Shift to draw a square"));
    btnList.append(m_rectButton);

    //添加椭圆按钮
    m_circleButton = new ToolButton();
    m_circleButton->setIconSize(TOOL_ICON_SIZE);
    m_circleButton->setIcon(QIcon::fromTheme("oval-normal"));
    installTipHint(m_circleButton, tr("Ellipse (O)\nPress and hold Shift to draw a circle"));
    Utils::setAccessibility(m_circleButton, AC_SUBTOOLWIDGET_CIRCL_BUTTON);
    m_shotBtnGroup->addButton(m_circleButton);
    m_circleButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_circleButton);

    //添加直线按钮
    m_lineButton = new ToolButton();
    m_lineButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_lineButton, tr("Line (L)\nPress and hold Shift to draw a vertical or horizontal line"));
    m_lineButton->setIcon(QIcon::fromTheme("line-normal"));
    Utils::setAccessibility(m_lineButton, AC_SUBTOOLWIDGET_LINE_BUTTON);
    m_shotBtnGroup->addButton(m_lineButton);
    m_lineButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_lineButton);

    //添加箭头按钮
    m_arrowButton = new ToolButton();
    m_arrowButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_arrowButton, tr("Arrow (X)\nPress and hold Shift to draw a vertical or horizontal arrow"));
    m_arrowButton->setIcon(QIcon::fromTheme("Arrow-normal"));
    Utils::setAccessibility(m_arrowButton, AC_SUBTOOLWIDGET_LINE_BUTTON);
    m_shotBtnGroup->addButton(m_arrowButton);
    m_arrowButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_arrowButton);

    //添加画笔按钮
    m_penButton = new ToolButton();
    m_penButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_penButton, tr("Pencil (P)"));
    m_penButton->setIcon(QIcon::fromTheme("Combined Shape-normal"));
    Utils::setAccessibility(m_penButton, AC_SUBTOOLWIDGET_PEN_BUTTON);
    m_shotBtnGroup->addButton(m_penButton);
    m_penButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_penButton);

    //添加模糊按钮
    m_mosaicButton = new ToolButton();
    m_mosaicButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_mosaicButton, tr("Blur (B)"));
    m_mosaicButton->setIcon(QIcon::fromTheme("Mosaic_normal"));
    Utils::setAccessibility(m_mosaicButton, AC_SUBTOOLWIDGET_MOSAIC_BUTTON);
    m_shotBtnGroup->addButton(m_mosaicButton);
    m_mosaicButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_mosaicButton);

    //添加文字按钮
    m_textButton = new ToolButton();
    m_textButton->setIconSize(TOOL_ICON_SIZE);
    m_textButton->setIcon(QIcon::fromTheme("text"));
    installTipHint(m_textButton, tr("Text (T)"));
    Utils::setAccessibility(m_textButton, AC_SUBTOOLWIDGET_TEXT_BUTTON);
    m_shotBtnGroup->addButton(m_textButton);
    m_textButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_textButton);

    //添加滚动截图按钮
    m_scrollShotButton = new ToolButton();
    m_scrollShotButton->setIconSize(TOOL_ICON_SIZE);
    m_scrollShotButton->setIcon(QIcon::fromTheme("scrollShot"));
    Utils::setAccessibility(m_scrollShotButton, AC_SUBTOOLWIDGET_SCROLLSHOT_BUTTON);
    m_shotBtnGroup->addButton(m_scrollShotButton);
    m_scrollShotButton->setFixedSize(TOOL_BUTTON_SIZE);
    installTipHint(m_scrollShotButton, tr("Scrollshot (Alt+I）"));
#ifdef  OCR_SCROLL_FLAGE_ON
    btnList.append(m_scrollShotButton);
#endif
    connect(m_scrollShotButton, &DPushButton::clicked, this, [ = ] {
        qDebug() << "滚动截图的按钮按下！";
        switchContent("scroll");
        emit changeShotToolFunc("scroll");
    });
    //添加ocr图文识别按钮
    m_ocrButton = new ToolButton();
    m_ocrButton->setIconSize(TOOL_ICON_SIZE);
    m_ocrButton->setIcon(QIcon::fromTheme("ocr-normal"));
    Utils::setAccessibility(m_ocrButton, AC_SUBTOOLWIDGET_OCR_BUTTON);
    m_shotBtnGroup->addButton(m_ocrButton);
    m_ocrButton->setFixedSize(TOOL_BUTTON_SIZE);
    installTipHint(m_ocrButton, tr("Extract text (Alt+O）"));
#ifdef  OCR_SCROLL_FLAGE_ON
    btnList.append(m_ocrButton);
#endif
    //添加贴图按钮
    m_pinButton = new ToolButton();
    m_pinButton->setIconSize(TOOL_ICON_SIZE);
    m_pinButton->setIcon(QIcon::fromTheme("pinscreenshots"));
    Utils::setAccessibility(m_pinButton, AC_SUBTOOLWIDGET_PINSCREENSHOTS_BUTTON);
    m_shotBtnGroup->addButton(m_pinButton);
    m_pinButton->setFixedSize(TOOL_BUTTON_SIZE);
    installTipHint(m_pinButton, tr("Pin screenshots (Alt+P）"));
    btnList.append(m_pinButton);

    // 撤销按钮
    m_cancelButton = new ToolButton();
    m_cancelButton->setUndoButtonFlag(true);
    m_cancelButton->setCheckable(false);
    m_cancelButton->setIconSize(TOOL_ICON_SIZE);
    m_cancelButton->setIcon(QIcon::fromTheme("cancel"));
    Utils::setAccessibility(m_cancelButton, AC_SUBTOOLWIDGET_UNDO_BUTTON);
    m_cancelButton->setFixedSize(TOOL_BUTTON_SIZE);
    installTipHint(m_cancelButton, tr("Undo (Ctrl+Z)"));
    btnList.append(m_cancelButton);
    m_cancelButton->setEnabled(false);
    connect(m_cancelButton, &ToolButton::clicked, m_pMainWindow, &MainWindow::unDo);
    connect(m_cancelButton, &ToolButton::isInUndoBtn, m_pMainWindow, &MainWindow::isInUndoBtn);
    connect(m_pMainWindow, &MainWindow::setUndoEnable, this, [ = ](bool isEnabled) {
        m_cancelButton->setEnabled(isEnabled);
    });

    //切换到录屏按钮
    m_recorderButton = new ToolButton();
    m_recorderButton->setCheckable(false);
    m_recorderButton->setIconSize(TOOL_ICON_SIZE);
    m_recorderButton->setIcon(QIcon::fromTheme("recorder"));
    Utils::setAccessibility(m_recorderButton, AC_SUBTOOLWIDGET_RECORDER_BUTTON);
    m_recorderButton->setFixedSize(TOOL_BUTTON_SIZE);
    installTipHint(m_recorderButton, tr("Record"));
    btnList.append(m_recorderButton);
    connect(m_recorderButton, &ToolButton::clicked, this, [ = ] {
        m_pMainWindow->getToolBarPoint();
        switchContent("record");
        emit changeShotToolFunc("record");
    });

    //截图选项按钮
    m_shotOptionButton = new ToolButton();
    DPalette pa = m_shotOptionButton->palette();
    m_shotOptionButton->setOptionButtonFlag(true);
    if (Utils::themeType == 1) {
        pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
        pa.setColor(DPalette::Dark, QColor(192, 192, 192, 255));
        pa.setColor(DPalette::Light, QColor(192, 192, 192, 255));
    } else {
        pa.setColor(DPalette::ButtonText, QColor(228, 228, 228, 255));
        pa.setColor(DPalette::Dark, QColor(64, 64, 64, 255));
        pa.setColor(DPalette::Light, QColor(64, 64, 64, 255));
    }
    m_shotOptionButton->setPalette(pa);
    m_shotOptionButton->setCheckable(false);
    m_shotOptionButton->setFlat(false);
    m_shotOptionButton->setHoverState(false);
    DFontSizeManager::instance()->bind(m_shotOptionButton, DFontSizeManager::T6);
    m_shotOptionButton->setText(tr("Settings"));
    Utils::setAccessibility(m_shotOptionButton, AC_SUBTOOLWIDGET_SHOT_OPTION_BUT);
    m_shotOptionButton->setMinimumSize(MEDIUM_TOOL_BUTTON_SIZE);
    installTipHint(m_shotOptionButton, tr("Settings (F3)"));
    m_shotBtnGroup->addButton(m_shotOptionButton);
    btnList.append(m_shotOptionButton);


    if (Utils::is3rdInterfaceStart) {
        m_shotOptionButton->hide();
        m_scrollShotButton->hide(); //隐藏滚动截图按钮
        m_ocrButton->hide(); //隐藏ocr按钮
        m_pinButton->hide(); //隐藏pin按钮
    }

    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setSizeConstraint(QLayout::SetFixedSize);
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(10);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        if (btnList[i] == m_recorderButton) {
            rectLayout->addSpacing(10);
        }
    }
    m_shotSubTool->setLayout(rectLayout);
    addWidget(m_shotSubTool);



    connect(m_shotBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        Q_UNUSED(status);
        if (m_pinButton->isChecked()) {
            emit changeShotToolFunc("pinScreenshots");
        }
        if (m_scrollShotButton->isChecked()) {
            emit changeShotToolFunc("scrollShot");
            //switchContent("scroll");
        }
        if (m_ocrButton->isChecked()) {
            emit changeShotToolFunc("ocr");
        }
        if (m_rectButton->isChecked()) {
            emit changeShotToolFunc("rectangle");
        }
        if (m_circleButton->isChecked()) {
            emit changeShotToolFunc("oval");
        }

        if (m_lineButton->isChecked()) {
            emit changeShotToolFunc("line");
        }

        if (m_arrowButton->isChecked()) {
            emit changeShotToolFunc("arrow");
        }

        if (m_penButton->isChecked()) {
            emit changeShotToolFunc("pen");
        }
        if (m_textButton->isChecked()) {
            emit changeShotToolFunc("text");
        }
        if (m_mosaicButton->isChecked()) {
            emit changeShotToolFunc("effect");
        }
    });

    initShotOption();
    qDebug() << "截图工具栏UI已初始化";
}

void SubToolWidget::initShotOption()
{
    qDebug() << "正在初始化截图工具栏选项UI...";
    QActionGroup *t_saveGroup = new QActionGroup(this);
    QActionGroup *t_formatGroup = new QActionGroup(this);
    t_saveGroup->setExclusive(true);
    t_formatGroup->setExclusive(true);

    m_optionMenu = new DMenu(this);
    m_optionMenu->installEventFilter(this);
    //系统级别为 T6 的字体大小, 默认是14 px
    DFontSizeManager::instance()->bind(m_optionMenu, DFontSizeManager::T6);

    // 保存位置
    QAction *saveTitleAction = new QAction(tr("Save to"), m_optionMenu);
    QAction *saveToClipAction = new QAction(tr("Clipboard"), m_optionMenu);
    QAction *saveToDesktopAction = new QAction(tr("Desktop"), m_optionMenu);
    QAction *saveToPictureAction = new QAction(tr("Pictures"), m_optionMenu);
    QAction *saveToSpecialPath = new QAction(tr("Folder"), m_optionMenu);
    m_saveToSpecialPathMenu = new DMenu(m_optionMenu);
    m_saveToSpecialPathMenu->installEventFilter(this);
    m_saveToSpecialPathMenu->setTitle(tr("Folder"));
    m_saveToSpecialPathMenu->setToolTipsVisible(true);
    m_saveToSpecialPathMenu->menuAction()->setCheckable(true);
    DFontSizeManager::instance()->bind(m_saveToSpecialPathMenu, DFontSizeManager::T8);
    QString specialPath = ConfigSettings::instance()->getValue("shot", "save_dir").value<QString>();
    //设置或更新指定路径的菜单按键
    m_changeSaveToSpecialPath = new QAction(m_saveToSpecialPathMenu);
    m_changeSaveToSpecialPath->setCheckable(true);
    //历史保存路径
    m_saveToSpecialPathAction = new QAction(m_saveToSpecialPathMenu);
    if (specialPath.isEmpty() || !QFileInfo::exists(specialPath)) {
        qDebug() << "不存在指定路径";
        m_changeSaveToSpecialPath->setText(tr("Set a path on save"));
    } else {
        qDebug() << "存在指定路径: " /*<< specialPath*/;
        m_changeSaveToSpecialPath->setText(tr("Change the path on save"));
        //根据字体大小计算字符串宽度，确定路径省略的长度
        QFontMetrics tempFont(m_changeSaveToSpecialPath->font());
        auto changeSaveToSpecialPathFontWidth = tempFont.boundingRect(m_changeSaveToSpecialPath->text()).width();
        QFontMetrics tmpFont(m_saveToSpecialPathAction->font());
        QString sFileName = tmpFont.elidedText(specialPath, Qt::TextElideMode::ElideRight, changeSaveToSpecialPathFontWidth);
        m_saveToSpecialPathAction->setText(sFileName);
        m_saveToSpecialPathAction->setToolTip(specialPath);
        m_saveToSpecialPathAction->setCheckable(true);
        m_saveToSpecialPathMenu->addAction(m_saveToSpecialPathAction);
        t_saveGroup->addAction(m_saveToSpecialPathAction);
    }
    m_saveToSpecialPathMenu->addAction(m_changeSaveToSpecialPath);

    // 保存格式
    QAction *formatTitleAction = new QAction(tr("Format"), m_optionMenu);
    QAction *pngAction = new QAction(tr("PNG"), m_optionMenu);
    QAction *jpgAction = new QAction(tr("JPG"), m_optionMenu);
    QAction *bmpAction = new QAction(tr("BMP"), m_optionMenu);

    // 显示鼠标光标
    QAction *m_clipTitleAction = new QAction(tr("Options"), m_optionMenu);
    QAction *m_saveCursorAction = new QAction(tr("Show pointer"), m_optionMenu);

    // 边框样式
    QAction *borderTitleAction = new QAction(tr("Border Effects"), m_optionMenu);
    QAction *noBorderAction = new QAction(tr("None"), m_optionMenu);
    ImageMenu *borderProjectionMenu = ImageBorderHelper::instance()->getBorderMenu(ImageBorderHelper::BorderType::Projection, tr("Shadow"), m_optionMenu);
    ImageMenu *externalBorderMenu = ImageBorderHelper::instance()->getBorderMenu(ImageBorderHelper::BorderType::External, tr("Border"), m_optionMenu);
    ImageMenu *borderPrototypeMenu = ImageBorderHelper::instance()->getBorderMenu(ImageBorderHelper::BorderType::Prototype, tr("Device"), m_optionMenu);

    Utils::setAccessibility(saveToDesktopAction, "saveToDesktopAction");
    Utils::setAccessibility(saveToPictureAction, "saveToPictureAction");
    Utils::setAccessibility(saveToSpecialPath, "saveToSpecialPath");
    Utils::setAccessibility(saveToClipAction, "saveToClipAction");
    Utils::setAccessibility(m_saveCursorAction, "saveCursorAction");
    Utils::setAccessibility(pngAction, "pngAction");
    Utils::setAccessibility(jpgAction, "jpgAction");
    Utils::setAccessibility(bmpAction, "bmpAction");

    saveTitleAction->setDisabled(true);
    saveToDesktopAction->setCheckable(true);
    saveToPictureAction->setCheckable(true);
    saveToSpecialPath->setCheckable(true);
    saveToClipAction->setCheckable(true);
    t_saveGroup->addAction(saveToDesktopAction);
    t_saveGroup->addAction(saveToPictureAction);
    t_saveGroup->addAction(saveToSpecialPath);
    t_saveGroup->addAction(m_changeSaveToSpecialPath);
    t_saveGroup->addAction(saveToClipAction);

    formatTitleAction->setDisabled(true);
    pngAction->setCheckable(true);
    jpgAction->setCheckable(true);
    bmpAction->setCheckable(true);
    t_formatGroup->addAction(pngAction);
    t_formatGroup->addAction(jpgAction);
    t_formatGroup->addAction(bmpAction);

    m_clipTitleAction->setDisabled(true);
    m_saveCursorAction->setCheckable(true);

    borderTitleAction->setDisabled(true);
    noBorderAction->setCheckable(true);
    noBorderAction->setChecked(true);

    //保存目录
    m_optionMenu->addAction(saveTitleAction);
    m_optionMenu->addAction(saveToClipAction);
    m_optionMenu->addAction(saveToDesktopAction);
    m_optionMenu->addAction(saveToPictureAction);
    //m_optionMenu->addAction(saveToSpecialPath);
    m_optionMenu->addMenu(m_saveToSpecialPathMenu);
    //m_optionMenu->addSeparator();

    //边框样式
    m_optionMenu->addAction(borderTitleAction);
    m_optionMenu->addAction(noBorderAction);
    m_optionMenu->addMenu(borderProjectionMenu);
    m_optionMenu->addMenu(externalBorderMenu);
    m_optionMenu->addMenu(borderPrototypeMenu);
    //m_optionMenu->addSeparator();

    //保存格式
    m_optionMenu->addAction(formatTitleAction);
    m_optionMenu->addAction(pngAction);
    m_optionMenu->addAction(jpgAction);
    m_optionMenu->addAction(bmpAction);
    //m_optionMenu->addSeparator();

    //保存光标
    m_optionMenu->addAction(m_clipTitleAction);
    m_optionMenu->addAction(m_saveCursorAction);
    m_optionMenu->hide();
    m_shotOptionButton->setMenu(m_optionMenu);

    // 根据配置，初始化Action状态
    SaveAction t_saveIndex = ConfigSettings::instance()->getValue("shot", "save_op").value<SaveAction>();

    switch (t_saveIndex) {
    case SaveToDesktop: {
        saveToDesktopAction->setChecked(true);
        break;
    }
    case SaveToImage: {
        saveToPictureAction->setChecked(true);
        break;
    }
    case SaveToSpecificDir: {
        //saveToSpecialPath->setChecked(true);
        m_saveToSpecialPathMenu->menuAction()->setChecked(true);
        bool isChangeSpecificDir = ConfigSettings::instance()->getValue("shot", "save_dir_change").value<bool>();
        if (specialPath.isEmpty() || isChangeSpecificDir || !QFileInfo::exists(specialPath)) {
            m_changeSaveToSpecialPath->setChecked(true);
            ConfigSettings::instance()->setValue("shot", "save_dir_change", true);
        } else {
            m_saveToSpecialPathAction->setChecked(true);
        }
        break;
    }
    default:
        saveToClipAction->setChecked(true);
        break;
    }

    connect(t_saveGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == saveToDesktopAction) {
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToDesktop);
            //ConfigSettings::instance()->setValue("shot", "save_dir", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
            m_saveToSpecialPathMenu->menuAction()->setChecked(false);
        } else if (t_act == saveToPictureAction) {
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToImage);
            //ConfigSettings::instance()->setValue("shot", "save_dir", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            m_saveToSpecialPathMenu->menuAction()->setChecked(false);
        } else if (t_act == saveToClipAction) {
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToClipboard);
            m_saveToSpecialPathMenu->menuAction()->setChecked(false);
        } else if (t_act == m_changeSaveToSpecialPath) {
            qDebug() << ">>>>>>>>>>> 设置或更改保存的指定位置";
            //此流程应是之前保存到指定目录的流程
            m_saveToSpecialPathMenu->menuAction()->setChecked(true);
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToSpecificDir);
            ConfigSettings::instance()->setValue("shot", "save_dir_change", true);
        } else {
            qDebug() << ">>>>>>>>>>> 保存指定位置";
            //此流程不是之前的流程，不会再打开文管
            m_saveToSpecialPathMenu->menuAction()->setChecked(true);
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToSpecificDir);
            ConfigSettings::instance()->setValue("shot", "save_dir_change", false);
        }
    });

    int t_pictureFormat = ConfigSettings::instance()->getValue("shot", "format").toInt();
    switch (t_pictureFormat) {
    case 1:
        jpgAction->setChecked(true);
        break;
    case 2:
        bmpAction->setChecked(true);
        break;
    default:
        pngAction->setChecked(true);
    }

    connect(t_formatGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == pngAction) {
            ConfigSettings::instance()->setValue("shot", "format", 0);
        } else if (t_act == jpgAction) {
            ConfigSettings::instance()->setValue("shot", "format", 1);
        } else if (t_act == bmpAction) {
            ConfigSettings::instance()->setValue("shot", "format", 2);
        }
    });

    int t_saveCursor = ConfigSettings::instance()->getValue("shot", "save_cursor").toInt();

    switch (t_saveCursor) {
    case 1:
        m_saveCursorAction->setChecked(true);
        break;
    default:
        m_saveCursorAction->setChecked(false);
        break;
    }

    int border_index = ConfigSettings::instance()->getValue("shot", "border_index").toInt();
    if (border_index != ImageBorderHelper::Nothing) {
        noBorderAction->setChecked(false);
        ImageBorderHelper::instance()->setBorderTypeDetail(border_index);
    }

    connect(m_saveCursorAction, &QAction::triggered, [ = ] {
        ConfigSettings::instance()->setValue("shot", "save_cursor", m_saveCursorAction->isChecked() ? 1 : 0);
    });

    connect(ImageBorderHelper::instance(), &ImageBorderHelper::updateBorderState, [ = ](bool hasBorderChecked) {
        noBorderAction->setChecked(!hasBorderChecked);
    });

    connect(noBorderAction, &QAction::triggered, [ = ] {
        qDebug() << __FUNCTION__ << __LINE__ << noBorderAction->isChecked();
        if (noBorderAction->isChecked())
        {
            ConfigSettings::instance()->setValue("shot", "border_index", 0);
            ImageBorderHelper::instance()->setActionState(ImageBorderHelper::BorderType::Nothing,  false);
        }
    });
    qDebug() << "截图工具栏选项UI已初始化";
    //ImageBorderHelper::instance()->setBorderTypeDetail(260);
}

void SubToolWidget::initScrollLabel()
{
    qDebug() << "正在初始化滚动截图工具栏UI...";
    m_scrollSubTool = new DLabel(this);
    QList<ToolButton *> btnList;

    //文字识别按钮
    m_ocrScrollButton = new ToolButton();
    Utils::setAccessibility(m_ocrScrollButton, AC_SUBTOOLWIDGET_KEYBOARD_BUTTON);
    m_ocrScrollButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_ocrScrollButton,  tr("Extract Text"));
    m_ocrScrollButton->setIcon(QIcon::fromTheme("ocr-normal"));
    m_ocrScrollButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(m_ocrScrollButton);
    connect(m_ocrScrollButton, &DPushButton::clicked, this, [ = ] {
        emit changeShotToolFunc("ocr");
    });

    //滚动截图选项
    m_scrollOptionButton = new ToolButton();
    m_scrollOptionButton->setFlat(false);
    DPalette pa = m_scrollOptionButton->palette();
    if (Utils::themeType == 1) {
        pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
        pa.setColor(DPalette::Dark, QColor(192, 192, 192, 255));
        pa.setColor(DPalette::Light, QColor(192, 192, 192, 255));
    } else {
        pa.setColor(DPalette::ButtonText, QColor(228, 228, 228, 255));
        pa.setColor(DPalette::Dark, QColor(64, 64, 64, 255));
        pa.setColor(DPalette::Light, QColor(64, 64, 64, 255));
    }
    m_scrollOptionButton->setPalette(pa);
    DFontSizeManager::instance()->bind(m_scrollOptionButton, DFontSizeManager::T6);
    Utils::setAccessibility(m_scrollOptionButton, AC_SUBTOOLWIDGET_RECORD_OPTION_BUT);
    m_scrollOptionButton->setHoverState(false);
    m_scrollOptionButton->setText(tr("Settings"));
    m_scrollOptionButton->setMinimumSize(MEDIUM_TOOL_BUTTON_SIZE);
    installTipHint(m_scrollOptionButton, tr("Settings"));
    btnList.append(m_scrollOptionButton);


    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setSizeConstraint(QLayout::SetFixedSize);
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(10);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
    }
    m_scrollSubTool->setLayout(rectLayout);
    addWidget(m_scrollSubTool);


    QActionGroup *t_saveGroup = new QActionGroup(this);
    QActionGroup *t_formatGroup = new QActionGroup(this);
    t_saveGroup->setExclusive(true);
    t_formatGroup->setExclusive(true);

    m_scrollOptionMenu = new DMenu(this);
    m_scrollOptionMenu->installEventFilter(this);
    DFontSizeManager::instance()->bind(m_scrollOptionMenu, DFontSizeManager::T6);

    // 保存位置
    QAction *saveTitleAction = new QAction(tr("Save to"), m_scrollOptionMenu);
    QAction *saveToClipAction = new QAction(tr("Clipboard"), m_scrollOptionMenu);
    QAction *saveToDesktopAction = new QAction(tr("Desktop"), m_scrollOptionMenu);
    QAction *saveToPictureAction = new QAction(tr("Pictures"), m_scrollOptionMenu);
    //QAction *saveToSpecialPath = new QAction(tr("Folder"), m_scrollOptionMenu);
    m_scrollSaveToSpecialPathMenu = new DMenu(m_scrollOptionMenu);
    m_scrollSaveToSpecialPathMenu->installEventFilter(this);
    m_scrollSaveToSpecialPathMenu->setTitle(tr("Folder"));
    m_scrollSaveToSpecialPathMenu->setToolTipsVisible(true);
    m_scrollSaveToSpecialPathMenu->menuAction()->setCheckable(true);
    DFontSizeManager::instance()->bind(m_scrollSaveToSpecialPathMenu, DFontSizeManager::T8);
    QString specialPath = ConfigSettings::instance()->getValue("shot", "save_dir").value<QString>();
    //设置或更新指定路径的菜单按键
    m_scrollChangeSaveToSpecialPath = new QAction(m_scrollSaveToSpecialPathMenu);
    m_scrollChangeSaveToSpecialPath->setCheckable(true);
    //历史保存路径
    m_scrollSaveToSpecialPathAction = new QAction(m_scrollSaveToSpecialPathMenu);
    if (specialPath.isEmpty() || !QFileInfo::exists(specialPath)) {
        qDebug() << "不存在指定路径";
        m_scrollChangeSaveToSpecialPath->setText(tr("Set a path on save"));
    } else {
        qDebug() << "存在指定路径: " /*<< specialPath*/;
        m_scrollChangeSaveToSpecialPath->setText(tr("Change the path on save"));
        //根据字体大小计算字符串宽度，确定路径省略的长度
        QFontMetrics tempFont(m_scrollChangeSaveToSpecialPath->font());
        auto changeSaveToSpecialPathFontWidth = tempFont.boundingRect(m_scrollChangeSaveToSpecialPath->text()).width();
        QFontMetrics tmpFont(m_scrollSaveToSpecialPathAction->font());
        QString sFileName = tmpFont.elidedText(specialPath, Qt::TextElideMode::ElideRight, changeSaveToSpecialPathFontWidth);
        m_scrollSaveToSpecialPathAction->setText(sFileName);
        m_scrollSaveToSpecialPathAction->setToolTip(specialPath);
        m_scrollSaveToSpecialPathAction->setCheckable(true);
        m_scrollSaveToSpecialPathMenu->addAction(m_scrollSaveToSpecialPathAction);
        t_saveGroup->addAction(m_scrollSaveToSpecialPathAction);
    }
    m_scrollSaveToSpecialPathMenu->addAction(m_scrollChangeSaveToSpecialPath);

    // 保存格式
    QAction *formatTitleAction = new QAction(tr("Format"), m_scrollOptionMenu);
    QAction *pngAction = new QAction(tr("PNG"), m_scrollOptionMenu);
    QAction *jpgAction = new QAction(tr("JPG"), m_scrollOptionMenu);
    QAction *bmpAction = new QAction(tr("BMP"), m_scrollOptionMenu);

    //保存目录
    m_scrollOptionMenu->addAction(saveTitleAction);
    m_scrollOptionMenu->addAction(saveToClipAction);
    m_scrollOptionMenu->addAction(saveToDesktopAction);
    m_scrollOptionMenu->addAction(saveToPictureAction);
    //m_scrollOptionMenu->addAction(saveToSpecialPath);
    m_scrollOptionMenu->addMenu(m_scrollSaveToSpecialPathMenu);
    m_scrollOptionMenu->addSeparator();

    //保存格式
    m_scrollOptionMenu->addAction(formatTitleAction);
    m_scrollOptionMenu->addAction(pngAction);
    m_scrollOptionMenu->addAction(jpgAction);
    m_scrollOptionMenu->addAction(bmpAction);

    saveTitleAction->setDisabled(true);
    saveToDesktopAction->setCheckable(true);
    saveToPictureAction->setCheckable(true);
    //saveToSpecialPath->setCheckable(true);
    saveToClipAction->setCheckable(true);
    t_saveGroup->addAction(saveToDesktopAction);
    t_saveGroup->addAction(saveToPictureAction);
    //t_saveGroup->addAction(saveToSpecialPath);
    t_saveGroup->addAction(m_scrollChangeSaveToSpecialPath);
    t_saveGroup->addAction(saveToClipAction);

    formatTitleAction->setDisabled(true);
    pngAction->setCheckable(true);
    jpgAction->setCheckable(true);
    bmpAction->setCheckable(true);
    t_formatGroup->addAction(pngAction);
    t_formatGroup->addAction(jpgAction);
    t_formatGroup->addAction(bmpAction);

    m_scrollOptionButton->setMenu(m_scrollOptionMenu);


    // 根据配置，初始化Action状态
    SaveAction t_saveIndex = ConfigSettings::instance()->getValue("shot", "save_op").value<SaveAction>();
    switch (t_saveIndex) {
    case SaveToDesktop: {
        saveToDesktopAction->setChecked(true);
        break;
    }
    case SaveToImage: {
        saveToPictureAction->setChecked(true);
        break;
    }
    case SaveToSpecificDir: {
        //saveToSpecialPath->setChecked(true);
        m_scrollSaveToSpecialPathMenu->menuAction()->setChecked(true);
        bool isChangeSpecificDir = ConfigSettings::instance()->getValue("shot", "save_dir_change").value<bool>();
        if (specialPath.isEmpty() || isChangeSpecificDir) {
            m_scrollChangeSaveToSpecialPath->setChecked(true);
            ConfigSettings::instance()->setValue("shot", "save_dir_change", true);
        } else {
            m_scrollSaveToSpecialPathAction->setChecked(true);
        }
        break;
    }
    default:
        saveToClipAction->setChecked(true);
        break;
    }

    connect(t_saveGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == saveToDesktopAction) {
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToDesktop);
            //ConfigSettings::instance()->setValue("shot", "save_dir", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
            m_scrollSaveToSpecialPathMenu->menuAction()->setChecked(false);
        } else if (t_act == saveToPictureAction) {
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToImage);
            //ConfigSettings::instance()->setValue("shot", "save_dir", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            m_scrollSaveToSpecialPathMenu->menuAction()->setChecked(false);
        }/* else if (t_act == saveToSpecialPath) {
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToSpecificDir);
        }*/ else if (t_act == saveToClipAction) {
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToClipboard);
            m_scrollSaveToSpecialPathMenu->menuAction()->setChecked(false);
        } else if (t_act == m_scrollChangeSaveToSpecialPath) {
            qDebug() << ">>>>>>>>>>> 设置或更改保存的指定位置";
            //此流程应是之前保存到指定目录的流程
            m_scrollSaveToSpecialPathMenu->menuAction()->setChecked(true);
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToSpecificDir);
            ConfigSettings::instance()->setValue("shot", "save_dir_change", true);
        } else {
            qDebug() << ">>>>>>>>>>> 保存指定位置";
            //此流程不是之前的流程，不会再打开文管
            m_scrollSaveToSpecialPathMenu->menuAction()->setChecked(true);
            ConfigSettings::instance()->setValue("shot", "save_op", SaveAction::SaveToSpecificDir);
            ConfigSettings::instance()->setValue("shot", "save_dir_change", false);
        }
    });

    int t_pictureFormat = ConfigSettings::instance()->getValue("shot", "format").toInt();
    switch (t_pictureFormat) {
    case 1:
        jpgAction->setChecked(true);
        break;
    case 2:
        bmpAction->setChecked(true);
        break;
    default:
        pngAction->setChecked(true);
        break;
    }

    connect(t_formatGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == pngAction) {
            ConfigSettings::instance()->setValue("shot", "format", 0);
        } else if (t_act == jpgAction) {
            ConfigSettings::instance()->setValue("shot", "format", 1);
        } else if (t_act == bmpAction) {
            ConfigSettings::instance()->setValue("shot", "format", 2);
        }
    });
    qDebug() << "滚动截图工具栏UI已初始化";
}

void SubToolWidget::installTipHint(QWidget *w, const QString &hintstr)
{
    // TODO: parent must be mainframe
    auto hintWidget = new ToolTips("", m_pMainWindow);
    hintWidget->hide();
    hintWidget->setText(hintstr);
//    hintWidget->setFixedHeight(32);
    installHint(w, hintWidget);
}

void SubToolWidget::installHint(QWidget *w, QWidget *hint)
{
    w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    w->installEventFilter(hintFilter);
}

void SubToolWidget::setScrollShotDisabled(const bool state)
{
    m_scrollShotButton->setDisabled(state);
}

void SubToolWidget::setPinScreenshotsEnable(const bool &state)
{
    m_pinButton->setEnabled(state);
}

void SubToolWidget::setOcrScreenshotEnable(const bool &state)
{
    m_ocrButton->setEnabled(state);
}

void SubToolWidget::setButEnableOnLockScreen(const bool &state)
{
    m_textButton->setEnabled(state);
    m_optionButton->setEnabled(state);
    m_shotOptionButton->setEnabled(state);
}

int SubToolWidget::getFuncSubToolX(QString &shape)
{
    int x = 0;
//    qDebug() << __FUNCTION__ << "m_rectButton->x() : " << m_rectButton->x();
//    qDebug() << __FUNCTION__ << "m_circleButton->x() : " << m_circleButton->x();
//    qDebug() << __FUNCTION__ << "m_lineButton->x() : " << m_lineButton->x();
//    qDebug() << __FUNCTION__ << "m_arrowButton->x() : " << m_arrowButton->x();
//    qDebug() << __FUNCTION__ << "m_penButton->x() : " << m_penButton->x();
    if (!shape.isEmpty()) {
        if (shape == "rectangle") {
            x = m_rectButton->x();
        } else if (shape == "oval") {
            x = m_circleButton->x();
        } else if (shape == "line") {
            x = m_lineButton->x();
        } else if (shape == "arrow") {
            x = m_arrowButton->x();
        } else if (shape == "pen") {
            x = m_penButton->x();
        } else if (shape == "text") {
            x = m_textButton->x();
        }  else  {
            x = -1;
        }
    }
//    qDebug() << __FUNCTION__ << "x : " << x;
    return x;
}

// 屏蔽DMenu，触发QAction Trigger时，收回菜单
bool SubToolWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_recordOptionMenu || watched == m_optionMenu || watched == m_scrollOptionMenu || watched == m_saveToSpecialPathMenu || watched == m_scrollSaveToSpecialPathMenu) {
        if (event->type() == QEvent::MouseButtonPress) {
            QAction *action = static_cast<DMenu *>(watched)->actionAt(static_cast<QMouseEvent *>(event)->pos());
            if (action) {
                if (nullptr != m_saveToSpecialPathMenu && action == m_saveToSpecialPathMenu->menuAction()) {
                    return QStackedWidget::eventFilter(watched, event);
                }
                if (nullptr != m_scrollSaveToSpecialPathMenu && action == m_saveToSpecialPathMenu->menuAction()) {
                    return QStackedWidget::eventFilter(watched, event);
                }
                action->activate(QAction::Trigger);
                return true;
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            QAction *action = static_cast<DMenu *>(watched)->actionAt(static_cast<QMouseEvent *>(event)->pos());
            if (action) {
                return true;
            }
        }
    }
    return false;
}

void SubToolWidget::switchContent(QString shapeType)
{
    qDebug() << __FUNCTION__ << __LINE__ << "切换截图或者录屏工具栏" << shapeType << QCursor().pos() << this->count();
    if (shapeType == "record") {
        this->addWidget(m_recordSubTool);
        this->removeWidget(m_shotSubTool);
        setCurrentWidget(m_recordSubTool);
        m_currentType = shapeType;
    } else if (shapeType == "shot") {
        this->addWidget(m_shotSubTool);
        this->removeWidget(m_recordSubTool);
        setCurrentWidget(m_shotSubTool);
        m_currentType = shapeType;
    } else if (shapeType == "scroll") {
        initScrollLabel();
        this->addWidget(m_scrollSubTool);
        this->removeWidget(m_recordSubTool);
        this->removeWidget(m_shotSubTool);
        setCurrentWidget(m_scrollSubTool);
        m_currentType = shapeType;
    }
    qDebug() << __FUNCTION__ << __LINE__ << "已切换工具栏" << shapeType << this->count();
}
void SubToolWidget::setRecordButtonDisable()
{
    m_recorderButton->setDisabled(true);
}

void SubToolWidget::setRecordLaunchMode(const unsigned int funType)
{
    qDebug() << __FUNCTION__ << __LINE__ << funType;
    if (funType == MainWindow::record) {
        //setCurrentWidget(m_recordSubTool);
        switchContent("record");
    } else if (funType == MainWindow::ocr) {
        m_ocrButton->click();
    } else if (funType == MainWindow::scrollshot) {
        m_scrollShotButton->click();
    }
}

void SubToolWidget::setVideoButtonInitFromSub()
{
    if (m_cameraButton->isChecked()) {
        m_cameraButton->click();
    }

    if (m_keyBoardButton->isChecked()) {
        m_keyBoardButton->click();
    }
}

void SubToolWidget::shapeClickedFromWidget(QString shape)
{
    qDebug() << "SubToolWidget::shapeClickedFromWidget " << shape;

    if (!shape.isEmpty()) {
        if (shape == "pinScreenshots") {
            if (!m_pinButton->isChecked())
                m_pinButton->click();
        } else if (shape == "scrollShot") {
            if (!m_scrollShotButton->isChecked())
                m_scrollShotButton->click();
        } else if (shape == "ocr") {
            if (!m_ocrButton->isChecked())
                m_ocrButton->click();
        } else if (shape == "rect") {
            if (!m_rectButton->isChecked())
                m_rectButton->click();
        } else if (shape == "circ") {
            if (!m_circleButton->isChecked())
                m_circleButton->click();
        } else if (shape == "line") {
            m_lineButton->click();
        } else if (shape == "arrow") {
            m_arrowButton->click();
        } else if (shape == "pen") {
            m_penButton->click();
        } else if (shape == "text") {
            m_textButton->click();
        } else if (shape == "option") {
            if (m_currentType == "shot" || currentWidget() == m_shotSubTool) {
                if (m_optionMenu->isHidden()) {
                    m_shotOptionButton->showMenu();
                } else {
                    m_optionMenu->hide();
                }
            } else if (m_currentType == "record" || currentWidget() == m_recordSubTool) {
                if (m_recordOptionMenu->isHidden()) {
                    m_optionButton->showMenu();
                } else {
                    m_recordOptionMenu->hide();
                }
            } else if (m_currentType == "scroll" || currentWidget() == m_scrollSubTool) {
                if (m_scrollOptionMenu->isHidden()) {
                    m_scrollOptionButton->showMenu();
                } else {
                    m_scrollOptionMenu->hide();
                }
            }

        } else if (shape == "keyBoard") {
            m_keyBoardButton->click();
        } else if (shape == "camera") {
            m_cameraButton->click();
        } else if (shape == "effect") {
            m_mosaicButton->click();
        } else  {
            qDebug() << __FUNCTION__ << __LINE__ << "ERROR" << shape;
        }
    }
}

void SubToolWidget::setMicroPhoneEnable(bool status)
{
    qDebug() << "mic 是否可选？" << status;
    m_microphoneAction->setEnabled(status);
    m_microphoneAction->setChecked(!status);
    //trigger()函数会改变当前的checked状态
    m_microphoneAction->trigger();
}

void SubToolWidget::setCameraDeviceEnable(bool status)
{
    if (status) {
        if (!m_cameraButton->isEnabled()) {
            m_cameraButton->setChecked(false);
            m_cameraButton->setEnabled(true);

            if (!m_cameraButton->isChecked()) {
                installTipHint(m_cameraButton, tr("Turn on camera (C)"));
            }
        }
    }

    else {
        if (m_cameraButton->isEnabled()) {
            m_cameraButton->setChecked(false);
            if (m_cameraButton->isChecked()) {
                installTipHint(m_cameraButton, tr("Turn off camera (C)"));
            } else {
                installTipHint(m_cameraButton, tr("Turn on camera (C)"));
            }
            m_cameraButton->setDisabled(true);

        }
    }
}

void SubToolWidget::setSystemAudioEnable(bool status)
{
    Q_UNUSED(status);
}

void SubToolWidget::onChangeAudioType(bool checked)
{
    Q_UNUSED(checked);
}

SubToolWidget::~SubToolWidget()
{
    if (nullptr != hintFilter) {
        delete hintFilter;
        hintFilter = nullptr;
    }
    if (nullptr != m_recordOptionMenu) {
        delete m_recordOptionMenu;
        m_recordOptionMenu = nullptr;
    }
    if (nullptr != m_shotBtnGroup) {
        delete m_shotBtnGroup;
        m_shotBtnGroup = nullptr;
    }
}

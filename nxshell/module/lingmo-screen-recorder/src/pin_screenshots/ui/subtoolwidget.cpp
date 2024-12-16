// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subtoolwidget.h"
#include "accessibility/acTextDefine.h"
#include "settings.h"

#include <QActionGroup>
#include <QFileInfo>
#include <DFontSizeManager>

#define THEMETYPE 1 // 主题颜色为浅色

const QSize MIN_TOOL_BUTTON_SIZE = QSize(42, 40);


SubToolWidget::SubToolWidget(DWidget *parent): DStackedWidget(parent)
{
    initShotLable();
    setCurrentWidget(m_shotSubTool);
}

// 初始化贴图工具栏按钮
void SubToolWidget::initShotLable()
{
    qInfo() << __LINE__ << __FUNCTION__ << "正在初始化贴图工具栏...";
    m_shotSubTool = new DLabel(this);
    // ocr按钮
    m_ocrButton = new ToolButton(this);
//    m_ocrButton->setCheckable(false);
//    m_ocrButton->setFlat(true);
//    m_ocrButton->setFocusPolicy(Qt::NoFocus);
    m_ocrButton->setObjectName(AC_SUBTOOLWIDGET_PIN_OCR_BUT);
    m_ocrButton->setAccessibleName(AC_SUBTOOLWIDGET_PIN_OCR_BUT);
    m_ocrButton->setIconSize(QSize(32, 32));
    m_ocrButton->setFixedSize(32, 32);
    m_ocrButton->setIcon(QIcon::fromTheme("ocr-normal"));
    m_ocrButton->setToolTip(tr("Extract Text"));
    connect(m_ocrButton, SIGNAL(clicked()), this, SIGNAL(signalOcrButtonClicked()));

    // 选项按钮
    m_pinOptionButton = new ToolButton(this);
    m_pinOptionButton->setCheckable(false);
    m_pinOptionButton->setFlat(false);
    m_pinOptionButton->setHoverState(false);
    DPalette pa = m_pinOptionButton->palette();
    DGuiApplicationHelper::ColorType t_type = DGuiApplicationHelper::instance()->themeType();
    if (t_type == DGuiApplicationHelper::ColorType::LightType) {
        pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
        pa.setColor(DPalette::Dark, QColor(192, 192, 192, 255));
        pa.setColor(DPalette::Light, QColor(192, 192, 192, 255));
    } else {
        pa.setColor(DPalette::ButtonText, QColor(228, 228, 228, 255));
        pa.setColor(DPalette::Dark, QColor(64, 64, 64, 255));
        pa.setColor(DPalette::Light, QColor(64, 64, 64, 255));
    }
    m_pinOptionButton->setPalette(pa);
    m_pinOptionButton->setFocusPolicy(Qt::NoFocus);
    m_pinOptionButton->setObjectName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    m_pinOptionButton->setAccessibleName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    DFontSizeManager::instance()->bind(m_pinOptionButton, DFontSizeManager::T6);
    m_pinOptionButton->setText(tr("Options"));
    m_pinOptionButton->setMinimumSize(QSize(60, 36));
    m_pinOptionButton->setToolTip(tr("Options"));
    m_saveGroup = new QActionGroup(this);
    QActionGroup *t_formatGroup = new QActionGroup(this);
    m_saveGroup->setExclusive(true);
    t_formatGroup->setExclusive(true);

    // 选项菜单
    m_optionMenu = new DMenu(this);
    DFontSizeManager::instance()->bind(m_optionMenu, DFontSizeManager::T6);
    connect(m_optionMenu, &DMenu::aboutToShow, this, &SubToolWidget::updateOptionChecked);

    QAction *saveTitleAction = new QAction(m_optionMenu);
    QAction *saveToClipAction = new QAction(m_optionMenu);
    QAction *saveToDesktopAction = new QAction(m_optionMenu);
    QAction *saveToPictureAction = new QAction(m_optionMenu);
//    QAction *saveToSpecialPath = new QAction(m_optionMenu);

    m_saveToSpecialPathMenu = new DMenu(m_optionMenu);
    m_saveToSpecialPathMenu->setTitle(tr("Folder"));
    m_saveToSpecialPathMenu->setToolTipsVisible(true);
    m_saveToSpecialPathMenu->menuAction()->setCheckable(true);
    DFontSizeManager::instance()->bind(m_saveToSpecialPathMenu, DFontSizeManager::T8);
    QString specialPath = Settings::instance()->getSavePath();
    //设置或更新指定路径的菜单按键
    m_changeSaveToSpecialPath = new QAction(m_saveToSpecialPathMenu);
    m_changeSaveToSpecialPath->setCheckable(true);
    m_saveToSpecialPathMenu->addAction(m_changeSaveToSpecialPath);
    //历史保存路径
    m_saveToSpecialPathAction = new QAction(m_saveToSpecialPathMenu);
    if (specialPath.isEmpty() || !QFileInfo::exists(specialPath)) {
        qDebug() << "不存在指定路径";
        m_changeSaveToSpecialPath->setText(tr("Set a path on save"));
    } else {
        qDebug() << "存在指定路径: " /*<< specialPath*/;
        initChangeSaveToSpecialAction(specialPath);
    }
    m_SavePathActions.insert(FOLDER_CHANGE, m_changeSaveToSpecialPath);

    QAction *formatTitleAction = new QAction(m_optionMenu);
    QAction *pngAction = new QAction(m_optionMenu);
    QAction *jpgAction = new QAction(m_optionMenu);
    QAction *bmpAction = new QAction(m_optionMenu);

    saveTitleAction->setDisabled(true);
    saveTitleAction->setText(tr("Save to"));

    saveToDesktopAction->setText(tr("Desktop"));
    saveToDesktopAction->setCheckable(true);

    saveToPictureAction->setText(tr("Pictures"));
    saveToPictureAction->setCheckable(true);

    //saveToSpecialPath->setText(tr("Folder"));
    //saveToSpecialPath->setCheckable(true);

    saveToClipAction->setText(tr("Clipboard"));
    saveToClipAction->setCheckable(true);

    m_saveGroup->addAction(saveToClipAction);
    m_saveGroup->addAction(saveToDesktopAction);
    m_saveGroup->addAction(saveToPictureAction);
    //m_saveGroup->addAction(saveToSpecialPath);
    m_saveGroup->addAction(m_changeSaveToSpecialPath);

    m_SavePathActions.insert(CLIPBOARD, saveToClipAction);
    m_SavePathActions.insert(DESKTOP, saveToDesktopAction);
    m_SavePathActions.insert(PICTURES, saveToPictureAction);

    formatTitleAction->setDisabled(true);
    formatTitleAction->setText(tr("Format"));
    pngAction->setText(tr("PNG"));
    pngAction->setCheckable(true);
    jpgAction->setText(tr("JPG"));
    jpgAction->setCheckable(true);
    bmpAction->setText(tr("BMP"));
    bmpAction->setCheckable(true);

    t_formatGroup->addAction(pngAction);
    t_formatGroup->addAction(jpgAction);
    t_formatGroup->addAction(bmpAction);

    m_SaveFormatActions.insert(PNG, pngAction);
    m_SaveFormatActions.insert(JPG, jpgAction);
    m_SaveFormatActions.insert(BMP, bmpAction);

    //保存方式
    m_optionMenu->addAction(saveTitleAction);
    m_optionMenu->addAction(saveToClipAction);
    m_optionMenu->addAction(saveToDesktopAction);
    m_optionMenu->addAction(saveToPictureAction);
    //m_optionMenu->addAction(saveToSpecialPath);
    m_optionMenu->addMenu(m_saveToSpecialPathMenu);
    m_optionMenu->addSeparator();
    // 格式
    m_optionMenu->addAction(formatTitleAction);
    m_optionMenu->addAction(pngAction);
    m_optionMenu->addAction(jpgAction);
    m_optionMenu->addAction(bmpAction);
    m_pinOptionButton->setMenu(m_optionMenu);

    //updateOptionChecked();

    connect(m_saveGroup, QOverload<QAction *>::of(&QActionGroup::triggered), [ = ](QAction * t_act) {
        Settings::instance()->setIsChangeSavePath(false);
        m_saveToSpecialPathMenu->menuAction()->setChecked(false);
        if (t_act == saveToDesktopAction) {
            qDebug() << "save to desktop";
            m_SaveInfo.first = DESKTOP;
        } else if (t_act == saveToPictureAction) {
            qDebug() << "save to picture";
            m_SaveInfo.first = PICTURES;
        } else if (t_act == saveToClipAction) {
            qDebug() << "save to clip";
            m_SaveInfo.first = CLIPBOARD;
        } else if (t_act == m_changeSaveToSpecialPath) {
            qDebug() << "设置或更改保存的指定位置";
            m_SaveInfo.first = FOLDER_CHANGE;
            m_saveToSpecialPathMenu->menuAction()->setChecked(true);
            Settings::instance()->setIsChangeSavePath(true);
        } else {
            qDebug() << "保存指定位置";
            m_SaveInfo.first = FOLDER;
            m_saveToSpecialPathMenu->menuAction()->setChecked(true);
        }
        Settings::instance()->setSaveOption(m_SaveInfo);
    });

    connect(t_formatGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == pngAction) {
            m_SaveInfo.second = PNG;
        } else if (t_act == jpgAction) {
            m_SaveInfo.second = JPG;
        } else if (t_act == bmpAction) {
            m_SaveInfo.second = BMP;
        }
        Settings::instance()->setSaveOption(m_SaveInfo);
    });

    //工具栏布局
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSizeConstraint(QLayout::SetFixedSize);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_ocrButton, 0,  Qt::AlignCenter);
    hLayout->addWidget(m_pinOptionButton, 0, Qt::AlignCenter);
    m_shotSubTool->setLayout(hLayout);
    addWidget(m_shotSubTool);
    updateOptionChecked();
    qInfo() << __LINE__ << __FUNCTION__ << "贴图工具栏初始化完成";
}

void SubToolWidget::initChangeSaveToSpecialAction(const QString specialPath)
{
    m_changeSaveToSpecialPath->setText(tr("Change the path on save"));
    //根据字体大小计算字符串宽度，确定路径省略的长度
    QFontMetrics tempFont(m_changeSaveToSpecialPath->font());
    auto changeSaveToSpecialPathFontWidth = tempFont.boundingRect(m_changeSaveToSpecialPath->text()).width();
    QFontMetrics tmpFont(m_saveToSpecialPathAction->font());
    QString sFileName = tmpFont.elidedText(specialPath, Qt::TextElideMode::ElideRight, changeSaveToSpecialPathFontWidth);
    m_saveToSpecialPathAction->setText(sFileName);
    m_saveToSpecialPathAction->setToolTip(specialPath);
    m_saveToSpecialPathAction->setCheckable(true);
    m_saveToSpecialPathMenu->insertAction(m_changeSaveToSpecialPath, m_saveToSpecialPathAction);
    m_saveGroup->addAction(m_saveToSpecialPathAction);
    m_SavePathActions.insert(FOLDER, m_saveToSpecialPathAction);
}

// 获取保存信息
QPair<int, int> SubToolWidget::getSaveInfo()
{
    return m_SaveInfo;
}

// 选项按钮被点击
void SubToolWidget::onOptionButtonClicked()
{
    if (m_optionMenu->isHidden())
        m_pinOptionButton->showMenu();
    else
        m_optionMenu->hide();
}

void SubToolWidget::updateOptionChecked()
{
    qInfo() << __LINE__ << __FUNCTION__ << "更新菜单选项";
    QPair<int, int> saveInfo = Settings::instance()->getSaveOption();

    if(saveInfo.second != PNG && saveInfo.second != JPG && saveInfo.second != BMP){
        saveInfo.second = PNG;
    }
    //没有配置文件时，给定一个默认值
    if (saveInfo.first == 0 || saveInfo.second == 0) {
        m_SaveInfo.first = CLIPBOARD; // 默认保存路径
        m_SaveInfo.second = PNG; // 默认保存格式
    } else {
        m_SaveInfo = saveInfo;
    }
    qDebug() << "m_SaveInfo: " << m_SaveInfo;
    //当前选中的是指定目录时需要重判断
    if (m_SaveInfo.first == FOLDER || m_SaveInfo.first == FOLDER_CHANGE) {
        m_saveToSpecialPathMenu->menuAction()->setChecked(true);
        QString specialPath = Settings::instance()->getSavePath();
        //判断是否存在历史路径
        if (specialPath.isEmpty() || !QFileInfo::exists(specialPath)) {
            m_SaveInfo.first = FOLDER_CHANGE;
        } else {
            m_SaveInfo.first = FOLDER;
            //配置文件中存在历史路径时(打开多个贴图，对其中某个贴图保存到指定路径)，但是当前的工具栏菜单中不存在对应的历史路径action
            if (!m_SavePathActions.contains(m_SaveInfo.first)) {
                initChangeSaveToSpecialAction(specialPath);
            } else {
                //根据字体大小计算字符串宽度，确定路径省略的长度
                QFontMetrics tempFont(m_changeSaveToSpecialPath->font());
                auto changeSaveToSpecialPathFontWidth = tempFont.boundingRect(m_changeSaveToSpecialPath->text()).width();
                QFontMetrics tmpFont(m_saveToSpecialPathAction->font());
                QString sFileName = tmpFont.elidedText(specialPath, Qt::TextElideMode::ElideRight, changeSaveToSpecialPathFontWidth);
                m_saveToSpecialPathAction->setText(sFileName);
                m_saveToSpecialPathAction->setToolTip(specialPath);
            }
        }
        if (Settings::instance()->getIsChangeSavePath()) {
            m_SaveInfo.first = FOLDER_CHANGE;
        }
    } else {
        m_saveToSpecialPathMenu->menuAction()->setChecked(false);
    }

    m_SavePathActions.value(m_SaveInfo.first)->setChecked(true);
    m_SaveFormatActions.value(m_SaveInfo.second)->setChecked(true);
}

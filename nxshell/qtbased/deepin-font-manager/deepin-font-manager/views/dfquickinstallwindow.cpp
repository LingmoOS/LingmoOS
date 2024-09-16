// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfquickinstallwindow.h"
#include "dfinstallnormalwindow.h"
#include "dfontinfomanager.h"
#include "fontmanagercore.h"
#include "dfmdbmanager.h"
#include "utils.h"
#include "globaldef.h"
#include "interfaces/dfontpreviewer.h"

#include <DWidgetUtil>
#include <DLog>
#include <DPushButton>
#include <DTitlebar>
#include <DApplicationHelper>

#include <QFontDatabase>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QDir>

DFQuickInstallWindow::DFQuickInstallWindow(const QStringList &files, QWidget *parent)
    : DMainWindow(parent)
    , m_fontInfoManager(DFontInfoManager::instance())
    , m_fontManager(FontManagerCore::instance())
    , m_dbManager(DFMDBManager::instance())
    , m_installFiles(files)
{
    initUI();
    initConnections();

    // Q_EMIT fileSelected(m_installFiles);
}

DFQuickInstallWindow::~DFQuickInstallWindow()
{
    qDebug() << __FUNCTION__ << "destructor";
}

/*************************************************************************
 <Function>      initUI
 <Description>   初始化ui界面
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFQuickInstallWindow::initUI()
{
    //hide maximize button, minimize button & menu button
    setTitlebarShadowEnabled(false);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
    titlebar()->setMenuVisible(false);

    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));

    //Set task bar icon
    setWindowIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));

    //Clear titlebar's background
    titlebar()->setBackgroundRole(QPalette::Background);
    titlebar()->setAutoFillBackground(false);

    titlebar()->setFixedHeight(44);
    titlebar()->setIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));

    m_titleLabel = new DLabel(this);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_titleLabel->setText(DApplication::translate("QuickInstallWindow", "Unknown"));

    titlebar()->addWidget(m_titleLabel, Qt::AlignBottom | Qt::AlignHCenter);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(0);

    // Style combox
    m_fontType = new DComboBox(this);
    m_fontType->setFixedSize(QSize(114, 36));

    // Text Preview
    m_fontPreviewTxt = new DFontPreviewer(this);
    // m_fontPreviewTxt->setFixedSize(QSize(381, 216));

    // m_fontPreviewTxt->setFixedHeight(216);
    m_fontPreviewTxt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QFont previewFont;
    previewFont.setPixelSize(28);
    m_fontPreviewTxt->setFont(previewFont);

    // Action bar
    QHBoxLayout *actionBarLayout = new QHBoxLayout();
    actionBarLayout->setContentsMargins(0, 0, 0, 0);
    actionBarLayout->setSpacing(0);

    //    QFont actionFont;
    //    actionFont.setPixelSize(14);

    m_stateLabel = new DLabel(this);
    m_stateLabel->setFixedHeight(m_stateLabel->fontMetrics().height());
    m_stateLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // m_stateLabel->setFont(actionFont);
    DPalette pa = DApplicationHelper::instance()->palette(m_stateLabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    m_stateLabel->setPalette(pa);
    m_oldPaStateLbl = DApplicationHelper::instance()->palette(m_stateLabel);

    m_actionBtn = new DPushButton(this);
    m_actionBtn->setFixedSize(QSize(120, 40));
    m_actionBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    // m_actionBtn->setFont(actionFont);
    m_actionBtn->setText(DApplication::translate("QuickInstallWindow", "Install Font"));

    // actionBarLayout->addSpacing(20);
    actionBarLayout->addWidget(m_stateLabel);
    actionBarLayout->addStretch();
    actionBarLayout->addWidget(m_actionBtn);
    //actionBarLayout->addSpacing(20);

    mainLayout->addSpacing(2);
    mainLayout->addWidget(m_fontType, 0, Qt::AlignCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_fontPreviewTxt);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(actionBarLayout);

    m_mainFrame = new QWidget(this);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainFrame->setLayout(mainLayout);

    setCentralWidget(m_mainFrame);

#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_mainFrame->setStyleSheet("background: red");
    m_fontPreviewTxt->setStyleSheet("background: blue");
    m_fontType->setStyleSheet("background: green");
    m_stateLabel->setStyleSheet("background: green");
    m_titleFrame->setStyleSheet("background: green");
    m_stateLabel->setStyleSheet("background: blue");
    m_titleLabel->setStyleSheet("background: yellow");
#endif
}

/*************************************************************************
 <Function>      initConnections
 <Description>   初始化链接
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFQuickInstallWindow::initConnections()
{
    connect(this, &DFQuickInstallWindow::fileSelected, this, &DFQuickInstallWindow::onFileSelected);
    connect(m_actionBtn, &DPushButton::clicked, this, &DFQuickInstallWindow::onInstallBtnClicked);
    connect(this, &DFQuickInstallWindow::quickInstall, this, [this]() {
        this->installFont(m_installFiles);
    });
}

/*************************************************************************
 <Function>      resizeEvent
 <Description>   尺寸变化事件
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFQuickInstallWindow::resizeEvent(QResizeEvent *event)
{
    DMainWindow::resizeEvent(event);

    // m_mainFrame->setFixedSize(event->size().width() - 150, event->size().height());
}


void DFQuickInstallWindow::onFileSelected(const QStringList &fileList)
{
    if (fileList.size() > 0) {
        QString file = fileList.at(0);

        //Clear old files
        m_installFiles.clear();
        m_installFiles.append(file);

        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(file);
        if (fontInfo.isError) {
            m_stateLabel->setText(DApplication::translate("QuickInstallWindow", "Broken file"));
            DPalette pa = DApplicationHelper::instance()->palette(m_stateLabel);
            pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextWarning));
            m_stateLabel->setPalette(pa);

            m_actionBtn->setDisabled(true);
            m_fontType->clear();
            m_fontType->addItem(DApplication::translate("QuickInstallWindow", "Unknown"));
        } else {
            if (fontInfo.isInstalled) {
                DPalette pa = DApplicationHelper::instance()->palette(m_stateLabel);
                pa.setBrush(DPalette::WindowText, QColor("#417505")/* pa.color(DPalette::TextWarning)*/);
                m_stateLabel->setPalette(pa);
                m_stateLabel->setText(DApplication::translate("QuickInstallWindow", "Installed"));

            } else {
                m_stateLabel->setPalette(m_oldPaStateLbl);
                m_stateLabel->setText(
                    DApplication::translate("QuickInstallWindow", "Not Installed"));
            }

            m_actionBtn->setDisabled(false);

            m_titleLabel->setText(fontInfo.familyName);

            m_fontType->clear();
            if (fontInfo.styleName.isEmpty()) {
                m_fontType->addItem(DApplication::translate("QuickInstallWindow", "Unknown"));
            } else {
                m_fontType->addItem(fontInfo.styleName);
            }

            if (!fontInfo.familyName.isEmpty()) {
                m_titleLabel->setText(fontInfo.familyName);
            }

            InitPreviewFont(fontInfo);
        }
    }
}

/*************************************************************************
 <Function>      onInstallBtnClicked
 <Description>   安装按钮按下后响应函数
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFQuickInstallWindow::onInstallBtnClicked()
{
    // Hide Quick install first
    hide();
    Q_EMIT quickInstall();
}

/*************************************************************************
 <Function>      InitPreviewFont
 <Description>   初始化预览字体
 <Author>        null
 <Input>
    <param1>     fontInfo            Description:字体信息
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFQuickInstallWindow::InitPreviewFont(DFontInfo &fontInfo)
{
    qDebug() << __FUNCTION__ << "enter";

    if (!fontInfo.isError) {
        //fontInfo = m_fontInfoManager->getFontInfo(fontInfo.filePath);
        if (!fontInfo.isInstalled) {
            int fontId = QFontDatabase::addApplicationFont(fontInfo.filePath);
            QStringList familys = QFontDatabase::applicationFontFamilies(fontId);

            if (familys.size() > 0) {
                fontInfo.familyName = familys.at(0);
            }
        }

        qDebug() << __FUNCTION__ << fontInfo.familyName;

        QFont preivewFont;
        preivewFont.setFamily(fontInfo.familyName);
        preivewFont.setPixelSize(28);
        QString styleName = fontInfo.styleName;

        if (styleName.contains("Italic")) {
            preivewFont.setItalic(true);
        }

        //之前代码顺序出现问题，导致有的时候contains判断出错 比如DemiBold与Bold，现在调整代码顺序
        if (styleName.contains("Regular", Qt::CaseInsensitive)) {
            preivewFont.setWeight(QFont::Normal);
        } else if (styleName.contains("DemiBold", Qt::CaseInsensitive)) {
            preivewFont.setWeight(QFont::DemiBold);
        } else if (styleName.contains("ExtraBold", Qt::CaseInsensitive)) {
            preivewFont.setWeight(QFont::ExtraBold);
        } else if (styleName.contains("Bold", Qt::CaseInsensitive)) {
            preivewFont.setWeight(QFont::Bold);
        } else if (styleName.contains("ExtraLight", Qt::CaseInsensitive)) {
            preivewFont.setWeight(QFont::ExtraLight);
        } else if (styleName.contains("Light", Qt::CaseInsensitive)) {
            preivewFont.setWeight(QFont::Light);
        } else if (styleName.contains("Thin", Qt::CaseInsensitive)) {
            preivewFont.setWeight(QFont::Thin);
        } else if (styleName.contains("Medium", Qt::CaseInsensitive)) {
            preivewFont.setWeight(QFont::Medium);
        } else if (styleName.contains("Black", Qt::CaseInsensitive)) {
            preivewFont.setWeight(QFont::Black);
        }

        m_fontPreviewTxt->setFont(preivewFont);
        m_fontPreviewTxt->setPreviewFontPath(fontInfo.filePath);

        m_titleLabel->setText(fontInfo.familyName);
    }
}

/*************************************************************************
 <Function>      installFont
 <Description>   安装字体
 <Author>        null
 <Input>
    <param1>     files            Description:需要安装的字体路径
 <Return>        null             Description:null
 <Note>          null
*************************************************************************/
void DFQuickInstallWindow::installFont(const QStringList &files)
{
    qDebug() << __FUNCTION__ << files;
    DFInstallNormalWindow dfNormalInstalldlg(files, nullptr);

    dfNormalInstalldlg.setSkipException(true);


    //Update db after install finish
    connect(SignalManager::instance(), &SignalManager::finishFontInstall, this,
            &DFQuickInstallWindow::onFontInstallFinished);

    Dtk::Widget::moveToCenter(&dfNormalInstalldlg);
    dfNormalInstalldlg.exec();
}

/*************************************************************************
 <Function>      onFontInstallFinished
 <Description>   字体安装结束后响应函数
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFQuickInstallWindow::onFontInstallFinished()
{
    DFontPreviewItemData itemData;
    QFileInfo filePathInfo(m_installFiles[0]);
    itemData.fontInfo = m_fontInfoManager->getFontInfo(m_installFiles[0]);

    qDebug() << __FUNCTION__ << "file:" << m_installFiles << " isInstalled:" << itemData.fontInfo.isInstalled;

    //Only insert record when db don't have record
    //avoid dummy record
    if (!itemData.fontInfo.isInstalled) {
        QStringList chineseFontPathList = m_fontInfoManager->getAllChineseFontPath();
        QStringList monoSpaceFontPathList = m_fontInfoManager->getAllMonoSpaceFontPath();

        m_dbManager->beginTransaction();

        const QString sysDir = QDir::homePath() + "/.local/share/fonts";
        QString dirName = itemData.fontInfo.familyName;
        QString target = QString("%1/%2/%3").arg(sysDir).arg(dirName).arg(filePathInfo.fileName());

        itemData.fontInfo.filePath = target;

        if (itemData.fontInfo.styleName.length() > 0) {
            itemData.fontData.strFontName =
                QString("%1-%2").arg(itemData.fontInfo.familyName).arg(itemData.fontInfo.styleName);
        } else {
            itemData.fontData.strFontName = itemData.fontInfo.familyName;
        }

        //itemData.strFontId = QString::number(m_dbManager->getRecordCount()+1);
//        itemData.strFontFileName = filePathInfo.baseName();
//        itemData.strFontPreview = QString(DApplication::translate("Font", "Don't let your dreams be dreams"));
//        itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
//        itemData.isPreviewEnabled = true;
        itemData.fontData.setEnabled(true);
        itemData.fontData.setCollected(false);
        itemData.fontData.setChinese(chineseFontPathList.contains(m_installFiles[0]));
        itemData.fontData.setMonoSpace(monoSpaceFontPathList.contains(m_installFiles[0]));

        itemData.fontInfo.isInstalled = true;

        m_dbManager->addFontInfo(itemData);


        m_dbManager->endTransaction();
    }

    QStringList list;
    list << m_installFiles[0];
    Q_EMIT requestShowMainWindow(list);
}

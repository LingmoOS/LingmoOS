// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepage.h"
#include "customwidget.h"
#include "uitools.h"

#include <DFileDialog>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DApplicationHelper>

#include <QVBoxLayout>
#include <QSettings>
#include <QSettings>
#include <QShortcut>
#include <QDragEnterEvent>
#include <QMimeData>

DGUI_USE_NAMESPACE

HomePage::HomePage(QWidget *parent)
    : DWidget(parent)
{
    initUI();           // 初始化界面配置
    initConnections();  // 初始化信号槽链接
}

HomePage::~HomePage()
{

}

void HomePage::initUI()
{
    // 初始化相关变量
    m_pIconLbl = new DLabel(this);
    m_pTipLbl = new DLabel(tr("Drag file or folder here"), this);
    m_pSplitLbl = new DLabel(this);
    m_pChooseBtn = new CustomCommandLinkButton(tr("Select File"), this);
    m_pSettings = new QSettings(QDir(UiTools::getConfigPath()).filePath("config.conf"), QSettings::IniFormat, this);

    // 初始化控件布局
    m_pLayout = new QVBoxLayout(this);
    m_pLayout->addStretch();
    m_pLayout->addWidget(m_pIconLbl, 0, Qt::AlignHCenter);
    m_pLayout->addSpacing(13);
    m_pLayout->addWidget(m_pTipLbl, 0, Qt::AlignHCenter);
    m_pLayout->addSpacing(15);
    m_pLayout->addWidget(m_pSplitLbl, 0, Qt::AlignHCenter);
    m_pLayout->addSpacing(15);
    m_pLayout->addWidget(m_pChooseBtn, 0, Qt::AlignHCenter);
    m_pLayout->addStretch();

    // 设置首页图标
    m_pixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_folder_128px.svg", QSize(128, 128));
    m_pIconLbl->setFixedSize(128, 128);
    m_pIconLbl->setPixmap(m_pixmap);

    // 设置提示和按钮字体
    DFontSizeManager::instance()->bind(m_pTipLbl, DFontSizeManager::T8);
    m_pTipLbl->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_pChooseBtn, DFontSizeManager::T8);

    // 初始化配置数据
    if (m_pSettings->value("dir").toString().isEmpty()) {
        m_pSettings->setValue("dir", "");
    }

    slotThemeChanged();     // 初始化分割线图标

    // 设置快捷键
    auto openkey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
    openkey->setContext(Qt::ApplicationShortcut);
    connect(openkey, &QShortcut::activated, this, &HomePage::signalFileChoose);

    setAcceptDrops(true);
    setBackgroundRole(DPalette::Base);
}

void HomePage::initConnections()
{
    connect(m_pChooseBtn, &DCommandLinkButton::clicked, this, &HomePage::signalFileChoose);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &HomePage::slotThemeChanged);
}

void HomePage::dragEnterEvent(QDragEnterEvent *e)
{
    const auto *mime = e->mimeData();

    // 判断是否有url
    if (!mime->hasUrls()) {
        e->ignore();
    }

    // 判断是否是本地设备文件，过滤 手机 网络 ftp smb 等
    for (const auto &url : mime->urls()) {
        if (!UiTools::isLocalDeviceFile(url.toLocalFile())) {
            e->ignore();
            return;
        }

    }

    e->accept();
}

void HomePage::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void HomePage::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();

    if (false == mime->hasUrls()) {
        e->ignore();
    }

    e->accept();

    // 判断本地文件
    QStringList fileList;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        fileList << url.toLocalFile();
    }

    if (fileList.size() == 0) {
        return;
    }

    emit signalDragFiles(fileList);
}

void HomePage::slotThemeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    if (DGuiApplicationHelper::LightType == themeType) {    // 浅色
        m_pSplitLbl->setPixmap(QPixmap(":assets/icons/deepin/builtin/light/icons/split_line.svg"));
    } else if (DGuiApplicationHelper::DarkType == themeType) {  // 深色
        m_pSplitLbl->setPixmap(QPixmap(":assets/icons/deepin/builtin/dark/icons/split_line_dark.svg"));
    } else {        // 其它默认
        m_pSplitLbl->setPixmap(QPixmap(":assets/icons/deepin/builtin/light/icons/split_line.svg"));
    }
}



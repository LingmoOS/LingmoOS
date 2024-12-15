// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewwidget.h"
#include "utils/utils.h"
#include "generalpreviewplugin.h"
#include "generalwidget/detailwidget.h"
#include "generalwidget/generaltoolbar.h"
#include "generalwidget/aitoolbar.h"
#include "pluginproxy.h"

#include <DScrollArea>
#include <DApplicationHelper>
#include <DFrame>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QScrollBar>
#include <QToolButton>
#include <QClipboard>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define CONTENT_WIDTH           372

PreviewWidget::PreviewWidget(QWidget *parent)
    : DWidget(parent)
    , m_proxy(new PluginProxy(this))
{
    m_generalPreview = QSharedPointer<PreviewPlugin>(new GeneralPreviewPlugin());
    m_generalPreview->init(m_proxy);

    initUi();
    initConnect();
}

PreviewWidget::~PreviewWidget()
{
    // 解除当前预览插件界面与预览主界面父子窗口关系，所有预览插件界面统一由插件管理类析构函数释放
    clearLayoutWidgets();
    if (m_vSpaceItem) {
        delete m_vSpaceItem;
        m_vSpaceItem = nullptr;
    }
}

bool PreviewWidget::previewItem(const MatchedItem &item)
{
    m_item = item;

    QSharedPointer<PreviewPlugin> preview = QSharedPointer<PreviewPlugin>(m_pluginManager.getPreviewPlugin(item));

    if (!preview)
        preview = m_generalPreview;
    else
        preview->init(m_proxy);

    // 插件界面根据新来搜索结果刷新预览内容
    ItemInfo itemInfo;
    itemInfo[PREVIEW_ITEMINFO_ITEM] = item.item;
    itemInfo[PREVIEW_ITEMINFO_NAME] = item.name;
    itemInfo[PREVIEW_ITEMINFO_ICON] = item.icon;
    itemInfo[PREVIEW_ITEMINFO_TYPE] = item.type;
    itemInfo[PREVIEW_ITEMINFO_SEARCHER] = item.searcher;
    preview->previewItem(itemInfo);

    // 插件有变更， 更换新插件界面内容到主界面布局中
    if (preview != m_preview) {

        // 清空主界面布局中原有预览插件界面内容
        clearLayoutWidgets();

        // 1. 更换新插件界面内容部件到主界面布局
        if (QWidget* contentWidget = preview->contentWidget()) {
            contentWidget->setFixedWidth(CONTENT_WIDTH);
            contentWidget->adjustSize();

            m_vMainLayout->addWidget(contentWidget);
            contentWidget->setVisible(true);
        }

        // 2. 添加详情属性部件到预览界面
        m_vMainLayout->addWidget(m_detailInfoWidget);

        // 文档类型的文件添加AI工具栏到预览界面
        m_vMainLayout->addWidget(m_aiToolBar);

        // 3. 添加垂直弹簧条，将工具栏部件置底到底部显示
        m_vMainLayout->addSpacerItem(m_vSpaceItem);

        if (preview->showToolBar()) {
            // 未定制工具栏，使用通用工具栏
            QWidget *toolBar = preview->toolBarWidget();
            if (!toolBar)
                toolBar = m_generalToolBar;
            Q_ASSERT(toolBar);
            m_vMainLayout->addWidget(toolBar);
        }

        m_preview = preview;
    }

    // 文档类型的文件添加AI工具栏到预览界面
    QString lowerName = item.name.toLower();
    bool isShowAiToolBar = lowerName.endsWith(".txt") || lowerName.endsWith(".doc") || lowerName.endsWith(".docx")  || lowerName.endsWith(".xls")
            || lowerName.endsWith(".xlsx") || lowerName.endsWith(".ppt") || lowerName.endsWith(".pptx") || lowerName.endsWith(".pdf");
    isShowAiToolBar = isShowAiToolBar && AiToolBar::checkUosAiInstalled();
    m_aiToolBar->setVisible(isShowAiToolBar);
    m_aiToolBar->setFilePath(item.item);

    m_detailInfoWidget->setDetailInfoList(preview->getAttributeDetailInfo());
    if (isShowAiToolBar) {
        if (preview->getAttributeDetailInfo().isEmpty()) {
            m_detailInfoWidget->setVisible(false);
            m_aiToolBar->setTopSpace(10);
            m_aiToolBar->adjustSize();
        } else {
            m_detailInfoWidget->setVisible(true);
            m_aiToolBar->setTopSpace(0);
            m_aiToolBar->adjustSize();
        }
    } else {
        m_detailInfoWidget->setVisible(true);
    }

    this->show();

    return true;
}

void PreviewWidget::initUi()
{
    m_vMainLayout = new QVBoxLayout(this);
    //不设置边距，由预览界面控制边距
    m_vMainLayout->setContentsMargins(0, 0, 0, 0);
    m_vMainLayout->setSpacing(0);

    m_detailInfoWidget = new DetailWidget(this);
    m_generalToolBar = new GeneralToolBar(this);
    m_aiToolBar = new AiToolBar(this);

    m_vSpaceItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    setFixedWidth(CONTENT_WIDTH);
}

void PreviewWidget::initConnect()
{
    connect(m_generalToolBar, &GeneralToolBar::sigOpenClicked, this, &PreviewWidget::onOpenClicked);
    connect(m_generalToolBar, &GeneralToolBar::sigOpenPathClicked, this, &PreviewWidget::onOpenpathClicked);
    connect(m_generalToolBar, &GeneralToolBar::sigCopyPathClicked, this, &PreviewWidget::onCopypathClicked);
}

void PreviewWidget::clearLayoutWidgets()
{
    if (m_preview) {
        m_preview->stopPreview();
        if (m_preview->contentWidget()) {
            m_preview->contentWidget()->setParent(nullptr);
            m_preview->contentWidget()->hide();
            m_vMainLayout->removeWidget(m_preview->contentWidget());
        }

        if (m_preview->toolBarWidget()) {
            m_preview->toolBarWidget()->setParent(nullptr);
            m_preview->toolBarWidget()->hide();
            m_vMainLayout->removeWidget(m_preview->toolBarWidget());
        }
    }

    m_vMainLayout->removeWidget(m_detailInfoWidget);
    m_vMainLayout->removeWidget(m_aiToolBar);
    m_vMainLayout->removeItem(m_vSpaceItem);

    if (m_generalPreview) {
        if (m_generalPreview->contentWidget()) {
            m_generalPreview->contentWidget()->setParent(nullptr);
            m_generalPreview->contentWidget()->hide();
            m_vMainLayout->removeWidget(m_generalPreview->contentWidget());
        }
        if (m_generalPreview->toolBarWidget()) {
            m_generalPreview->toolBarWidget()->setParent(nullptr);
            m_generalPreview->toolBarWidget()->hide();
            m_vMainLayout->removeWidget(m_generalPreview->toolBarWidget());
        }
    }
}

void PreviewWidget::onOpenClicked()
{
    Utils::openMatchedItem(m_item);
}

void PreviewWidget::onOpenpathClicked()
{
    Utils::openInFileManager(m_item);
}

void PreviewWidget::onCopypathClicked()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(m_item.item);
}

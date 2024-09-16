// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groupwidget_p.h"
#include "groupwidget.h"
#include "listview/grandsearchlistview.h"
#include "viewmore/viewmorebutton.h"
#include "utils/utils.h"
#include "global/accessibility/acintelfunctions.h"

#include <DLabel>
#include <DPushButton>
#include <DHorizontalLine>
#include <DApplicationHelper>
#include <DSpinner>

#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

#define ListItemHeight            36       // 列表行高
#define GroupLabelHeight          28       // 组标签高度
#define MoreBtnMaxHeight          25       // 查看更多按钮最大高度
#define LayoutMagrinSize          10       // 布局边距
#define SpacerWidth               40       // 弹簧宽度
#define SpacerHeight              20       // 弹簧高度

GroupWidgetPrivate::GroupWidgetPrivate(GroupWidget *parent)
    : q_p(parent)
{

}

GroupWidget::GroupWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new GroupWidgetPrivate(this))
{
    m_firstFiveItems.clear();
    m_restShowItems.clear();
    m_cacheItems.clear();
    m_cacheWeightItems.clear();

    initUi();
    initConnect();
}

GroupWidget::~GroupWidget()
{

}

void GroupWidget::setGroupName(const QString &groupName)
{
    Q_ASSERT(m_groupLabel);
    Q_ASSERT(m_listView);

    m_groupLabel->setText(groupName);

    AC_SET_ACCESSIBLE_NAME(m_listView, groupName);
}

void GroupWidget::appendMatchedItems(const MatchedItems &newItems, const QString& searchGroupName)
{
    Q_UNUSED(searchGroupName)

    if (newItems.isEmpty())
        return;

    // 结果列表未展开
    if (!m_bListExpanded) {
        // 当前组没有数据，则需要判断本次数据是否包含权重
        if (0 == m_listView->rowCount()) {
            const MatchedItem &item = newItems.first();
            if (!item.extra.isNull()
                    && item.extra.type() == QVariant::Hash
                    && item.extra.toHash().keys().contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT)) {
                // 已经排序过，直接显示
                m_cacheWeightItems << newItems;
                updateShowItems(m_cacheWeightItems);

                return;
            }
        }

        // 对未展开的数据排序
        m_cacheItems << newItems;
        Utils::sortByWeight(m_cacheItems);
        updateShowItems(m_cacheItems);
    } else {
        // 结果列表已展开，已经显示的数据保持不变，仅对新增数据排序，然后追加到列表末尾
        MatchedItems& tempNewItems = const_cast<MatchedItems&>(newItems);
        Utils::sortByWeight(tempNewItems);
        m_listView->addRows(tempNewItems);
    }
}

void GroupWidget::showHorLine(bool bShow)
{
    Q_ASSERT(m_line);

    m_line->setVisible(bShow);
}

bool GroupWidget::isHorLineVisilbe()
{
    Q_ASSERT(m_line);

    return  m_line->isVisible();
}

GrandSearchListView *GroupWidget::getListView()
{
    return m_listView;
}

ViewMoreButton *GroupWidget::getViewMoreButton()
{
    return m_viewMoreButton;
}

int GroupWidget::itemCount()
{
    Q_ASSERT(m_listView);

    return m_listView->rowCount();
}

int GroupWidget::getCurSelectHeight()
{
    Q_ASSERT(m_listView);

    int nHeight = 0;
    if (m_listView->currentIndex().isValid() || m_viewMoreButton->isSelected()) {
        nHeight += m_groupLabel->height();
        nHeight += (m_listView->currentIndex().row() + 1) * ListItemHeight;
    }

    return nHeight;
}

void GroupWidget::reLayout()
{
    Q_ASSERT(m_listView);
    Q_ASSERT(m_groupLabel);
    Q_ASSERT(m_line);

    m_listView->setFixedHeight(m_listView->rowCount() * ListItemHeight);

    int nHeight = 0;
    nHeight += m_groupLabel->height();

    if (m_listView->isVisible())
        nHeight += m_listView->height();

    if (m_resultLabel->isVisible())
        nHeight += m_resultLabel->height();

    if (!m_line->isHidden()) {
        nHeight += m_line->height();
        nHeight += LayoutMagrinSize;
        m_vContentLayout->setSpacing(10);
    } else {
        m_vContentLayout->setSpacing(0);
    }

    this->setFixedHeight(nHeight);
}

void GroupWidget::clear()
{
    Q_ASSERT(m_listView);

    m_firstFiveItems.clear();
    m_restShowItems.clear();
    m_cacheWeightItems.clear();
    m_cacheItems.clear();

    m_bListExpanded = false;

    m_listView->clear();
    m_viewMoreButton->hide();

    showLabel(false);
    setVisible(false);
}

void GroupWidget::setSearchGroupName(const QString &searchGroupName)
{
    m_searchGroupName = searchGroupName;
}

QString GroupWidget::searchGroupName() const
{
    return m_searchGroupName;
}

QString GroupWidget::groupName() const
{
    Q_ASSERT(m_groupLabel);

    return m_groupLabel->text();
}

void GroupWidget::setIcon(const QIcon &icon)
{
    if (icon.isNull()) {
       m_groupIcon->hide();
    } else {
       m_groupIcon->setPixmap(icon.pixmap(QSize(18, 18)));
       m_groupIcon->show();
    }
}

void GroupWidget::showSpinner(bool bShow)
{
    if (bShow) {
        m_spinner->parentWidget()->show();
        m_spinner->start();
    } else {
        m_spinner->parentWidget()->hide();
        m_spinner->stop();
    }
}

void GroupWidget::showLabel(bool bShow)
{
    m_listView->setVisible(!bShow);
    m_resultLabel->setVisible(bShow);
}

QString GroupWidget::convertDisplayName(const QString &searchGroupName)
{
    static const QHash<QString, QString> groupDisplayName{
        {GRANDSEARCH_GROUP_BEST, GroupName_Best}
        , {GRANDSEARCH_GROUP_APP, GroupName_App}
        , {GRANDSEARCH_GROUP_SETTING, GroupName_Setting}
        , {GRANDSEARCH_GROUP_FILE_VIDEO, GroupName_Video}
        , {GRANDSEARCH_GROUP_FILE_AUDIO, GroupName_Audio}
        , {GRANDSEARCH_GROUP_FILE_PICTURE, GroupName_Picture}
        , {GRANDSEARCH_GROUP_FILE_DOCUMNET, GroupName_Document}
        , {GRANDSEARCH_GROUP_FOLDER, GroupName_Folder}
        , {GRANDSEARCH_GROUP_FILE, GroupName_File}
        , {GRANDSEARCH_GROUP_WEB, GroupName_Web}
        , {GRANDSEARCH_GROUP_FILE_INFERENCE, GroupName_Inference}
    };

    return groupDisplayName.value(searchGroupName, searchGroupName);
}

void GroupWidget::initUi()
{
    // 获取设置当前窗口文本颜色
    QColor groupTextColor = QColor(0, 0, 0, static_cast<int>(255*0.6));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        groupTextColor = QColor(255, 255, 255, static_cast<int>(255*0.6));
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, groupTextColor);

    // 组列表内控件沿垂直方向布局
    m_vLayout = new QVBoxLayout(this);
    m_vLayout->setContentsMargins(0, 0, LayoutMagrinSize, 0);
    m_vLayout->setSpacing(0);
    this->setLayout(m_vLayout);

    // 组名标签
    m_groupLabel = new DLabel("", this);
    m_groupLabel->setFixedHeight(GroupLabelHeight);
    m_groupLabel->setPalette(labelPalette);
    m_groupLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_groupLabel->setContentsMargins(0, 0, 0, 0);

    // 组名图标
    m_groupIcon = new DLabel("", this);
    m_groupIcon->setFixedSize(QSize(23, GroupLabelHeight)); // 18px of icon and 5px space
    m_groupIcon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_groupLabel->setContentsMargins(0, 5, 5, 5);
    m_groupIcon->hide();

    QFont groupLabelFont = m_groupLabel->font();
    groupLabelFont.setWeight(QFont::Normal);
    groupLabelFont = DFontSizeManager::instance()->get(DFontSizeManager::T8, groupLabelFont);
    m_groupLabel->setFont(groupLabelFont);

    // 查看更多按钮
    m_viewMoreButton = new ViewMoreButton(tr("More"), this);
    m_viewMoreButton->setMaximumHeight(MoreBtnMaxHeight);

    DFontSizeManager::instance()->bind(m_viewMoreButton, DFontSizeManager::T8, QFont::Normal);
    QFont fontMoreBtn = m_viewMoreButton->font();
    fontMoreBtn.setWeight(QFont::Normal);
    fontMoreBtn = DFontSizeManager::instance()->get(DFontSizeManager::T8, fontMoreBtn);
    m_viewMoreButton->setFont(fontMoreBtn);
    m_viewMoreButton->hide();

    QWidget *spinnerContainer = new QWidget(this);
    {
        spinnerContainer->setFixedSize(38, 18); //20px space on right.
        m_spinner = new DSpinner(spinnerContainer);
        m_spinner->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_spinner->setFocusPolicy(Qt::NoFocus);
        m_spinner->setFixedSize(QSize(18, 18));
        m_spinner->setGeometry(0, 0, 18, 18);
        spinnerContainer->hide();
    }

    // 组列表标题栏布局
    m_hTitelLayout = new QHBoxLayout();
    m_hTitelLayout->setContentsMargins(LayoutMagrinSize, 0, 0, 0);
    m_hTitelLayout->setSpacing(0);
    m_hTitelLayout->addWidget(m_groupIcon);
    m_hTitelLayout->addWidget(m_groupLabel);
    m_hTitelLayout->addSpacerItem(new QSpacerItem(SpacerWidth,SpacerHeight,QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_hTitelLayout->addWidget(m_viewMoreButton);
    m_hTitelLayout->addWidget(spinnerContainer);

    // 组内结果列表
    m_listView = new GrandSearchListView(this);
    m_listView->setFocusPolicy(Qt::NoFocus);

    // 无结果提示
    m_resultLabel = new DLabel(tr("No search results"), this);
    DFontSizeManager::instance()->bind(m_resultLabel, DFontSizeManager::T8, QFont::Normal);
    // color
    {
        QColor colorText = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType ?
                    Qt::white : Qt::black;
        colorText.setAlphaF(0.5);
        auto pal = m_resultLabel->palette();
        pal.setColor(m_resultLabel->foregroundRole(), colorText);
        m_resultLabel->setPalette(pal);
    }
    m_resultLabel->setFixedHeight(14 + DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T8));
    m_resultLabel->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    m_resultLabel->setContentsMargins(LayoutMagrinSize, 10, 0, 0);
    m_resultLabel->hide();

    // 分割线
    m_line = new DHorizontalLine;

    // 列表和分割线放到内容布局内
    m_vContentLayout = new QVBoxLayout();
    m_vContentLayout->setMargin(0);
    m_vContentLayout->setSpacing(0);
    m_vContentLayout->addWidget(m_listView);
    m_vContentLayout->addWidget(m_resultLabel);
    m_vContentLayout->addWidget(m_line);

    // 标题栏布局和内容布局放到主布局内
    m_vLayout->addLayout(m_hTitelLayout);
    m_vLayout->addLayout(m_vContentLayout);
}

void GroupWidget::initConnect()
{
    Q_ASSERT(m_viewMoreButton);

    connect(m_viewMoreButton, &DPushButton::clicked, this, &GroupWidget::onMoreBtnClicked);
}

void GroupWidget::updateShowItems(MatchedItems &items)
{
    // 显示不足5个，补足显示
    if (GROUP_MAX_SHOW != m_listView->rowCount()) {
        for (int i = m_firstFiveItems.count(); i < GROUP_MAX_SHOW; i++) {
            if (!items.isEmpty())
                m_firstFiveItems.push_back(items.takeFirst());
        }
        m_listView->setMatchedItems(m_firstFiveItems);
    }

    // 缓存中有数据，显示'查看更多'按钮
    m_viewMoreButton->setVisible(!items.isEmpty());
}

void GroupWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);
}

void GroupWidget::onMoreBtnClicked()
{
    Q_ASSERT(m_listView);
    Q_ASSERT(m_viewMoreButton);

    // 将缓存中的数据转移到剩余显示结果中
    m_restShowItems << m_cacheWeightItems;
    m_restShowItems << m_cacheItems;

    // 剩余显示结果追加显示到列表中
    m_listView->addRows(m_restShowItems);

    // 清空缓存中数据
    m_cacheWeightItems.clear();
    m_cacheItems.clear();

    reLayout();

    emit showMore();
    m_viewMoreButton->hide();

    // '查看更多'按钮处于被选中状态时，回车、鼠标点击后，选中列表第一项
    if (m_viewMoreButton->isSelected()) {
        m_viewMoreButton->setSelected(false);
        const QModelIndex &index = m_listView->model()->index(0, 0);
        if (Q_LIKELY(index.isValid())) {
            m_listView->setCurrentIndex(index);
            MatchedItem item = m_listView->currentIndex().data(DATA_ROLE).value<MatchedItem>();
            emit sigCurrentItemChanged(m_searchGroupName, item);
        }
    }

    m_bListExpanded = true;
}



// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "matchwidget_p.h"
#include "matchwidget.h"
#include "listview/grandsearchlistview.h"
#include "viewmore/viewmorebutton.h"
#include "utils/utils.h"
#include "gui/datadefine.h"

#include <DScrollArea>
#include <DApplicationHelper>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QScrollBar>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define ListItemHeight 36
#define ScrollBarToListRightSpace 2

MatchWidgetPrivate::MatchWidgetPrivate(MatchWidget *parent)
    : q_p(parent)
{

}

void MatchWidgetPrivate::setGroupIcon(GroupWidget *wid)
{
    if (!wid)
        return;

    if (wid->searchGroupName() == GRANDSEARCH_GROUP_FILE_INFERENCE) {
        wid->setIcon(QIcon(":/icons/aisearch.svg"));
    }
}

MatchWidget::MatchWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new MatchWidgetPrivate(this))
{
    m_groupHashShowOrder << GRANDSEARCH_GROUP_BEST << GRANDSEARCH_GROUP_FILE_INFERENCE
                         << GRANDSEARCH_GROUP_APP << GRANDSEARCH_GROUP_SETTING
                         << GRANDSEARCH_GROUP_WEB << GRANDSEARCH_GROUP_FILE_VIDEO
                         << GRANDSEARCH_GROUP_FILE_AUDIO << GRANDSEARCH_GROUP_FILE_PICTURE
                         << GRANDSEARCH_GROUP_FILE_DOCUMNET << GRANDSEARCH_GROUP_FOLDER << GRANDSEARCH_GROUP_FILE;
    initUi();
    initConnect();

    reLayout();
}

MatchWidget::~MatchWidget()
{

}

void MatchWidget::appendMatchedData(const MatchedItemMap &matchedData)
{
    bool bNeedRelayout = false;

    // 数据处理
    MatchedItemMap::ConstIterator itData = matchedData.begin();
    while (itData != matchedData.end()) {
        // 根据groupHash创建对应类目列表，若已存在，直接返回已有类目列表
        GroupWidget* groupWidget = createGroupWidget(itData.key());
        if (!groupWidget) {
            itData++;
            continue;
        }

        // ai搜索的动态名称
        if (groupWidget->searchGroupName() == GRANDSEARCH_GROUP_FILE_INFERENCE && groupWidget->itemCount() == 0) {
            if (itData.value().isEmpty())
                groupWidget->setGroupName(GroupWidget::convertDisplayName(groupWidget->searchGroupName()));
            else
                groupWidget->setGroupName(tr("Guess you want to search the following"));
        }

        // 追加匹配数据到类目列表中
        groupWidget->appendMatchedItems(itData.value(), itData.key());

        // 列表中没有数据，显示等待效果
        groupWidget->setVisible(true);
        groupWidget->showSpinner(groupWidget->itemCount() < 1);

        // 有新增匹配结果，需要调整重新布局
        if (!bNeedRelayout && itData.value().size() > 0)
            bNeedRelayout = true;

        itData++;
    }

    // 对需要显示的类目列表排序
    sortVislibleGroupList();

    // 重新调整布局
    if (bNeedRelayout)
        reLayout();

    // 用户未手动切换选中项时，确保选中当前结果中的第一个
    if (!m_customSelected) {
        clearSelectItem();
        selectFirstItem();
    }
}

void MatchWidget::clearMatchedData()
{
    Q_ASSERT(m_scrollAreaContent);

    m_vGroupWidgets.clear();

    // 清空并隐藏所有类目列表
    for (GroupWidgetMap::Iterator it = m_groupWidgetMap.begin(); it != m_groupWidgetMap.end(); ++it) {
        if (it.value())
            it.value()->clear();
    }

    // 重置滚动区域内容部件高度
    if (Q_LIKELY(m_scrollAreaContent))
        m_scrollAreaContent->setFixedHeight(0);

    layout();

    m_customSelected = false;
}

void MatchWidget::onSearchCompleted()
{
    // 搜索结束关闭等待动画
    bool need = false;
    for (GroupWidget *wid : m_groupWidgetMap.values()) {
        if (wid) {
            wid->showSpinner(false);
            if (wid->itemCount() == 0) {
                wid->showLabel(true);
                need = true;
            }
        }
    }

    if (need)
        reLayout();

    if (!m_vGroupWidgets.isEmpty())
        return;

    emit sigShowNoContent(true);
}

void MatchWidget::onShowMore()
{
    reLayout();
}

void MatchWidget::selectNextItem()
{
    if (this->isHidden())
        return;

    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {

        if (!hasSelectItem(i))
            continue;

        // 选择项在当前列表中
        GroupWidget *group = m_vGroupWidgets.at(i);
        GrandSearchListView *listView = group->getListView();
        ViewMoreButton *viewMoreBtn = group->getViewMoreButton();

        int nextRow = -1;
        if (viewMoreBtn->isSelected()) {
            nextRow = 0;
            viewMoreBtn->setSelected(false);
        } else {
            const QModelIndex &index = listView->currentIndex();
            nextRow = index.row() + 1;
        }

        const QModelIndex &nextIndex = listView->model()->index(nextRow, 0);
        if (nextIndex.isValid()) {
            listView->setCurrentIndex(nextIndex);
            m_customSelected = true;
            currentIndexChanged(group->searchGroupName(), nextIndex);
            break;
        } else {
            // 选择项是当前列表的最后一项，需要选择下一个列表的第一项
            if (i < m_vGroupWidgets.count() - 1) {
                bool selected = selectFirstItem(++i);
                if (Q_LIKELY(selected)) {
                    // 切换成功后，需要将当前列表选择为空
                    listView->setCurrentIndex(QModelIndex());
                    m_customSelected = true;
                } else {
                    qDebug() << "select next failed";
                }
                break;
            } else {
                qInfo() << "it's the last one";
            }
        }
    }

    adjustScrollBar();
}

void MatchWidget::selectPreviousItem()
{
    if (this->isHidden())
        return;

    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {

        if (!hasSelectItem(i))
            continue;

        // 选择项在当前列表中
        GroupWidget *group = m_vGroupWidgets.at(i);
        GrandSearchListView *listView = group->getListView();
        ViewMoreButton *viewMoreBtn = group->getViewMoreButton();

        const QModelIndex &index = listView->currentIndex();
        int previousRow = index.row() - 1;
        const QModelIndex &previousIndex = listView->model()->index(previousRow, 0);
        if (previousIndex.isValid()) {
            listView->setCurrentIndex(previousIndex);
            m_customSelected = true;
            currentIndexChanged(group->searchGroupName(), previousIndex);
            break;
        } else if (viewMoreBtn->isVisible() && !viewMoreBtn->isSelected()) {
            viewMoreBtn->setSelected(true);
            listView->setCurrentIndex(QModelIndex());
            currentIndexChanged(group->searchGroupName(), QModelIndex());
            m_customSelected = true;
            break;
        } else {
            // 选择项是当前列表的第一项，需要选择上一个列表的最后一项
            if (i > 0) {
                bool selected = selectLastItem(--i);
                if (Q_LIKELY(selected)) {
                    // 切换成功后，需要将当前列表选择为空
                    listView->setCurrentIndex(QModelIndex());
                    viewMoreBtn->setSelected(false);
                    m_customSelected = true;
                } else {
                    qWarning() << "select previous failed";
                }
                break;
            } else {
                qInfo() << "it's the first one";
            }
        }
    }

    adjustScrollBar();
}

void MatchWidget::handleItem()
{
    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {
        if (hasSelectItem(i)) {
            ViewMoreButton *viewMoreBtn = m_vGroupWidgets.at(i)->getViewMoreButton();
            Q_ASSERT(viewMoreBtn);
            GrandSearchListView *listView = m_vGroupWidgets.at(i)->getListView();
            Q_ASSERT(listView);

            if (viewMoreBtn->isSelected()) {
                viewMoreBtn->click();
                break;
            }

            MatchedItem item = listView->currentIndex().data(DATA_ROLE).value<MatchedItem>();
            Utils::openMatchedItem(item);
            emit sigCloseWindow();
            break;
        }
    }
}

void MatchWidget::onPreviewStateChanged(const bool preview)
{
    if (this->isHidden() || preview == m_isPreviewItem)
        return ;

    m_isPreviewItem = preview;

    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {

        GroupWidget *group = m_vGroupWidgets.at(i);
        Q_ASSERT(group);

        GrandSearchListView *listView = group->getListView();
        Q_ASSERT(listView);

        listView->updatePreviewItemState(preview);
    }
}

void MatchWidget::onSelectItemByMouse(const MatchedItem &item)
{
    QString searchGroupName;

    // 通知其他列表取消选中
    GrandSearchListView* listView = qobject_cast<GrandSearchListView*>(sender());
    if (listView) {
        for (int i = 0; i < m_vGroupWidgets.count(); ++i) {
            if (hasSelectItem(i)) {
                ViewMoreButton *viewMoreBtn = m_vGroupWidgets.at(i)->getViewMoreButton();
                Q_ASSERT(viewMoreBtn);
                viewMoreBtn->setSelected(false);

                GrandSearchListView *tmpListView = m_vGroupWidgets.at(i)->getListView();
                Q_ASSERT(tmpListView);
                if (listView != tmpListView) {
                    tmpListView->setCurrentIndex(QModelIndex());
                } else {
                    searchGroupName = m_vGroupWidgets.at(i)->searchGroupName();
                    m_customSelected = true;
                }
            }
        }
    }
    // 通知当前选择项改变
    emit sigCurrentItemChanged(searchGroupName, item);
}

bool MatchWidget::selectFirstItem(int groupNumber)
{
    if (this->isHidden())
        return false;

    for (int i = groupNumber; i < m_vGroupWidgets.count(); ++i) {

        GroupWidget *group = m_vGroupWidgets.at(i);
        Q_ASSERT(group);

        ViewMoreButton *viewMoreBtn = group->getViewMoreButton();
        Q_ASSERT(viewMoreBtn);

        if (viewMoreBtn->isVisible()) {
            viewMoreBtn->setSelected(true);
            currentIndexChanged(group->searchGroupName(), QModelIndex());
            return true;
        }

        GrandSearchListView *listView = group->getListView();
        Q_ASSERT(listView);

        if (Q_LIKELY(listView->rowCount() > 0)) {
            const QModelIndex &index = listView->model()->index(0, 0);
            if (Q_LIKELY(index.isValid())) {
                listView->setCurrentIndex(index);
                currentIndexChanged(group->searchGroupName(), index);
                return true;
            }
        }
    }
    return false;
}

bool MatchWidget::selectLastItem(int groupNumber)
{
    for (int i = groupNumber; i > -1 && i < m_vGroupWidgets.count(); --i) {

        GroupWidget *group = m_vGroupWidgets.at(i);
        Q_ASSERT(group);

        GrandSearchListView *listView = group->getListView();
        Q_ASSERT(listView);

        if (Q_LIKELY(listView->rowCount() > 0)) {
            const QModelIndex &index = listView->model()->index(listView->rowCount() - 1, 0);
            if (Q_LIKELY(index.isValid())) {
                listView->setCurrentIndex(index);
                currentIndexChanged(group->searchGroupName(), index);
                return true;
            }
        }
    }
    return false;
}

bool MatchWidget::hasSelectItem()
{
    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {
        if (hasSelectItem(i))
            return true;
    }
    return false;
}

bool MatchWidget::hasSelectItem(int groupNumber)
{
    GroupWidget *group = m_vGroupWidgets.at(groupNumber);
    if (Q_UNLIKELY(!group))
        return false;

    ViewMoreButton *viewMoreBtn = group->getViewMoreButton();
    if (Q_UNLIKELY(!viewMoreBtn))
        return false;

    if (viewMoreBtn->isSelected())
        return true;

    GrandSearchListView *listView = group->getListView();
    if (Q_UNLIKELY(!listView))
        return false;

    const QModelIndex &index = listView->currentIndex();
    if (index.isValid())
        return true;

    return false;
}

void MatchWidget::clearSelectItem()
{
    bool selectChanged = false;

    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {
        if (hasSelectItem(i)) {
            ViewMoreButton *viewMoreBtn = m_vGroupWidgets.at(i)->getViewMoreButton();
            Q_ASSERT(viewMoreBtn);

            GrandSearchListView *tmpListView = m_vGroupWidgets.at(i)->getListView();
            Q_ASSERT(tmpListView);

            viewMoreBtn->setSelected(false);
            tmpListView->setCurrentIndex(QModelIndex());
            selectChanged = true;
        }
    }

    if (selectChanged) {
        // 通知当前选择项改变
        MatchedItem item;
        emit sigCurrentItemChanged(QString(), item);
    }
}

void MatchWidget::adjustScrollBar()
{
    int nCurSelHeight = 0;

    //计算当前选中Item在contentWidget整体高度位置
    for (auto group : m_vGroupWidgets) {
        if (Q_UNLIKELY((!group)))
            continue;

        ViewMoreButton *viewMoreBtn = group->getViewMoreButton();
        if (Q_UNLIKELY(!viewMoreBtn))
            continue;

        GrandSearchListView* listView = group->getListView();
        if (Q_UNLIKELY(!listView))
            continue;

        if (!listView->currentIndex().isValid() && !viewMoreBtn->isSelected()) {
            nCurSelHeight += group->height();
        } else {
            nCurSelHeight += group->getCurSelectHeight();
            break;
        }
    }

    int nScrollAreaHeight = m_scrollArea->height();// 滚动区域高度
    int nCurPosValue = m_scrollArea->verticalScrollBar()->value();//滚动条当前位置
    int nCurHeight = nScrollAreaHeight + nCurPosValue;//滚动条当前可显示的内容高度

    int nOffset = 0;// 记录需要滚动的偏移量
    int nNewPosValue = 0;// 滚动条位置新值

    // 选中行定位到当前滚动区域底部，逐个向下滚动
    if (nCurSelHeight > nCurHeight) {
        nNewPosValue = nCurSelHeight - nScrollAreaHeight + 2;
        m_scrollArea->verticalScrollBar()->setValue(nNewPosValue);
    }
    // 选中行定位到当前滚动区域顶部，逐个向上滚动
    else if (nCurSelHeight <= nCurPosValue + ListItemHeight) {
        nOffset = nCurSelHeight - nCurPosValue;
        nNewPosValue = nCurPosValue + nOffset - ListItemHeight;

        if (nNewPosValue < ListItemHeight)
            nNewPosValue = 0;
        m_scrollArea->verticalScrollBar()->setValue(nNewPosValue);
    }

//    int nMin = m_scrollArea->verticalScrollBar()->minimum();
//    int nMax = m_scrollArea->verticalScrollBar()->maximum();
//    qDebug() << QString("nMin:%1 nMax:%2 nCurSelHeight%3 nCurPosValue:%4 nNewPosValue:%5").arg(nMin).arg(nMax).arg(nCurSelHeight).arg(nCurPosValue).arg(nNewPosValue);
}

void MatchWidget::currentIndexChanged(const QString &searchGroupName, const QModelIndex &index)
{
    MatchedItem item;
    if (index.isValid())
        item = index.data(DATA_ROLE).value<MatchedItem>();

    emit sigCurrentItemChanged(searchGroupName, item);
}

void MatchWidget::initUi()
{
    m_vMainLayout = new QVBoxLayout(this);
    m_vMainLayout->setContentsMargins(0, 0, 0, 0);
    m_vMainLayout->setSpacing(0);
    m_scrollArea = new DScrollArea(this);
    m_scrollArea->setFocusPolicy(Qt::NoFocus);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidgetResizable(true);
    m_scrollAreaContent = new DWidget();
    m_scrollAreaContent->setFocusPolicy(Qt::NoFocus);
    m_scrollArea->setWidget(m_scrollAreaContent);

    QPalette palette = m_scrollArea->palette();
    palette.setColor(QPalette::Background, Qt::transparent);
    m_scrollArea->setPalette(palette);
    m_scrollArea->setLineWidth(0);

    m_vMainLayout->addWidget(m_scrollArea);

    m_vScrollLayout = new QVBoxLayout(m_scrollAreaContent);
    m_vScrollLayout->setContentsMargins(0, 0, 0, 0);
    m_vScrollLayout->setSpacing(0);

}

void MatchWidget::initConnect()
{

}


void MatchWidget::reLayout()
{
    Q_ASSERT(m_scrollAreaContent);

    this->show();

    // 计算匹配结果界面整体高度
    int nTotalHeight = 0;
    for (int i = 0; i < m_vGroupWidgets.size(); i++) {
        GroupWidget *groupWidget = m_vGroupWidgets[i];
        if (Q_LIKELY(groupWidget)) {

            // 默认显示类目列表内分割横线
            groupWidget->showHorLine(true);

            // 隐藏视觉上最后一个类目列表横线
            if (i == m_vGroupWidgets.size() - 1)
                groupWidget->showHorLine(false);

            // 类目列表重新布局
            groupWidget->reLayout();
            int nHeight = groupWidget->height();
            nTotalHeight += nHeight;
        }
    }

    // 设置滚动区域内容部件整体高度
    if (Q_LIKELY(m_scrollAreaContent))
        m_scrollAreaContent->setFixedHeight(nTotalHeight);

    // 打破当前布局，根据最新显示类目列表重新布局
    if (Q_LIKELY(m_vScrollLayout)) {
        delete m_vScrollLayout;
        m_vScrollLayout = nullptr;
    }
    m_vScrollLayout = new QVBoxLayout(m_scrollAreaContent);
    m_vScrollLayout->setContentsMargins(0, 0, 0, 0);
    m_vScrollLayout->setSpacing(0);
    m_scrollAreaContent->setLayout(m_vScrollLayout);

    for (auto groupWidget : m_vGroupWidgets) {
        if (Q_LIKELY(groupWidget))
            m_vScrollLayout->addWidget(groupWidget, 0);
    }

    layout();
}

GroupWidget *MatchWidget::createGroupWidget(const QString &searchGroupName)
{
    if (Q_UNLIKELY(searchGroupName.isEmpty()))
        return nullptr;

    if (!m_groupWidgetMap[searchGroupName]) {

        GroupWidget *groupWidget = new GroupWidget(m_scrollAreaContent);
        groupWidget->setFocusPolicy(Qt::NoFocus);

        connect(groupWidget, &GroupWidget::showMore, this, &MatchWidget::reLayout);
        connect(groupWidget, &GroupWidget::sigCurrentItemChanged, this, &MatchWidget::sigCurrentItemChanged);

        GrandSearchListView *listView = groupWidget->getListView();
        Q_ASSERT(listView);
        listView->updatePreviewItemState(m_isPreviewItem);
        connect(listView, &GrandSearchListView::sigCurrentItemChanged, this, &MatchWidget::onSelectItemByMouse);
        connect(listView, &GrandSearchListView::sigItemClicked, this, &MatchWidget::sigCloseWindow);

        groupWidget->setSearchGroupName(searchGroupName);
        d_p->setGroupIcon(groupWidget);

        const QString &groupName = GroupWidget::convertDisplayName(searchGroupName);
        groupWidget->setGroupName(groupName);
        m_groupWidgetMap[searchGroupName] = groupWidget;

        return groupWidget;
    }

    return m_groupWidgetMap[searchGroupName];
}

void MatchWidget::sortVislibleGroupList()
{
    m_vGroupWidgets.clear();
    for (auto searchGroupName : m_groupHashShowOrder) {
        if (m_groupWidgetMap[searchGroupName] && !m_groupWidgetMap[searchGroupName]->isHidden())
            m_vGroupWidgets.push_back(m_groupWidgetMap[searchGroupName]);
    }

    GroupWidgetMap::Iterator itemWidget = m_groupWidgetMap.begin();
    while (itemWidget != m_groupWidgetMap.end()) {

        if (itemWidget.value()
                && !itemWidget.value()->isHidden()
                && !m_vGroupWidgets.contains(itemWidget.value()))
            m_vGroupWidgets.push_back(itemWidget.value());

        itemWidget++;
    }
}

void MatchWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);
}

void MatchWidget::resizeEvent(QResizeEvent *event)
{
    // 重写resizeEvent，手动设置类目列表宽度，保证匹配列表布局正常显示
    for (GroupWidgetMap::Iterator it = m_groupWidgetMap.begin(); it !=m_groupWidgetMap.end(); ++it) {
        if (it.value()) {
            it.value()->setFixedWidth(rect().width() - ScrollBarToListRightSpace);// 滚动条与列表右侧需要有2px间隙
        }
    }

    DWidget::resizeEvent(event);
}

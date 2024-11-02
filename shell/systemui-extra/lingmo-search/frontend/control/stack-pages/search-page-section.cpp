/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#include "search-page-section.h"
#include <QDebug>
#include <QScrollBar>
using namespace LingmoUISearch;

#define RESULT_LAYOUT_MARGINS 0,0,0,0
#define RESULT_BACKGROUND_COLOR QColor(0, 0, 0, 0)
#define DETAIL_BACKGROUND_COLOR QColor(0, 0, 0, 0)
#define DETAIL_WIDGET_TRANSPARENT 0.04
#define DETAIL_WIDGET_MARGINS 8,0,8,0
#define DETAIL_FRAME_MARGINS 8,0,0,0
#define DETAIL_ICON_HEIGHT 120
#define NAME_LABEL_WIDTH 280
#define ICON_SIZE QSize(120, 120)
#define LINE_STYLE "QFrame{background: rgba(0,0,0,0.2);}"
#define ACTION_NORMAL_COLOR QColor(55, 144, 250, 255)
#define ACTION_HOVER_COLOR QColor(64, 169, 251, 255)
#define ACTION_PRESS_COLOR QColor(41, 108, 217, 255)
#define TITLE_HEIGHT 30
#define FRAME_HEIGHT 516
#define DETAIL_FRAME_WIDTH 376

ResultArea::ResultArea(QWidget *parent) : QScrollArea(parent)
{
    qRegisterMetaType<SearchPluginIface::ResultInfo>("SearchPluginIface::ResultInfo");
    this->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    initUi();
    initConnections();
}

void ResultArea::appendWidet(ResultWidget *widget)
{
    m_mainLyt->addWidget(widget);
    setupConnectionsForWidget(widget);
    widget->clearResult();
    m_widgetList.append(widget);
    int spacing_height = m_widgetList.length() > 1 ? m_mainLyt->spacing() : 0;
    m_widget->setFixedHeight(m_widget->height() + widget->height() + spacing_height);
}

void ResultArea::insertWidget(ResultWidget *widget, int index)
{
    m_mainLyt->insertWidget(index, widget);
    setupConnectionsForWidget(widget);
    widget->clearResult();
    m_widgetList.insert(index, widget);
    qDebug() << "========insert widget:" <<  widget->pluginId() << index;
    int spacing_height = m_widgetList.length() > 1 ? m_mainLyt->spacing() : 0;
    m_widget->setFixedHeight(m_widget->height() + widget->height() + spacing_height);
}

bool ResultArea::removeWidget(const QString &pluginName)
{
    int height = 0;
    bool res(false);
    for (ResultWidget *myWidget : m_widgetList) {
        if (myWidget->pluginId() == pluginName) {
            height = myWidget->height();
            myWidget->disconnect();
            myWidget->clearResult();
            m_mainLyt->removeWidget(myWidget);
            m_widgetList.removeAll(myWidget);
            res = true;
            break;
        }
    }
    if (res) {
        int spacing_height = m_widgetList.length() > 0 ? m_mainLyt->spacing() : 0;
        m_widget->setFixedHeight(m_widget->height() - (height + spacing_height));
        qDebug() << "Remove Widget " << pluginName;
    }
    return res;
}

bool ResultArea::moveWidget(const QString& pluginName, int index)
{
    for (ResultWidget *myWidget : m_widgetList) {
        if (myWidget->pluginId() == pluginName) {
            m_mainLyt->removeWidget(myWidget);
            m_mainLyt->insertWidget(index, myWidget);//第一个插件固定为bestlist
            return true;
        }
    }
    return false;
}

/**
 * @brief ResultArea::setVisibleList 设置哪些插件可见，默认全部可见
 * @param list
 */
void ResultArea::setVisibleList(const QStringList &list)
{
    Q_FOREACH (auto widget, m_widgetList) {
        if (list.contains(widget->pluginId())) {
            widget->setEnabled(true);
        } else {
            widget->setEnabled(false);
        }
    }
}

void ResultArea::pressEnter()
{
    if (false == m_isSelected) {//未选中时默认选取bestlist第一项
        int resultNum = m_bestListWidget->getResultNum();
        if (0 == resultNum) {//无搜索结果时默认选中websearch
            for (ResultWidget * i : m_widgetList) {
                if (m_selectedPluginID == m_widgetList.back()->pluginId()) {
                    QModelIndex index = i->getModlIndex(0, 0);
                    i->setResultSelection(index);
                    m_selectedPluginID = i->pluginId();
                    m_isSelected = true;
                    break;
                }
            }
        } else {//选取bestlist第一项
            QModelIndex index = m_bestListWidget->getModlIndex(0, 0);
            m_bestListWidget->setResultSelection(index);
            m_selectedPluginID = m_bestListWidget->getWidgetName();
            m_isSelected = true;
        }
    } else {//选中状态时默认启动action首项
        //先判断详情页是否打开
        if (m_detail_open_state) {
            if (m_selectedPluginID == m_bestListWidget->getWidgetName()) {//最佳匹配
                m_bestListWidget->activateIndex();
            } else {
                for (ResultWidget * i : m_widgetList) {
                    if (m_selectedPluginID == i->pluginId()) {
                        i->activateIndex();
                        break;
                    }
                }
            }
        } else {//打开详情页
            m_detail_open_state = true;
            sendKeyPressSignal(m_selectedPluginID);
        }
    }
}

void ResultArea::pressDown()
{
    if (m_selectedPluginID == m_widgetList.back()->pluginId()) {//当前为web search，暂不处理
        return;
    } else if (m_selectedPluginID == m_bestListWidget->getWidgetName()) {
        QModelIndex index = m_bestListWidget->getCurrentSelection();
        int maxNum = m_bestListWidget->getExpandState() ?
                    m_bestListWidget->getResultNum() : (m_bestListWidget->getResultNum() < NUM_LIMIT_SHOWN_DEFAULT ?
                                                            m_bestListWidget->getResultNum() : NUM_LIMIT_SHOWN_DEFAULT);
        if (index.row() < maxNum - 1 and index.row() >= 0) {
            int row = index.row();
            QModelIndex setIndex = m_bestListWidget->getModlIndex(++row, 0);
            m_bestListWidget->setResultSelection(setIndex);
            sendKeyPressSignal(m_selectedPluginID);
        } else if (index.row() >= maxNum - 1 or index.row() < 0) {//跳转下一个widget
            m_bestListWidget->clearResultSelection();
            for (ResultWidget * plugin : m_widgetList) {
                if (plugin->getResultNum() != 0) {
                    QModelIndex resultIndex = plugin->getModlIndex(0, 0);
                    plugin->setResultSelection(resultIndex);
                    m_selectedPluginID = plugin->pluginId();
                    sendKeyPressSignal(m_selectedPluginID);
                    break;
                }
            }
        } else {
            qWarning() << "QModelIndex error ! row:" << index.row() << "maxNum:" << maxNum;
        }
    } else {
        for (ResultWidget * plugin : m_widgetList) {
            if (m_selectedPluginID == plugin->pluginId()) {
                QModelIndex index = plugin->getCurrentSelection();
                int maxNum = plugin->getExpandState() ?
                            plugin->getResultNum() : (plugin->getResultNum() < NUM_LIMIT_SHOWN_DEFAULT ?
                                                                    plugin->getResultNum() : NUM_LIMIT_SHOWN_DEFAULT);
                if (index.row() < maxNum - 1 and index.row() >= 0) {
                    int row = index.row();
                    QModelIndex setIndex = plugin->getModlIndex(++row, 0);
                    plugin->setResultSelection(setIndex);
                    sendKeyPressSignal(m_selectedPluginID);
                } else if (index.row() >= maxNum - 1 or index.row() < 0) {//跳转下一个widget
                    plugin->clearResultSelection();
                    int indexNum = m_widgetList.indexOf(plugin);
                    bool findNextWidget = false;
                    while (++indexNum < m_widgetList.size()) {
                        plugin = m_widgetList[indexNum];
                        if (plugin->getResultNum() != 0) {
                            QModelIndex resultIndex = plugin->getModlIndex(0, 0);
                            plugin->setResultSelection(resultIndex);
                            m_selectedPluginID = plugin->pluginId();
                            findNextWidget = true;
                            sendKeyPressSignal(m_selectedPluginID);
                            break;
                        }
                    }
                    if (findNextWidget){
                        break;
                    }
                } else {
                    qWarning() << "QModelIndex error ! row:" << index.row() << "maxNum:" << maxNum;
                }
            }
        }
    }
}

void ResultArea::pressUp()
{
    if (!m_isSelected) {
        return;
    }
    if (m_selectedPluginID == m_bestListWidget->getWidgetName()) {
        QModelIndex index = m_bestListWidget->getCurrentSelection();
        int maxNum = m_bestListWidget->getExpandState() ?
                    m_bestListWidget->getResultNum() : (m_bestListWidget->getResultNum() < NUM_LIMIT_SHOWN_DEFAULT ?
                                                            m_bestListWidget->getResultNum() : NUM_LIMIT_SHOWN_DEFAULT);
        if (index.row() > 0 and index.row() < maxNum) {
            int row = index.row();
            QModelIndex setIndex = m_bestListWidget->getModlIndex(--row, 0);
            m_bestListWidget->setResultSelection(setIndex);
            sendKeyPressSignal(m_selectedPluginID);
        } else if (index.row() == 0) {
            //已到最上层，暂不处理
        } else {
            QModelIndex setIndex = m_bestListWidget->getModlIndex(--maxNum, 0);
            m_bestListWidget->setResultSelection(setIndex);
            sendKeyPressSignal(m_selectedPluginID);
        }
    } else {
        for (ResultWidget * plugin : m_widgetList) {
            if (m_selectedPluginID == plugin->pluginId()) {
                int indexMaxNum = plugin->getExpandState() ?
                            plugin->getResultNum() : (plugin->getResultNum() < NUM_LIMIT_SHOWN_DEFAULT ?
                                                                    plugin->getResultNum() : NUM_LIMIT_SHOWN_DEFAULT);
                QModelIndex index = plugin->getCurrentSelection();
                if (index.row() > 0 and index.row() < indexMaxNum) {
                    int row = index.row();
                    QModelIndex setIndex = plugin->getModlIndex(--row, 0);
                    plugin->setResultSelection(setIndex);
                    sendKeyPressSignal(m_selectedPluginID);
                } else if (index.row() == 0) {//跳转下一个widget
                    plugin->clearResultSelection();
                    int indexNum = m_widgetList.indexOf(plugin);
                    bool findNextWidget = false;
                    while (--indexNum >= 0) {
                        plugin = m_widgetList[indexNum];
                        if (plugin->getResultNum() != 0) {
                            int maxNum = plugin->getExpandState() ?
                                        plugin->getResultNum() : (plugin->getResultNum() < NUM_LIMIT_SHOWN_DEFAULT ?
                                                                                plugin->getResultNum() : NUM_LIMIT_SHOWN_DEFAULT);
                            QModelIndex resultIndex = plugin->getModlIndex(maxNum - 1, 0);
                            plugin->setResultSelection(resultIndex);
                            m_selectedPluginID = plugin->pluginId();
                            findNextWidget = true;
                            sendKeyPressSignal(m_selectedPluginID);
                            break;
                        }
                    }
                    if (indexNum < 0) {//下一项是best list
                        int bestListNum = m_bestListWidget->getExpandState() ?
                                    m_bestListWidget->getResultNum() : (m_bestListWidget->getResultNum() < NUM_LIMIT_SHOWN_DEFAULT ?
                                                                            m_bestListWidget->getResultNum() : NUM_LIMIT_SHOWN_DEFAULT);
                        QModelIndex setIndex = m_bestListWidget->getModlIndex(--bestListNum, 0);
                        m_bestListWidget->setResultSelection(setIndex);
                        m_selectedPluginID = m_bestListWidget->getWidgetName();
                        m_isSelected = true;
                        sendKeyPressSignal(m_selectedPluginID);
                    }
                    if (findNextWidget){
                        break;
                    }
                } else {
                    QModelIndex setIndex = plugin->getModlIndex(indexMaxNum - 1, 0);
                    plugin->setResultSelection(setIndex);
                    sendKeyPressSignal(m_selectedPluginID);
                }
            }
        }
    }
}

int ResultArea::getVScrollBarWidth()
{
    if (verticalScrollBar()->isVisible()) {
        return this->verticalScrollBar()->width();
    }
    return 0;
}

bool ResultArea::getSelectedState()
{
    return m_isSelected;
}

void ResultArea::sendKeyPressSignal(QString &pluginID)
{
    int height(0);
    int resultHeight = m_bestListWidget->getResultHeight();
    if (pluginID == m_bestListWidget->getWidgetName()) {
        QModelIndex index = m_bestListWidget->getCurrentSelection();
        height = index.row() == 0 ? 0 : index.row() * resultHeight + TITLE_HEIGHT;
        height = (height - resultHeight) < 0 ? 0 : height - resultHeight;
        this->ensureVisible(0, height, 0, 0);
        if (m_detail_open_state) {
            Q_EMIT this->keyPressChanged(m_bestListWidget->getPluginInfo(index), m_bestListWidget->getIndexResultInfo(index));
        }
    } else {
        height += m_bestListWidget->height();
        for (ResultWidget *plugin : m_widgetList) {
            if (pluginID == plugin->pluginId()) {
                QModelIndex index = plugin->getCurrentSelection();
                //todo 这里偶尔会导致崩溃@jxx，暂时规避。
                if(!index.isValid()) {
                    break;
                }
                height += index.row() == 0 ? 0 : index.row() * resultHeight + TITLE_HEIGHT;
                int moreHeight = index.row() == 0 ? (TITLE_HEIGHT + resultHeight * 2) : (resultHeight * 2);
                this->ensureVisible(0, height + moreHeight, 0, 0);
                height = (height - resultHeight) < 0 ? 0 : height - resultHeight;
                this->ensureVisible(0, height, 0, 0);
                if (m_detail_open_state) {
                    Q_EMIT this->keyPressChanged(m_selectedPluginID, plugin->getIndexResultInfo(index));
                }
                break;
            } else {
                height += plugin->height();
            }
        }
    }
}

void ResultArea::onWidgetSizeChanged()
{
    int whole_height = 0;
    Q_FOREACH (ResultWidget *widget, m_widgetList) {
        whole_height += widget->height();
    }
    whole_height += m_bestListWidget->height();

    int spacing_height = m_widgetList.length() > 1 ? m_mainLyt->spacing() : 0;
    m_widget->setFixedHeight(whole_height + spacing_height * (m_widgetList.length() - 1));
    Q_EMIT this->resizeHeight(whole_height + spacing_height * (m_widgetList.length() - 1));
}

void ResultArea::setSelectionInfo(QString &pluginID)
{
    m_detail_open_state = true;
    m_isSelected = true;
    m_selectedPluginID = pluginID;
    if (m_selectedPluginID != m_bestListWidget->getWidgetName()) {
        m_bestListWidget->clearResultSelection();
        m_isClicked = true;
    }
}

void ResultArea::mousePressEvent(QMouseEvent *event)
{
//    qDebug() << "mouse pressed"  << event->source() << event->button();
//    m_pressPoint = event->pos();
    return QScrollArea::mousePressEvent(event);
}

void ResultArea::mouseMoveEvent(QMouseEvent *event)
{
//    if(m_pressPoint.isNull()) {
//        return QScrollArea::mouseMoveEvent(event);
//    }
//    int delta = ((event->pos().y() - m_pressPoint.y()) / this->widget()->height()) * verticalScrollBar()->maximum();
//    this->verticalScrollBar()->setValue(verticalScrollBar()->value() + delta);
//    event->accept();
    return QScrollArea::mouseMoveEvent(event);
}

void ResultArea::mouseReleaseEvent(QMouseEvent *event)
{
    return QScrollArea::mouseReleaseEvent(event);
}

bool ResultArea::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = dynamic_cast<QMouseEvent *>(event);
        if (e->source() == Qt::MouseEventSynthesizedByApplication) {
            qDebug() << "MouseButtonPress MouseEventSynthesizedByApplication";
            m_pressPoint = m_widget->mapFrom(this, e->pos());
            event->accept();
            return true;
        }
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *e = dynamic_cast<QMouseEvent *>(event);
        if (e->source() == Qt::MouseEventSynthesizedByApplication) {
            qDebug() << "MouseMove MouseEventSynthesizedByApplication";
            int delta = m_pressPoint.y() - m_widget->mapFrom(this, e->pos()).y();
            //            qDebug() << "last pos:" << m_pressPoint.y();
            //            qDebug() << "new pos:" << m_widget->mapFrom(this, e->touchPoints().at(0).pos().toPoint()).y();
            //            qDebug() << "delta" << delta;
            //            qDebug() << "value" << verticalScrollBar()->value() << "--" << verticalScrollBar()->value() + delta;
            this->verticalScrollBar()->setValue(verticalScrollBar()->value() + delta);
            m_pressPoint = m_widget->mapFrom(this,e->pos());
            return true;
        }
    }
    return QScrollArea::viewportEvent(event);
}

void ResultArea::initUi()
{
    m_scrollBar = new ResultScrollBar(this);
    this->setVerticalScrollBar(m_scrollBar);
//    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    QPalette pal = palette();
    QPalette scroll_bar_pal = this->verticalScrollBar()->palette();
//    pal.setColor(QPalette::Base, RESULT_BACKGROUND_COLOR);
    pal.setColor(QPalette::Window, RESULT_BACKGROUND_COLOR);
    scroll_bar_pal.setColor(QPalette::Base, RESULT_BACKGROUND_COLOR);
    this->verticalScrollBar()->setPalette(scroll_bar_pal);
    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    this->setFrameShape(QFrame::Shape::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setPalette(pal);
    this->setWidgetResizable(true);
    this->setFrameShape(QFrame::Shape::NoFrame);
    m_widget = new QWidget(this);
    this->setWidget(m_widget);
    m_mainLyt = new QVBoxLayout(m_widget);
    m_widget->setLayout(m_mainLyt);
    m_bestListWidget = new BestListWidget(this);
    m_bestListWidget->clearResult();
    m_mainLyt->addWidget(m_bestListWidget);

    m_mainLyt->setContentsMargins(RESULT_LAYOUT_MARGINS);
    this->widget()->setContentsMargins(0,0,0,0);
    m_mainLyt->setSpacing(0);

    m_titleLabel = new TitleLabel(this);
    m_titleLabel->hide();
}

void ResultArea::initConnections()
{
    connect(this, &ResultArea::startSearch, m_bestListWidget, &BestListWidget::startSearch);
    connect(this, &ResultArea::startSearch, this, [=] () {
        m_detail_open_state = false;
        m_isSelected = false;
        m_selectedPluginID = "";
        m_isClicked = false;
    });
    connect(this, &ResultArea::stopSearch, m_bestListWidget, &BestListWidget::stopSearch);
    connect(m_bestListWidget, &BestListWidget::sizeChanged, this, &ResultArea::onWidgetSizeChanged);
    connect(m_bestListWidget, &BestListWidget::sizeChanged, this, [=] () {
        QModelIndex index = m_bestListWidget->getModlIndex(0, 0);
        if (index.isValid() && !m_isClicked) {
            m_bestListWidget->setResultSelection(index);
            m_selectedPluginID = m_bestListWidget->getWidgetName();
            m_isSelected = true;

            if(!m_widgetList.isEmpty()) {
                m_widgetList.last()->clearResultSelection();
            }
        }
    });

    connect(m_bestListWidget, &BestListWidget::currentRowChanged, this, &ResultArea::currentRowChanged);
    connect(m_bestListWidget, &BestListWidget::currentRowChanged, this, [=] () {
        m_detail_open_state = true;
        m_isSelected = true;
        m_selectedPluginID = m_bestListWidget->getWidgetName();
    });
    connect(this, &ResultArea::clearSelectedRow, m_bestListWidget, &BestListWidget::clearSelectedRow);
    connect(this, &ResultArea::resizeWidth, this, [=] (const int &size) {
        m_bestListWidget->setFixedWidth(size);
    });
    connect(m_bestListWidget, &BestListWidget::rowClicked, this, &ResultArea::rowClicked);
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, [=] (int value) {//判断显示和隐藏逻辑
        Q_FOREACH(auto widget, m_widgetList) {
            if (!widget->getExpandState()) {
                continue;
            }
            if (value < (widget->pos().ry() + TITLE_HEIGHT)
                    or value > (widget->pos().ry() + widget->height() - FRAME_HEIGHT + 2*TITLE_HEIGHT)) {//暂定下一项标题显示完全后悬浮标题隐藏
                if (!m_titleLabel->isHidden()) {
                    m_titleLabel->hide();
                    this->setViewportMargins(0,0,0,0);
                }
            } else {
                if (m_titleLabel->isHidden()) {
                    m_titleLabel->setText(widget->pluginName());
                    m_titleLabel->setShowMoreLableVisible();//防止多项展开后无法收回其他项
                    m_titleLabel->show();
                    this->setViewportMargins(0,TITLE_HEIGHT,0,0);
                }
                break;
            }
        }
    });
    connect(this->m_titleLabel, &TitleLabel::retractClicked, this, [=] () {
        Q_FOREACH(auto widget, m_widgetList) {
            if (widget->pluginName() == m_titleLabel->text()) {
                if (!m_titleLabel->isHidden()) {
                    m_titleLabel->hide();
                    this->setViewportMargins(0,0,0,0);
                }
                widget->reduceListSlot();
                this->verticalScrollBar()->setValue(widget->pos().ry());
                widget->resetTitleLabel();
            }
        }
    });
    connect(this, &ResultArea::resizeWidth, this, [=] (int size) {
        m_titleLabel->setFixedWidth(size);
    });
    connect(m_scrollBar, &ResultScrollBar::scrollBarAppeared, this, &ResultArea::scrollBarAppeared);
    connect(m_scrollBar, &ResultScrollBar::scrollBarIsHideen, this, &ResultArea::scrollBarIsHideen);
}

void ResultArea::setupConnectionsForWidget(ResultWidget *widget)
{
    connect(this, &ResultArea::startSearch, widget, &ResultWidget::startSearch);
    connect(this, &ResultArea::startSearch, this, [=] () {
        if (!m_titleLabel->isHidden()) {
            m_titleLabel->hide();
            this->setViewportMargins(0,0,0,0);
        }
    });
    connect(this, &ResultArea::stopSearch, widget, &ResultWidget::stopSearch);
    connect(widget, &ResultWidget::sizeChanged, this, &ResultArea::onWidgetSizeChanged);
    connect(widget, &ResultWidget::sizeChanged, this, [=] () {
        if (widget->pluginId() == m_widgetList.back()->pluginId() and m_selectedPluginID != m_bestListWidget->getWidgetName()) {//每次搜索默认选中websearch，由bestlist取消
            QModelIndex index = widget->getModlIndex(0, 0);
            if (index.isValid()) {
                widget->setResultSelection(index);
                m_isSelected = true;
                m_selectedPluginID = widget->pluginId();
            }
        }
    });
    connect(widget, &ResultWidget::showMoreClicked, this, [=] () {//点击展开搜索结果后   显示悬浮窗
        if (widget->height() > FRAME_HEIGHT) {//当前widget高度大于搜索结果界面高度则显示悬浮窗
            this->verticalScrollBar()->setValue(widget->pos().ry() + TITLE_HEIGHT); //置顶当前类型搜索结果,不显示标题栏
            viewport()->stackUnder(m_titleLabel);
            m_titleLabel->setText(widget->pluginName());
            m_titleLabel->setFixedSize(widget->width(), TITLE_HEIGHT);
            m_titleLabel->setShowMoreLableVisible();
            if (m_titleLabel->isHidden()) {
                m_titleLabel->show();
                this->setViewportMargins(0,TITLE_HEIGHT,0,0);
            }
        } else {
            this->verticalScrollBar()->setValue(widget->pos().ry()); //置顶当前类型搜索结果，显示标题栏
        }
    });
    connect(widget, &ResultWidget::retractClicked, this, [=] () {//点击收起搜索结果后
        if (!m_titleLabel->isHidden()) {
            this->setViewportMargins(0,0,0,0);
            m_titleLabel->hide();
        }
    });
    connect(widget, &ResultWidget::sendBestListData, m_bestListWidget, &BestListWidget::sendBestListData);

    connect(SearchPluginManager::getInstance(), &SearchPluginManager::unregistered, this, &ResultArea::removeWidget);
}

DetailArea::DetailArea(QWidget *parent) : QScrollArea(parent)
{
    initUi();
    connect(this, &DetailArea::setWidgetInfo, m_detailWidget, &DetailWidget::updateDetailPage);
    connect(this, &DetailArea::setWidgetInfo, this, &DetailArea::show);
}

void DetailArea::initUi()
{
    QPalette pal = palette();
//    pal.setColor(QPalette::Base, DETAIL_BACKGROUND_COLOR);
    pal.setColor(QPalette::Window, DETAIL_BACKGROUND_COLOR);
    QPalette scroll_bar_pal = this->verticalScrollBar()->palette();
    scroll_bar_pal.setColor(QPalette::Base, RESULT_BACKGROUND_COLOR);
    this->verticalScrollBar()->setPalette(scroll_bar_pal);
    this->setPalette(pal);
    this->setFrameShape(QFrame::Shape::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setWidgetResizable(true);
    this->setFixedSize(DETAIL_FRAME_WIDTH, FRAME_HEIGHT);
//    this->setStyleSheet("QScrollArea{border:2px solid red;}");
    this->setContentsMargins(0,0,0,0);
    m_detailWidget = new DetailWidget(this);
    this->setWidget(m_detailWidget);
    this->hide();
}

DetailWidget::DetailWidget(QWidget *parent) : QWidget(parent)
{
    m_radius = qApp->style()->property("maxRadius").toInt();
    connect(qApp, &QApplication::paletteChanged, this, [&]() {
        m_radius = qApp->style()->property("maxRadius").toInt();
    });
    this->setFixedWidth(368);
    m_mainLyt = new QVBoxLayout(this);
    this->setLayout(m_mainLyt);
    m_mainLyt->setContentsMargins(DETAIL_WIDGET_MARGINS);
    m_mainLyt->setAlignment(Qt::AlignHCenter);
}

QString escapeHtml(const QString & str) {
    QString temp = str;
    temp.replace("<", "&lt;");
    temp.replace(">", "&gt;");
    return temp;
}


void DetailWidget::updateDetailPage(const QString &plugin_name, const SearchPluginIface::ResultInfo &info)
{
    if(m_detailPage) {
        if(m_currentPluginId == plugin_name) {
            SearchPluginManager::getInstance()->getPlugin(plugin_name)->detailPage(info);
        } else {
            m_mainLyt->removeWidget(m_detailPage);
            m_detailPage->hide();
            m_detailPage = SearchPluginManager::getInstance()->getPlugin(plugin_name)->detailPage(info);
            m_detailPage->setParent(this);
            m_mainLyt->addWidget(m_detailPage);
            m_detailPage->show();
//            m_mainLyt->insertWidget(0, m_detailPage, 0);
        }
    } else {
        m_detailPage = SearchPluginManager::getInstance()->getPlugin(plugin_name)->detailPage(info);
        m_detailPage->setParent(this);
        m_mainLyt->addWidget(m_detailPage);
//        m_mainLyt->insertWidget(0, m_detailPage, 0);
    }
    m_currentPluginId = plugin_name;
}

void DetailWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QRect rect = this->rect().adjusted(0, 0, 0, 0);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.setBrush(opt.palette.color(QPalette::Text));
    p.setOpacity(DETAIL_WIDGET_TRANSPARENT);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect, m_radius, m_radius);
    return QWidget::paintEvent(event);
}

void DetailWidget::clearLayout(QLayout *layout)
{
    if(!layout) return;
    QLayoutItem * child;
    while((child = layout->takeAt(0)) != 0) {
        if(child->widget()) {
            child->widget()->setParent(NULL);
        }
        delete child;
    }
    child = NULL;
}

ResultScrollBar::ResultScrollBar(QWidget *parent) : QScrollBar(parent)
{

}

void ResultScrollBar::showEvent(QShowEvent *event)
{
    Q_EMIT this->scrollBarAppeared();
    return QScrollBar::showEvent(event);
}

void ResultScrollBar::hideEvent(QHideEvent *event)
{
    Q_EMIT this->scrollBarIsHideen();
    return QScrollBar::hideEvent(event);
}

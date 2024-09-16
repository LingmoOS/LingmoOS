// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderselectdialog.h"
#include "views/leftviewdelegate.h"
#include "views/leftviewsortfilter.h"
#include "globaldef.h"

#include <DApplication>
#include <DApplicationHelper>

#include <QVBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>

FolderSelectView::FolderSelectView(QWidget *parent)
    : DTreeView(parent)
{
}

/**
 * @brief mousePressEvent 鼠标press事件处理函数
 * @return event 事件
 */
void FolderSelectView::mousePressEvent(QMouseEvent *event)
{
    //此页面只需单击选中功能，此处屏蔽辅助功能键
    event->setModifiers(Qt::NoModifier);
    //触摸屏手势操作
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //更新触控起始时间点与起始位置
        m_touchPressPointY = event->pos().y();
        m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
        return;
    }
    //如果选择位置为空，不继续执行，避免取消当前选中
    if (indexAt(event->pos()).isValid())
        DTreeView::mousePressEvent(event);
}

/**
 * @brief mouseReleaseEvent 鼠标release事件处理函数
 * @return event 事件
 */
void FolderSelectView::mouseReleaseEvent(QMouseEvent *event)
{
    //触摸屏手势操作，恢复参数状态与判断选中
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        if (m_isTouchSliding)
            m_isTouchSliding = false;
        else {
            //如果触摸点击未移动，选中当前index
            if (indexAt(event->pos()).isValid())
                setCurrentIndex(indexAt(event->pos()));
        }
        return;
    }
    return DTreeView::mouseReleaseEvent(event);
}

/**
 * @brief keyPressEvent 键盘press事件处理函数
 * @return event 事件
 */
void FolderSelectView::keyPressEvent(QKeyEvent *event)
{
    //QAbstractItemView底层问题，索引0按上键会取消选中效果，通过keypress屏蔽
    if (event->key() == Qt::Key_Up && currentIndex().row() == 0) {
        event->ignore();
    } else if (event->key() == Qt::Key_PageDown || event->key() == Qt::Key_PageUp || event->key() == Qt::Key_Home) {
        event->ignore();
    } else {
        DTreeView::keyPressEvent(event);
    }
}

/**
 * @brief mouseMoveEvent 鼠标move事件处理函数
 * @return event 事件
 */
void FolderSelectView::mouseMoveEvent(QMouseEvent *event)
{
    //此处解决选择闪烁问题
    event->setModifiers(Qt::NoModifier);
    //触摸屏收拾操作，判断滚动
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        return doTouchMoveEvent(event);
    }
    DTreeView::mouseMoveEvent(event);
}

void FolderSelectView::focusInEvent(QFocusEvent *e)
{
    LeftViewDelegate *delegate = dynamic_cast<LeftViewDelegate *>(itemDelegate());
    if (e->reason() == Qt::TabFocusReason) {
        if (!selectedIndexes().count()) {
            QModelIndex index = model()->index(0, 0).child(0, 0);
            setCurrentIndex(index);
        }
        delegate->setTabFocus(true);
    } else {
        delegate->setTabFocus(false);
    }
    DTreeView::focusInEvent(e);
}
void FolderSelectView::focusOutEvent(QFocusEvent *e)
{
    LeftViewDelegate *delegate = dynamic_cast<LeftViewDelegate *>(itemDelegate());
    delegate->setTabFocus(false);
    DTreeView::focusOutEvent(e);
}

/**
 * @brief doTouchMoveEvent 手势滑动处理函数
 * @return event 事件
 */
void FolderSelectView::doTouchMoveEvent(QMouseEvent *event)
{
    //处理触摸屏单指滑动事件, distY为上下移动距离，timeInterval为滑动时间间隔
    double distY = event->pos().y() - m_touchPressPointY;
    qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 timeInterval = currentTime - m_touchPressStartMs;
    //上下移动距离超过10px执行滚动操作，滚动过程中移动距离大于5继续执行
    if (m_isTouchSliding) {
        if (qAbs(distY) > 5)
            handleTouchSlideEvent(timeInterval, distY, event->pos());
    } else {
        if (qAbs(distY) > 10) {
            m_isTouchSliding = true;
            handleTouchSlideEvent(timeInterval, distY, event->pos());
        }
    }
}

/**
 * @brief doTouchMoveEvent 手势滑动处理
 * @return timeInterval 时间间隔
 * @return distY 上下滑动距离
 * @return point 上一次鼠标点击位置
 */
void FolderSelectView::handleTouchSlideEvent(qint64 timeInterval, double distY, QPoint point)
{
    if (timeInterval == 0)
        return;
    //根据滑动速度计算滚动步长
    double param = ((qAbs(distY)) / timeInterval) + 0.3;
    verticalScrollBar()->setSingleStep(static_cast<int>(20 * param));
    verticalScrollBar()->triggerAction((distY > 0) ? QScrollBar::SliderSingleStepSub : QScrollBar::SliderSingleStepAdd);
    //更新用于下次判断的位置和时间节点
    m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_touchPressPointY = point.y();
}

/**
 * @brief FolderSelectDialog::FolderSelectDialog
 * @param model 数据模型
 * @param parent
 */
FolderSelectDialog::FolderSelectDialog(QStandardItemModel *model, QWidget *parent)
    : DAbstractDialog(parent)
{
    m_model = new LeftViewSortFilter(this);
    m_model->setSourceModel(model);
    initUI();
    initConnections();
    m_model->sort(0, Qt::DescendingOrder);
}

/**
 * @brief FolderSelectDialog::initUI
 */
void FolderSelectDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 0, 10);

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setIconSize(QSize(50, 50));
    m_view = new FolderSelectView(this);
    m_view->setModel(m_model);
    m_view->setContextMenuPolicy(Qt::NoContextMenu);
    m_delegate = new LeftViewDelegate(m_view);
    //更新代理判断标志
    m_delegate->setSelectView(true);
    m_delegate->setDrawNotesNum(false);
    m_view->setItemDelegate(m_delegate);
    m_view->setHeaderHidden(true);
    m_view->setItemsExpandable(false);
    m_view->setIndentation(0);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QModelIndex notepadRootIndex = m_model->index(0, 0);
    m_view->expand(notepadRootIndex);
    m_view->setFrameShape(DTreeView::NoFrame);

    m_labMove = new DLabel(this);
    DFontSizeManager::instance()->bind(m_labMove, DFontSizeManager::T6, QFont::Medium);
    m_labMove->setText(DApplication::translate("FolderSelectDialog", "Move Notes"));

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->addStretch();
    titleLayout->setContentsMargins(10, 0, 0, 0);
    titleLayout->addSpacing(m_closeButton->width() / 3);

    titleLayout->addWidget(m_labMove, 0, Qt::AlignCenter | Qt::AlignVCenter);
    titleLayout->addStretch();
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    m_noteInfo = new DLabel(this);
    //长度不超过视图宽度
    m_noteInfo->setFixedWidth(VNOTE_SELECTDIALOG_W - 20);
    //初始化标题与提示字体颜色
    bool isDark = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType ? true : false;
    refreshTextColor(isDark);

    DFontSizeManager::instance()->bind(m_noteInfo, DFontSizeManager::T6, QFont::Normal);
    QHBoxLayout *actionBarLayout = new QHBoxLayout();
    actionBarLayout->setSpacing(5);
    actionBarLayout->setContentsMargins(0, 0, 0, 0);

    m_cancelBtn = new DPushButton(this);
    m_confirmBtn = new DSuggestButton(this);

    m_cancelBtn->setText(DApplication::translate("FolderSelectDialog", "Cancel", "button"));
    m_confirmBtn->setText(DApplication::translate("FolderSelectDialog", "Confirm", "button"));

    m_buttonSpliter = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(m_buttonSpliter);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    m_buttonSpliter->setPalette(pa);
    m_buttonSpliter->setBackgroundRole(QPalette::Background);
    m_buttonSpliter->setAutoFillBackground(true);
    m_buttonSpliter->setFixedSize(4, 28);

    actionBarLayout->addWidget(m_cancelBtn);
    actionBarLayout->addWidget(m_buttonSpliter);
    actionBarLayout->addWidget(m_confirmBtn);
    actionBarLayout->addSpacing(10);

    //实现内部视图列表圆角
    DFrame *viewFrame = new DFrame(this);
    QHBoxLayout *viewFrameLayout = new QHBoxLayout();
    //根据ui调整边距大小

    viewFrameLayout->setContentsMargins(5, 5, 0, 5);
    viewFrameLayout->addWidget(m_view);
    viewFrame->setLayout(viewFrameLayout);
    viewFrame->setContentsMargins(0, 0, 10, 0);
    //设置外部frame背景透明
    viewFrame->setAttribute(Qt::WA_TranslucentBackground, true);

    mainLayout->addLayout(titleLayout);
    //解决提示内容不居中问题
    QHBoxLayout *noteInfoLayout = new QHBoxLayout();
    noteInfoLayout->addStretch();
    noteInfoLayout->addWidget(m_noteInfo, Qt::AlignHCenter);
    noteInfoLayout->addSpacing(10);
    noteInfoLayout->addStretch();
    //    mainLayout->addWidget(m_noteInfo, 0, Qt::AlignCenter);
    mainLayout->addLayout(noteInfoLayout, 0);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(viewFrame, 1);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(actionBarLayout);
    this->setLayout(mainLayout);
}

/**
 * @brief FolderSelectDialog::initConnections
 */
void FolderSelectDialog::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [=]() {
        this->reject();
    });

    connect(m_confirmBtn, &DPushButton::clicked, this, [=]() {
        this->accept();
    });

    connect(m_closeButton, &DWindowCloseButton::clicked, this, [this]() {
        this->close();
    });

    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &FolderSelectDialog::onVNoteFolderSelectChange);

    //由于ui中颜色与dtk颜色库不对应，需要手动判断并设置s
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, [=] {
        bool isDark = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType ? true : false;
        this->refreshTextColor(isDark);
    });
    //字体切换长度适应
    connect(qGuiApp, &QGuiApplication::fontChanged, this, &FolderSelectDialog::onFontChanged);
}

/**
 * @brief FolderSelectDialog::themeChanged
 * @param dark 深色主题或浅色主题
 * 主题切换刷新文本颜色
 */
void FolderSelectDialog::refreshTextColor(bool dark)
{
    // 黑色主题
    if (dark) {
        //标题
        DPalette titlePalette = DApplicationHelper::instance()->palette(m_labMove);
        QColor color = QColor(255, 255, 255);
        titlePalette.setBrush(DPalette::WindowText, color);
        DApplicationHelper::instance()->setPalette(m_labMove, titlePalette);
        //提示内容
        DPalette infoPalette = DApplicationHelper::instance()->palette(m_noteInfo);
        color.setAlphaF(0.7);
        infoPalette.setBrush(DPalette::WindowText, color);
        //30%的透明度
        DApplicationHelper::instance()->setPalette(m_noteInfo, infoPalette);
    }
    //白色主题
    else {
        //标题
        DPalette titlePalette = DApplicationHelper::instance()->palette(m_labMove);
        QColor color = QColor(0, 0, 0, 1);
        //10%的透明度
        color.setAlphaF(0.9);
        titlePalette.setBrush(DPalette::WindowText, color);
        DApplicationHelper::instance()->setPalette(m_labMove, titlePalette);
        //提示内容
        DPalette infoPalette = DApplicationHelper::instance()->palette(m_noteInfo);
        color.setAlphaF(1);
        //30%的透明度
        color.setAlphaF(0.7);
        infoPalette.setBrush(DPalette::WindowText, color);
        DApplicationHelper::instance()->setPalette(m_noteInfo, infoPalette);
    }
}

/**
 * @brief FolderSelectDialog::setNoteContext
 * @param text 移动笔记信息
 */
void FolderSelectDialog::setNoteContextInfo(const QString &text, int notesNumber)
{
    m_noteInfo->setAlignment(Qt::AlignCenter);
    m_notesName = text;
    m_notesNumber = notesNumber;
    onFontChanged();
}

/**
 * @brief FolderSelectDialog::getSelectIndex
 * @return 选中的记事本
 */
void FolderSelectDialog::onFontChanged()
{
    QString itemInfo = "";
    //自动截断提示长度
    QFontMetrics fontMetric(this->font());
    //用于计算当前文本名截断宽度的常量
    int constantWidth = m_notesNumber > 1 ? fontMetric.width(DApplication::translate("LeftView", "Move %1 notes (%2, ...) to:").arg("").arg("\"\""))
                                          : fontMetric.width(DApplication::translate("LeftView", "Move the note \"%1\" to:").arg("  "));
    QString notesName = fontMetric.elidedText(m_notesName, Qt::ElideRight, VNOTE_SELECTDIALOG_W - 20 - constantWidth);
    QString temp = "";
    int width = fontMetric.width(DApplication::translate("LeftView", "Move the note \"%1\" to:").arg("").at(0)) / 3;
    for (int i = 0; i < 10; i++) {
        if (fontMetric.width(temp) + fontMetric.width(" ") > width) {
            break;
        }
        temp.append(" ");
    }
    if (1 == m_notesNumber) {
        itemInfo = QString(temp).append(DApplication::translate("LeftView", "Move the note \"%1\" to:").arg(notesName));
    } else {
        itemInfo = QString(temp).append(DApplication::translate("LeftView", "Move %1 notes (%2, ...) to:").arg(m_notesNumber).arg(notesName));
    }
    m_noteInfo->setText(itemInfo);
}

void FolderSelectDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Up && event->key() != Qt::Key_Down) {
        event->ignore();
    }
}

/**
 * @brief FolderSelectDialog::getSelectIndex
 * @return 选中的记事本
 */
QModelIndex FolderSelectDialog::getSelectIndex()
{
    QModelIndex index = m_view->currentIndex();
    QItemSelectionModel *model = m_view->selectionModel();
    if (!model->isSelected(index)) {
        index = QModelIndex();
    }
    return index;
}

/**
 * @brief FolderSelectDialog::setFolderBlack
 * @param folders 不需要显示的笔记项
 */
void FolderSelectDialog::setFolderBlack(const QList<VNoteFolder *> &folders)
{
    m_model->setBlackFolders(folders);
}

/**
 * @brief FolderSelectDialog::clearSelection
 */
void FolderSelectDialog::clearSelection()
{
    m_view->clearSelection();
    m_confirmBtn->setEnabled(false);
}

/**
 * @brief FolderSelectDialog::onVNoteFolderSelectChange
 * @param selected
 * @param deselected
 * 没有选中项确定按钮不可用
 */
void FolderSelectDialog::onVNoteFolderSelectChange(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    m_confirmBtn->setEnabled(!!selected.indexes().size());
}

/**
 * @brief FolderSelectDialog::hideEvent
 * @param event
 *  隐藏事件 bug 54701
 */
void FolderSelectDialog::hideEvent(QHideEvent *event)
{
    //取消按钮的hover状态
    m_closeButton->setAttribute(Qt::WA_UnderMouse, false);
    m_view->setFocus(Qt::MouseFocusReason);
    DAbstractDialog::hideEvent(event);
}


void FolderSelectDialog::focusInEvent(QFocusEvent *event)
{
    DAbstractDialog::focusInEvent(event);
    m_cancelBtn->setFocus();
}

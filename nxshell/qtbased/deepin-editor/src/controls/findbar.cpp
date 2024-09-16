// SPDX-FileCopyrightText: 2011-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findbar.h"
#include "../common/utils.h"

#include <QDebug>

// 不同布局模式下界面参数，不完全对应设计图固定值，调整后实际像素值和设计图对应
const int s_FBHeight = 60;
const QMargins s_FBContentMargins = {16, 0, 10, 0};
const int s_FBCloseBtnSize = 30;
const int s_FBHeightCompact = 40;
const QMargins s_FBContentMarginsCompact = {16, 0, 6, 0};
const int s_FBCloseBtnSizeCompact = 26;
const int s_FBCloseIconSizeCompact = 27;

FindBar::FindBar(QWidget *parent)
    : DFloatingWidget(parent)
{
    // Init.
    //setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    hide();
    setFixedHeight(60);

    // Init layout and widgets.

    m_layout = new QHBoxLayout();
    m_layout->setSpacing(10);
    m_layout->setAlignment(Qt::AlignVCenter);
    m_findLabel = new QLabel(tr("Find"));
    m_editLine = new LineBar();
    m_findPrevButton = new QPushButton(tr("Previous"));
    m_findNextButton = new QPushButton(tr("Next"));
    m_closeButton = new DIconButton(DStyle::SP_CloseButton);
    m_closeButton->setIconSize(QSize(30, 30));
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setEnabledCircle(true);
    m_closeButton->setFlat(true);
    m_layout->setContentsMargins(16, 6, 10, 6);

    m_layout->addWidget(m_findLabel);

    // 单独处理输入框，由于Qt坐标机制，奇数处理存在累计误差，导致显示效果和预期存在差异，添加 spacer 强制输入框居中
    QVBoxLayout *lineBarLayout = new QVBoxLayout();
    lineBarLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    lineBarLayout->addWidget(m_editLine);
    lineBarLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    m_layout->addLayout(lineBarLayout);

    m_layout->addWidget(m_findPrevButton);
    m_layout->addWidget(m_findNextButton);
    m_layout->addWidget(m_closeButton);
    this->setLayout(m_layout);

    // Make button don't grab keyboard focus after click it.
    // m_findNextButton->setFocusPolicy(Qt::NoFocus);
    // m_findPrevButton->setFocusPolicy(Qt::NoFocus);
    // m_closeButton->setFocusPolicy(Qt::NoFocus);

    connect(this, &FindBar::pressEsc, this, &FindBar::findCancel, Qt::QueuedConnection);
    // connect(m_editLine, &LineBar::pressEnter, this, &FindBar::findNext, Qt::QueuedConnection);            //Shielded by Hengbo ,for new demand. 20200220
    //connect(m_editLine, &LineBar::pressCtrlEnter, this, &FindBar::findPrev, Qt::QueuedConnection);
    connect(m_editLine, &LineBar::returnPressed, this, &FindBar::handleContentChanged, Qt::QueuedConnection);
    connect(m_editLine, &LineBar::signal_sentText, this, &FindBar::receiveText, Qt::QueuedConnection);
    //connect(m_editLine, &LineBar::contentChanged, this, &FindBar::slot_ifClearSearchWord, Qt::QueuedConnection);

    connect(m_findNextButton, &QPushButton::clicked,  this, &FindBar::handleFindNext, Qt::QueuedConnection);
    connect(m_findPrevButton, &QPushButton::clicked, this, &FindBar::handleFindPrev, Qt::QueuedConnection);
    //connect(m_findPrevButton, &QPushButton::clicked, this, &FindBar::findPrev, Qt::QueuedConnection);

    connect(m_closeButton, &DIconButton::clicked, this, &FindBar::findCancel, Qt::QueuedConnection);

#ifdef DTKWIDGET_CLASS_DSizeMode
    updateSizeMode();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &FindBar::updateSizeMode);
#endif
}

bool FindBar::isFocus()
{
    return m_editLine->lineEdit()->hasFocus();
}

void FindBar::focus()
{
    m_editLine->lineEdit()->setFocus();
    m_editLine->lineEdit()->selectAll();
}

void FindBar::activeInput(QString text, QString file, int row, int column, int scrollOffset)
{
    // Try fill keyword with select text.
    m_editLine->lineEdit()->clear();
    m_editLine->lineEdit()->insert(text);
    m_editLine->lineEdit()->selectAll();

    // Show.
    QWidget::show();

    // Save file info for back to position.
    m_findFile = file;
    m_findFileRow = row;
    m_findFileColumn = column;
    m_findFileSrollOffset = scrollOffset;

    // Focus.
    focus();
}

void FindBar::findCancel()
{
    QWidget::hide();
    emit sigFindbarClose();
}

void FindBar::handleContentChanged()
{
    updateSearchKeyword(m_findFile, m_editLine->lineEdit()->text());
}

void FindBar::handleFindPrev()
{
    findPrev(m_editLine->lineEdit()->text());
}

void FindBar::handleFindNext()
{
    findNext(m_editLine->lineEdit()->text());
}

void FindBar::hideEvent(QHideEvent *)
{
    //保留查询标记
    //removeSearchKeyword();
}

bool FindBar::focusNextPrevChild(bool next)
{
    return false;
}

void FindBar::keyPressEvent(QKeyEvent *e)
{
    const QString &key = Utils::getKeyshortcut(e);
    if (key == "Esc") {
        QWidget::hide();
        emit sigFindbarClose();
    }
    if (m_closeButton->hasFocus() && key == "Tab") {
        m_editLine->lineEdit()->setFocus();
    } else {
        DFloatingWidget::keyPressEvent(e);
    }
    if (key == "Enter") {
        if (m_findPrevButton->hasFocus()) {
            m_findPrevButton->click();
        }
        if (m_findNextButton->hasFocus()) {
            m_findNextButton->click();
        }
    }
}

/**
   @brief 根据界面布局模式 `DGuiApplicationHelper::isCompactMode()` 切换当前界面布局参数。
        需要注意，界面参数同设计图参数并非完全一致，而是按照实际的显示像素值进行比对。
 */
void FindBar::updateSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::isCompactMode()) {
        setFixedHeight(s_FBHeightCompact);
        m_closeButton->setFixedSize(s_FBCloseBtnSizeCompact, s_FBCloseBtnSizeCompact);
        m_closeButton->setIconSize(QSize(s_FBCloseIconSizeCompact, s_FBCloseIconSizeCompact));

        m_layout->setContentsMargins(s_FBContentMarginsCompact);
        m_layout->invalidate();
    } else {
        setFixedHeight(s_FBHeight);
        m_closeButton->setFixedSize(s_FBCloseBtnSize, s_FBCloseBtnSize);
        m_closeButton->setIconSize(QSize(s_FBCloseBtnSize, s_FBCloseBtnSize));

        m_layout->setContentsMargins(s_FBContentMargins);
        m_layout->invalidate();
    }
#endif
}

void FindBar::setMismatchAlert(bool isAlert)
{
    m_editLine->setAlert(isAlert);
}

void FindBar::receiveText(QString t)
{
    searched = false;
    if (t != "") {
        m_receivedText = t;
    }
}

void FindBar::setSearched(bool _)
{
    searched = _;
}

void FindBar::findPreClicked()
{
    if (!searched) {
        updateSearchKeyword(m_findFile, m_editLine->lineEdit()->text());
        emit findPrev(m_editLine->lineEdit()->text());
        searched = true;
    } else {
        emit findPrev(m_editLine->lineEdit()->text());
    }
}

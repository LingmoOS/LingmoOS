// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbar.h"

#include "application.h"
#include "main_window.h"
#include "common/common.h"
#include "common/perf.h"

#include <DButtonBox>
#include <DSearchEdit>

#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTimer>

#define MAX_TEXT_LEN 200

using namespace common;
using namespace common::init;

// constructor
Toolbar::Toolbar(QWidget *parent)
    : DWidget(parent)
{
    // install event filter for toolbar
    installEventFilter(this);

    // toolbar layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // tab button group
    m_switchFuncTabBtnGrp = new CustomButtonBox(this);
    m_switchFuncTabBtnGrp->setFixedWidth(240);
    // process tab button instance
    m_procBtn = new DButtonBoxButton(
        DApplication::translate("Title.Bar.Switch", "Processes"), m_switchFuncTabBtnGrp);
    m_procBtn->setCheckable(true);
    m_procBtn->setChecked(true);
    m_procBtn->setFocusPolicy(Qt::TabFocus);

    DFontSizeManager::instance()->bind(m_procBtn, DFontSizeManager::T7, QFont::Medium);
    // service tab button instance
    m_svcBtn = new DButtonBoxButton(
        DApplication::translate("Title.Bar.Switch", "Services"), m_switchFuncTabBtnGrp);
    m_svcBtn->setCheckable(true);
    m_svcBtn->setFocusPolicy(Qt::TabFocus);

    DFontSizeManager::instance()->bind(m_svcBtn, DFontSizeManager::T7, QFont::Medium);
    // user process tab button instance
    m_accountProcBtn = new DButtonBoxButton(
        DApplication::translate("Title.Bar.Switch", "Users"), m_switchFuncTabBtnGrp);
    m_accountProcBtn->setCheckable(true);
    m_accountProcBtn->setFocusPolicy(Qt::TabFocus);

    DFontSizeManager::instance()->bind(m_accountProcBtn, DFontSizeManager::T7, QFont::Medium);
    QList<DButtonBoxButton *> list;
    list << m_procBtn << m_svcBtn << m_accountProcBtn;
    m_switchFuncTabBtnGrp->setButtonList(list, true);

    // move focus to process tab button when toolbar got focus
    setFocusProxy(m_switchFuncTabBtnGrp);

    // install event filer for both tab button to handle key press event
    m_procBtn->installEventFilter(this);
    m_svcBtn->installEventFilter(this);
    m_accountProcBtn->installEventFilter(this);

    // emit button clicked signal when process or service tab button toggled
    connect(m_procBtn, &DButtonBoxButton::toggled, this, [ = ](bool) { Q_EMIT procTabButtonClicked(); });
    connect(m_svcBtn, &DButtonBoxButton::toggled, this, [ = ](bool) { Q_EMIT serviceTabButtonClicked(); });
    connect(m_accountProcBtn, &DButtonBoxButton::toggled, this, [ = ](bool) { Q_EMIT accountProcTabButtonClicked(); });
    // search text editor instance
    searchEdit = new DSearchEdit(this);
    // set the search edit text max length
    searchEdit->lineEdit()->setMaxLength(MAX_TEXT_LEN);
    searchEdit->setFixedWidth(360);
    searchEdit->setPlaceHolder(DApplication::translate("Title.Bar.Search", "Search"));

    // add widgets into layout
    layout->addWidget(m_switchFuncTabBtnGrp, 0, Qt::AlignLeft);
    layout->addStretch();
    layout->addWidget(searchEdit, 0, Qt::AlignHCenter);
    layout->addStretch();


    // timer instance, handle search process only after timed out
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    connect(searchTimer, &QTimer::timeout, this, &Toolbar::handleSearch);

    // handle search input text changed event
    connect(searchEdit, &DSearchEdit::textChanged, this, &Toolbar::handleSearchTextChanged);

    // monitoring loading status changed signal, change button/input enabled/disabled state accordingly
    connect(gApp->mainWindow(), &MainWindow::loadingStatusChanged, this, [ = ](bool loading) {
        if (loading) {
            m_procBtn->setEnabled(false);
            m_svcBtn->setEnabled(false);
            m_accountProcBtn->setEnabled(false);
            searchEdit->setEnabled(false);
        } else {
            m_procBtn->setEnabled(true);
            m_svcBtn->setEnabled(true);
            m_accountProcBtn->setEnabled(true);
            searchEdit->setEnabled(true);
        }
    });
    m_switchFuncTabBtnGrp->installEventFilter(this);
}

// destructor
Toolbar::~Toolbar() {}

// event filter
bool Toolbar::eventFilter(QObject *obj, QEvent *event)
{
    // handle key press event
    if (event->type() == QEvent::KeyPress) {
        if (obj == this) {
            // emit press esc signal when ESC pressed inside toolbar itself
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                searchEdit->clearEdit();
                pressEsc();
            }
        } else if (obj == searchEdit) {
            // emit press tab signal when tab pressed inside search input
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Tab) {
                pressTab();
            }
        } else if (obj == m_procBtn) {
            // set focus to service tab button when right key pressed
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if (kev->key() == Qt::Key_Right) {
                m_svcBtn->setFocus();
                return true;
            }
        } else if (obj == m_svcBtn) {
            // set focus to process tab button when left key pressed
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if (kev->key() == Qt::Key_Left) {
                m_procBtn->setFocus();
                return true;
            }
        } else if (obj == m_accountProcBtn) {
            // set focus to process tab button when left key pressed
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if (kev->key() == Qt::Key_Left) {
                m_accountProcBtn->setFocus();
                return true;
            }
        }
    }  

    // propogate other events to base handler
    return DWidget::eventFilter(obj, event);
}

// emit search signal only after timed out, and current search input's text same as cached one
void Toolbar::handleSearch()
{
    if (searchEdit->text() == searchTextCache) {
        PERF_PRINT_BEGIN("POINT-03", QString("search(%1)").arg(searchTextCache));
        search(searchTextCache);
    }
}

// start timer when search content changed & cache current text
void Toolbar::handleSearchTextChanged()
{
    searchTextCache = searchEdit->text();

    if (searchTimer->isActive()) {
        searchTimer->stop();
    }
    searchTimer->start(50);
}

// set focus to search input
void Toolbar::focusInput()
{
    if (searchEdit->text() != "") {
        searchEdit->lineEdit()->selectAll();
        searchEdit->lineEdit()->setFocus();
    } else {
        searchEdit->lineEdit()->setFocus();
    }
}

// check if search input is empty
bool Toolbar::isSearchContentEmpty()
{
    if (searchEdit->text().isEmpty()) {
        return false;
    }
    return true;
}

void Toolbar::setProcessButtonChecked(bool checked)
{
    if (checked) {
        m_procBtn->setChecked(true);
    }
}

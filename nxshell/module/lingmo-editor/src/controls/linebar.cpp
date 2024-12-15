// SPDX-FileCopyrightText: 2011-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "linebar.h"
#include "../common/utils.h"

#include <DGuiApplicationHelper>

#include <QDebug>

// 不同布局模式(紧凑)
const int s_nLineBarHeight = 36;
const int s_nLineBarHeightCompact = 24;

LineBar::LineBar(DLineEdit *parent)
    : DLineEdit(parent)
{
    // Init.
    setClearButtonEnabled(true);

    m_autoSaveInternal = 50;
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setSingleShot(true);

    connect(m_autoSaveTimer, &QTimer::timeout, this, &LineBar::handleTextChangeTimer);
    connect(this, &DLineEdit::textEdited, this, &LineBar::sendText, Qt::QueuedConnection);
    connect(this, &DLineEdit::textChanged, this, &LineBar::handleTextChanged, Qt::QueuedConnection);

#ifdef DTKWIDGET_CLASS_DSizeMode
    setFixedHeight(DGuiApplicationHelper::isCompactMode() ? s_nLineBarHeightCompact : s_nLineBarHeight);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this](){
        setFixedHeight(DGuiApplicationHelper::isCompactMode() ? s_nLineBarHeightCompact : s_nLineBarHeight);
    });
#endif
}

void LineBar::handleTextChangeTimer()
{
    // Emit contentChanged signal.
    contentChanged();
}

void LineBar::handleTextChanged(const QString &str)
{
    // Stop timer if new character is typed, avoid unused timer run.
    if (m_autoSaveTimer->isActive()) {
        m_autoSaveTimer->stop();
    }
    if(str.isEmpty()) {
        setAlert(false);
    }
    // Start new timer.
    m_autoSaveTimer->start(m_autoSaveInternal);
}

void LineBar::sendText(QString t)
{
    emit signal_sentText(t);
}

void LineBar::focusOutEvent(QFocusEvent *e)
{
    // Emit focus out signal.
    focusOut();

    // Throw event out avoid DLineEdit can't hide cursor after lost focus.
    DLineEdit::focusOutEvent(e);
}

void LineBar::keyPressEvent(QKeyEvent *e)
{
    QString key = Utils::getKeyshortcut(e);
    Qt::KeyboardModifiers modifiers = e->modifiers();

    if(modifiers == Qt::ControlModifier && e->text() == "\r"){
       pressCtrlEnter();
    }else if(modifiers == Qt::AltModifier && e->text() == "\r"){
       pressAltEnter();
    }else if(modifiers == Qt::MetaModifier && e->text() == "\r"){
       pressMetaEnter();
    }else if(modifiers == Qt::NoModifier && e->text() == "\r"){
       pressEnter();
    }else {
      // Pass event to DLineEdit continue, otherwise you can't type anything after here. ;)
       DLineEdit::keyPressEvent(e);
    }
}

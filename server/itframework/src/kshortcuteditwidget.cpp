/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-xmlgui.h"
#include "kshortcutsdialog_p.h"

#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QRadioButton>

#include <KLocalizedString>
#if HAVE_GLOBALACCEL
#include <KGlobalAccel>
#endif

#include "kkeysequencewidget.h"

void TabConnectedWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter p(this);
    QPen pen(QPalette().highlight().color());
    pen.setWidth(6);
    p.setPen(pen);
    p.drawLine(0, 0, width(), 0);
    if (qApp->isLeftToRight()) {
        p.drawLine(0, 0, 0, height());
    } else {
        p.drawLine(width(), 0, width(), height());
    }
}

ShortcutEditWidget::ShortcutEditWidget(QWidget *viewport, const QKeySequence &defaultSeq, const QKeySequence &activeSeq, bool allowLetterShortcuts)
    : TabConnectedWidget(viewport)
    , m_defaultKeySequence(defaultSeq)
    , m_isUpdating(false)
    , m_action(nullptr)
    , m_noneText(i18nc("No shortcut defined", "None"))
{
    QGridLayout *layout = new QGridLayout(this);

    m_defaultRadio = new QRadioButton(i18nc("@option:radio", "Default:"), this);
    m_defaultLabel = new QLabel(m_noneText, this);
    const QString defaultText = defaultSeq.toString(QKeySequence::NativeText);
    if (!defaultText.isEmpty()) {
        m_defaultLabel->setText(defaultText);
    }

    m_customRadio = new QRadioButton(i18nc("@option:radio", "Custom:"), this);
    m_customEditor = new KKeySequenceWidget(this);
    m_customEditor->setModifierlessAllowed(allowLetterShortcuts);

    layout->addWidget(m_defaultRadio, 0, 0);
    layout->addWidget(m_defaultLabel, 0, 1);
    layout->addWidget(m_customRadio, 1, 0);
    layout->addWidget(m_customEditor, 1, 1);
    layout->setColumnStretch(2, 1);

    setKeySequence(activeSeq);

    connect(m_defaultRadio, &QRadioButton::toggled, this, &ShortcutEditWidget::defaultToggled);
    connect(m_customEditor, &KKeySequenceWidget::keySequenceChanged, this, &ShortcutEditWidget::setCustom);
    connect(m_customEditor, &KKeySequenceWidget::stealShortcut, this, &ShortcutEditWidget::stealShortcut);
#if HAVE_GLOBALACCEL
    connect(KGlobalAccel::self(), &KGlobalAccel::globalShortcutChanged, this, [this](QAction *action, const QKeySequence &seq) {
        if (action != m_action) {
            return;
        }
        setKeySequence(seq);
    });
#endif
}

KKeySequenceWidget::ShortcutTypes ShortcutEditWidget::checkForConflictsAgainst() const
{
    return m_customEditor->checkForConflictsAgainst();
}

// slot
void ShortcutEditWidget::defaultToggled(bool checked)
{
    if (m_isUpdating) {
        return;
    }

    m_isUpdating = true;
    if (checked) {
        // The default key sequence should be activated. We check first if this is
        // possible.
        if (m_customEditor->isKeySequenceAvailable(m_defaultKeySequence)) {
            // Clear the customs widget
            m_customEditor->clearKeySequence();
            Q_EMIT keySequenceChanged(m_defaultKeySequence);
        } else {
            // We tried to switch to the default key sequence and failed. Go
            // back.
            m_customRadio->setChecked(true);
        }
    } else {
        // The empty key sequence is always valid
        Q_EMIT keySequenceChanged(QKeySequence());
    }
    m_isUpdating = false;
}

void ShortcutEditWidget::setCheckActionCollections(const QList<KActionCollection *> &checkActionCollections)
{
    // We just forward them to out KKeySequenceWidget.
    m_customEditor->setCheckActionCollections(checkActionCollections);
}

void ShortcutEditWidget::setCheckForConflictsAgainst(KKeySequenceWidget::ShortcutTypes types)
{
    m_customEditor->setCheckForConflictsAgainst(types);
}

void ShortcutEditWidget::setComponentName(const QString &componentName)
{
    m_customEditor->setComponentName(componentName);
}

void ShortcutEditWidget::setMultiKeyShortcutsAllowed(bool allowed)
{
    // We just forward them to out KKeySequenceWidget.
    m_customEditor->setMultiKeyShortcutsAllowed(allowed);
}

bool ShortcutEditWidget::multiKeyShortcutsAllowed() const
{
    return m_customEditor->multiKeyShortcutsAllowed();
}

void ShortcutEditWidget::setAction(QObject *action)
{
    m_action = action;
}

// slot
void ShortcutEditWidget::setCustom(const QKeySequence &seq)
{
    if (m_isUpdating) {
        return;
    }

    // seq is a const reference to a private variable of KKeySequenceWidget.
    // Somewhere below we possible change that one. But we want to emit seq
    // whatever happens. So we make a copy.
    QKeySequence original = seq;

    m_isUpdating = true;

    // Check if the user typed in the default sequence into the custom field.
    // We do this by calling setKeySequence which will do the right thing.
    setKeySequence(original);

    Q_EMIT keySequenceChanged(original);
    m_isUpdating = false;
}

void ShortcutEditWidget::setKeySequence(const QKeySequence &activeSeq)
{
    const QString seqString = activeSeq.isEmpty() ? m_noneText : activeSeq.toString(QKeySequence::NativeText);
    if (seqString == m_defaultLabel->text()) {
        m_defaultRadio->setChecked(true);
        m_customEditor->clearKeySequence();
    } else {
        m_customRadio->setChecked(true);
        // m_customEditor->setKeySequence does some stuff we only want to
        // execute when the sequence really changes.
        if (activeSeq != m_customEditor->keySequence()) {
            m_customEditor->setKeySequence(activeSeq);
        }
    }
}

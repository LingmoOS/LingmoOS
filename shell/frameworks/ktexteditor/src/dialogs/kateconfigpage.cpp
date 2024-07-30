/*
    SPDX-FileCopyrightText: 2019 KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateconfigpage.h"

#include <KFontRequester>
#include <KUrlRequester>

#include <QAbstractButton>
#include <QAbstractSlider>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>

KateConfigPage::KateConfigPage(QWidget *parent, const char *)
    : KTextEditor::ConfigPage(parent)
{
    connect(this, &KateConfigPage::changed, this, &KateConfigPage::somethingHasChanged);
}

KateConfigPage::~KateConfigPage()
{
}

void KateConfigPage::observeChanges(KateConfigPage *page) const
{
    connect(page, &KateConfigPage::changed, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::observeChanges(KUrlRequester *requester) const
{
    connect(requester, &KUrlRequester::textChanged, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::observeChanges(QAbstractButton *button) const
{
    connect(button, &QAbstractButton::toggled, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::observeChanges(QAbstractSlider *slider) const
{
    connect(slider, &QAbstractSlider::valueChanged, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::observeChanges(QSpinBox *spinBox) const
{
    connect(spinBox, &QSpinBox::textChanged, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::observeChanges(QDoubleSpinBox *spinBox) const
{
    connect(spinBox, &QDoubleSpinBox::textChanged, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::observeChanges(QComboBox *comboBox) const
{
    connect(comboBox, &QComboBox::currentIndexChanged, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::observeChanges(QGroupBox *groupBox) const
{
    connect(groupBox, &QGroupBox::toggled, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::observeChanges(QLineEdit *lineEdit) const
{
    connect(lineEdit, &QLineEdit::textChanged, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::observeChanges(KFontRequester *chooser) const
{
    connect(chooser, &KFontRequester::fontSelected, this, &KateConfigPage::slotChanged);
}

void KateConfigPage::slotChanged()
{
    Q_EMIT changed();
}

void KateConfigPage::somethingHasChanged()
{
    m_changed = true;
}

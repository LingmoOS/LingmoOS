// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingshead.h"
#include "labels/normallabel.h"

#include <DCommandLinkButton>

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QGSettings>
#include <QApplication>
#include <DFontSizeManager>
#include "window/settingsdef.h"
#include "window/gsettingwatcher.h"

using namespace dcc_fcitx_configtool::widgets;
DWIDGET_USE_NAMESPACE

FcitxSettingsHead::FcitxSettingsHead(QFrame *parent)
    : FcitxSettingsItem(parent)
    , m_title(new FcitxTitleLabel)
    , m_edit(new DCommandLinkButton(""))
    , m_state(Cancel)
{
    m_title->setObjectName("SettingsHeadTitle");

    // can not translate correctly just using tr()
    m_edit->setText(qApp->translate("FcitxSettingsHead", "Edit"));
    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T5, QFont::DemiBold);
    DFontSizeManager::instance()->bind(m_edit, DFontSizeManager::T8, QFont::Normal);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 10, 0);
    mainLayout->addWidget(m_title);
    mainLayout->addStretch();
    mainLayout->addWidget(m_edit);

    setLayout(mainLayout);

    connect(m_edit, &DCommandLinkButton::clicked, this, &FcitxSettingsHead::onClicked);
    m_gsetting = new QGSettings("com.deepin.fcitx-config", QByteArray(), this);
    connect(m_gsetting, &QGSettings::changed, this, &FcitxSettingsHead::onStatusModeChanged);
}

FcitxSettingsHead::~FcitxSettingsHead()
{
    delete m_gsetting;
}

void FcitxSettingsHead::setTitle(const QString &title)
{
    m_title->setText(title);
    m_edit->setAccessibleName(title);
}

void FcitxSettingsHead::setEditEnable(bool state)
{
    QString value = m_gsetting->get(GSETTINGS_EDIT).toString();
    m_edit->setVisible(state & "Hidden" != value);
    m_editVisible = state;
}

void FcitxSettingsHead::toEdit()
{
    m_state = Edit;
    refershButton();

    Q_EMIT editChanged(true);
}

void FcitxSettingsHead::toCancel()
{
    m_state = Cancel;
    refershButton();

    Q_EMIT editChanged(false);
}

void FcitxSettingsHead::setEdit(bool flag)
{
    if (flag) {
        m_state = Edit;
        refershButton();
    } else {
        m_state = Cancel;
        refershButton();
    }
}

void FcitxSettingsHead::onClicked()
{
    if (m_state == Cancel) {
        toEdit();
    } else {
        toCancel();
    }
}

void FcitxSettingsHead::onStatusModeChanged(const QString &key)
{
    QString value = m_gsetting->get(GSETTINGS_EDIT).toString();
    if (key == GSETTINGS_EDIT) {
        m_edit->setVisible(m_editVisible & "Hidden" != value);
        m_edit->setEnabled("Enabled" == value);
    }
}

void FcitxSettingsHead::refershButton()
{
    if (m_state == Cancel) {
        m_edit->setText(qApp->translate("FcitxSettingsHead", "Edit"));
    } else {
        m_edit->setText(qApp->translate("FcitxSettingsHead", "Done"));
    }
}

FcitxTitleLabel* FcitxSettingsHead::getTitleLabel()
{
    return m_title;
}

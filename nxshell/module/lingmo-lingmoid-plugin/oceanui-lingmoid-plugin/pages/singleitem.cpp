// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "singleitem.h"

#include <DCommandLinkButton>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QHBoxLayout>
DWIDGET_USE_NAMESPACE

SingleItem::SingleItem(QWidget *parent):QWidget (parent)
  , m_labelIcon(new QLabel)
  , m_labelText(new QLabel)
  , m_btnLink(new DCommandLinkButton(""))
{
    InitUI();
    InitConnection();
}

void SingleItem::SetIcon(const QString &iconName)
{
    m_labelIcon->setPixmap(QIcon::fromTheme(iconName).pixmap(QSize(16, 16)));
    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::themeTypeChanged,
        this, [=] {
        m_labelIcon->setPixmap(QIcon::fromTheme(iconName).pixmap(16, 16));
    }, Qt::UniqueConnection);
}

void SingleItem::SetText(const QString &text)
{
    m_labelText->setText(text);
}

void SingleItem::SetLinkText(const QString &btntext)
{
    m_btnLink->setText(btntext);
}

void SingleItem::InitUI()
{
    setFixedHeight(36);
    QHBoxLayout *mainlayout = new QHBoxLayout(this);
    mainlayout->setContentsMargins(10, 0, 10, 0);
    mainlayout->setSpacing(0);
    mainlayout->addWidget(m_labelIcon);
    mainlayout->addSpacing(10);
    mainlayout->addWidget(m_labelText);
    mainlayout->addStretch();
    mainlayout->addWidget(m_btnLink);
    DFontSizeManager::instance()->bind(m_btnLink, DFontSizeManager::T7);
}

void SingleItem::InitConnection()
{
    connect(m_btnLink, &QAbstractButton::clicked, this, &SingleItem::clicked);
}

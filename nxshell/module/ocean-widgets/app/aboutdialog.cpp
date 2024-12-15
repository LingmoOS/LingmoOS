// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aboutdialog.h"
#include <DLabel>
#include <DTipLabel>
#include <DFontSizeManager>
#include <QCoreApplication>
#include <QVBoxLayout>

WIDGETS_FRAME_BEGIN_NAMESPACE

DWIDGET_USE_NAMESPACE
InstanceAboutDialog::InstanceAboutDialog(QWidget *parent)
    : DDialog(parent)
{
    setMinimumSize(UI::About::dialogSize);
    setContentLayoutContentsMargins(UI::About::margins);

    m_logo = new DLabel();
    addContent(m_logo, Qt::AlignHCenter);

    m_title = new DLabel();
    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T8, QFont::Bold);
    addSpacing(UI::About::titleSpacing);
    addContent(m_title, Qt::AlignHCenter);

    m_version = new DLabel();
    addSpacing(UI::About::versionSpacing);
    addContent(m_version, Qt::AlignHCenter);

    m_contributor = new DLabel();
    addSpacing(UI::About::contributorSpacing);
    addContent(m_contributor, Qt::AlignHCenter);

    m_description = new DTipLabel();
    m_description->DLabel::setForegroundRole(QPalette::WindowText);
    m_description->setWordWrap(true);
    m_description->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    DFontSizeManager::instance()->bind(m_description, DFontSizeManager::T9);
    addSpacing(UI::About::descriptionSpacing);
    addContent(m_description, Qt::AlignHCenter);
}

void InstanceAboutDialog::setLogo(const QIcon &logo)
{
    m_logo->setPixmap(logo.pixmap(UI::About::logoSize));
}

void InstanceAboutDialog::setVersion(const QString &version)
{
    m_version->setText(qApp->translate("InstanceAboutDialog", "version: %1").arg(version));
}

void InstanceAboutDialog::setTitle(const QString &title)
{
    m_title->setText(title);
}

void InstanceAboutDialog::setDescription(const QString &description)
{
    m_description->setText(description);
}

void InstanceAboutDialog::setContributor(const QString &contributor)
{
    m_contributor->setText(qApp->translate("InstanceAboutDialog", "contributor: %1").arg(contributor));
}

WIDGETS_FRAME_END_NAMESPACE

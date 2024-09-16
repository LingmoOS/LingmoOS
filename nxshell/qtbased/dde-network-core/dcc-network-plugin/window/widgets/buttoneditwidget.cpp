// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "widgets/buttoneditwidget.h"
#include "widgets/lineeditwidget.h"

#include <dpasswordedit.h>
#include <QVBoxLayout>
#include <DLineEdit>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

namespace dcc {
namespace network {

ErrorTip::ErrorTip(QWidget *parent) :
    DArrowRectangle(DArrowRectangle::ArrowTop, parent),
    m_label(new QLabel)
{
    m_label->setObjectName("New_Account_errorTip");
    m_label->setAccessibleName(m_label->objectName());
    m_label->setStyleSheet("padding: 5px 10px; color: #f9704f");
    m_label->setWordWrap(true);
    setContent(m_label);
}

void ErrorTip::setText(QString text)
{
    m_label->setText(text);
    m_label->setAccessibleDescription(text);
    m_label->adjustSize();
    resizeWithContent();
}

void ErrorTip::clear()
{
    m_label->clear();
    hide();
}

bool ErrorTip::isEmpty() const
{
    return m_label->text().isEmpty();
}

void ErrorTip::appearIfNotEmpty()
{
    if (!isEmpty() && !isVisible())
        QWidget::show();
}

ButtonEditWidget::ButtonEditWidget(QFrame *parent)
    : SettingsItem (parent)
    , m_title(new QLabel)
    , m_errTip(new ErrorTip(this))
    , m_edit(new DLineEdit)
    , m_addBtn(new DIconButton(DStyle::SP_AddButton, this))
    , m_reduceBtn(new DIconButton(DStyle::SP_DeleteButton, this))
{
    m_title->setFixedWidth(110);
    m_edit->setContextMenuPolicy(Qt::NoContextMenu);

    m_addBtn->setFixedSize(QSize(19, 19));
    m_addBtn->setIconSize(QSize(17, 17));

    m_reduceBtn->setFixedSize(QSize(19, 19));
    m_reduceBtn->setIconSize(QSize(17, 17));

    m_mainLayout = new QHBoxLayout;

    m_mainLayout->addWidget(m_title, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_edit, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_addBtn, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_reduceBtn, 0, Qt::AlignVCenter);

    setLayout(m_mainLayout);
    setObjectName("ButtonEditWidget");
    initConnect();
}

ButtonEditWidget::~ButtonEditWidget()
{

}

void ButtonEditWidget::initConnect()
{
    connect(m_addBtn, &DIconButton::clicked, this, &ButtonEditWidget::addNewDnsEdit);
    connect(m_reduceBtn, &DIconButton::clicked, this, &ButtonEditWidget::deleteCurrentDnsEdit);
    connect(m_edit, &DLineEdit::textChanged, this, &ButtonEditWidget::hideAlertMessage);
}

void ButtonEditWidget::hideIconBtn()
{
    m_addBtn->hide();
    m_reduceBtn->hide();
}

void ButtonEditWidget::showIconBtn()
{
    m_addBtn->show();
    m_reduceBtn->show();
}

void ButtonEditWidget::hideAlertMessage()
{
    m_edit->setAlert(false);
    setIsErr(false);
    m_errTip->hide();
}

void ButtonEditWidget::setTitle(const QString &title)
{
    m_title->setText(title);
    m_title->setWordWrap(true);

    setAccessibleName(title);
    m_edit->setAccessibleName(title);
}

void ButtonEditWidget::setText(const QString &text)
{
    m_edit->setText(text);
}

} // namespace ButtonEditWidget
} // namespase dcc

// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editlabel.h"

#include <QDebug>
#include <QKeyEvent>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

#define DOTSWIDTH 80

EditLabel::EditLabel(QWidget *parent)
    : QWidget(parent)
    , m_lineEdit(new QLineEdit(this))
    , m_label(new QLabel(this))
    , m_maxWidth(0)
{
    initUi();
    initConnection();
    initAccessibleName();
}

EditLabel::~EditLabel()
{
}

void EditLabel::initUi()
{
#ifdef QT_DEBUG
    setStyleSheet("QWidget{border: 1px solid red;}");
#endif

    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QColor labelColor(Qt::white);
    QPalette palette(m_label->palette());
    palette.setColor(QPalette::WindowText, labelColor);
    m_label->setPalette(palette);

    QColor editColor(Qt::transparent);
    QPalette editPalette(m_lineEdit->palette());
    editPalette.setColor(QPalette::Base, editColor);
    m_lineEdit->setPalette(editPalette);

    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_lineEdit->setFocusPolicy(Qt::ClickFocus);
    m_lineEdit->hide();
    m_lineEdit->setGeometry(m_label->geometry());

    m_lineEdit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_label->setFixedHeight(50);
    m_lineEdit->setFixedHeight(50);
    m_lineEdit->setTextMargins(5, 0, 5, 0);

    DFontSizeManager::instance()->bind(m_label, DFontSizeManager::T1);
    DFontSizeManager::instance()->bind(m_lineEdit, DFontSizeManager::T1); // 40pixel

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(QMargins(40, 40, 40, 40));
    hBoxLayout->setSpacing(0);
    hBoxLayout->addStretch();
    hBoxLayout->addWidget(m_label);
    hBoxLayout->addWidget(m_lineEdit);
    hBoxLayout->addStretch();
    setLayout(hBoxLayout);
}

void EditLabel::initConnection()
{
    connect(m_lineEdit, &QLineEdit::returnPressed, this, &EditLabel::onReturnPressed);
}

void EditLabel::initAccessibleName()
{
    m_label->setAccessibleName("drawerTitle");
    m_lineEdit->setAccessibleName("drawerLineEdit");
}

void EditLabel::cancelEditState()
{
    m_lineEdit->clearFocus();
    m_lineEdit->hide();
    m_label->show();
}

void EditLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    m_oldTitle = m_label->text();
    m_lineEdit->setFixedSize(m_label->geometry().size() + QSize(40, 0));
    m_lineEdit->show();
    m_label->hide();
    m_lineEdit->setText(m_originTitle);
    m_lineEdit->setFocus();
}

void EditLabel::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        cancelEditState();
}

void EditLabel::onReturnPressed()
{
    if (m_lineEdit->text().isEmpty()) {
        m_label->setText(m_oldTitle);
        cancelEditState();
        return;
    }

    QFontMetrics metrics(m_label->font());
    m_originTitle = m_lineEdit->text();
    const QString elideText = metrics.elidedText(m_originTitle, Qt::ElideRight, m_maxWidth - DOTSWIDTH);
    m_label->setText(elideText);
    cancelEditState();

    emit titleChanged();
}

void EditLabel::setText(int maxWidth, const QString &title)
{
    m_maxWidth = maxWidth;
    m_originTitle = title;
    QFontMetrics metrics(m_label->font());
    const QString &elideText = metrics.elidedText(title, Qt::ElideRight, m_maxWidth - DOTSWIDTH);
    m_label->setText(elideText);
}

QString EditLabel::text() const
{
    return m_originTitle;
}

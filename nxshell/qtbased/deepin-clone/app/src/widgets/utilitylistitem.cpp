// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "utilitylistitem.h"

#include <DHorizontalLine>

#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class RightContentLabel : public QWidget
{
public:
    explicit RightContentLabel(const QString &title, const QString &message, QWidget *parent = 0);
    explicit RightContentLabel(QWidget *parent = 0);

    void setTitle(const QString &text);

private:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void updateTitle();

    QLabel *titleLabel;
    QLabel *messageLabel;
    QString titleText;

    friend class UtilityListItem;
};

RightContentLabel::RightContentLabel(const QString &title, const QString &message, QWidget *parent)
    : RightContentLabel(parent)
{
    titleLabel->setText(title);
    messageLabel->setText(message);
}

RightContentLabel::RightContentLabel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    titleLabel = new QLabel(this);
    titleLabel->setObjectName("RightContentLabel_Title");

    messageLabel = new QLabel(this);
    messageLabel->setObjectName("RightContentLabel_Message");
    messageLabel->setWordWrap(true);
    messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout->addWidget(titleLabel);
    layout->addWidget(messageLabel);
}

void RightContentLabel::setTitle(const QString &text)
{
    titleText = text;
    updateTitle();
}

void RightContentLabel::resizeEvent(QResizeEvent *e)
{
    updateTitle();

    return QWidget::resizeEvent(e);
}

void RightContentLabel::updateTitle()
{
    QFontMetrics font_metrics = titleLabel->fontMetrics();

    titleLabel->setText(font_metrics.elidedText(titleText, Qt::ElideMiddle, titleLabel->width()));
}

UtilityListItem::UtilityListItem(QWidget *parent)
    : QWidget(parent)
{
    m_icon = new QLabel(this);

    QHBoxLayout *main_layout = new QHBoxLayout(this);
    QVBoxLayout *content_layout = new QVBoxLayout();

    m_layout = new QHBoxLayout();
    m_label = new RightContentLabel(this);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_layout->addWidget(m_label, 0, Qt::AlignVCenter);

    m_spaceSeparator = new QWidget(this);
    m_bottomSeparator = new DHorizontalLine(this);

    m_spaceSeparator->setFixedHeight(2);

    main_layout->setContentsMargins(10, 0, 10, 0);
    main_layout->addWidget(m_icon);
    main_layout->addLayout(content_layout);

    content_layout->addWidget(m_spaceSeparator);
    content_layout->addLayout(m_layout);
    content_layout->addWidget(m_bottomSeparator, 0, Qt::AlignBottom);
}

void UtilityListItem::setTitle(const QString &title)
{
    m_label->setTitle(title);
}

void UtilityListItem::setMessage(const QString &message)
{
    m_label->messageLabel->setText(message);
}

QString UtilityListItem::title() const
{
    return m_label->titleLabel->text();
}

void UtilityListItem::setIcon(const QIcon &icon, const QSize &size)
{
    m_iconFile = icon;

    if (size.isValid())
        m_icon->setPixmap(icon.pixmap(size));
    else
        m_icon->setPixmap(icon.pixmap(height()));
}

QIcon UtilityListItem::icon() const
{
    return m_iconFile;
}

void UtilityListItem::addWidget(QWidget *w, int stretch, Qt::Alignment alignment)
{
    m_layout->addWidget(w, stretch, alignment);
}

void UtilityListItem::enterEvent(QEvent *event)
{
    m_spaceSeparator->hide();
    m_bottomSeparator->hide();

    return QWidget::enterEvent(event);
}

void UtilityListItem::leaveEvent(QEvent *event)
{
    m_spaceSeparator->show();
    m_bottomSeparator->show();

    return QWidget::leaveEvent(event);
}

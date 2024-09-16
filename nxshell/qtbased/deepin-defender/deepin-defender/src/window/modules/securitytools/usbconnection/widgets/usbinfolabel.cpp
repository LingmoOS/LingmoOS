// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbinfolabel.h"
#include "../../deepin-defender/src/window/modules/common/common.h"

#include <QVBoxLayout>

UsbInfoLabel::UsbInfoLabel(const QString &text, QWidget *parent)
    : DTipLabel(text, parent)
    , m_originText(text)
{
    setWordWrap(true);
    setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    installEventFilter(this);
}

UsbInfoLabel::~UsbInfoLabel()
{
}

void UsbInfoLabel::setOriginText(QString text)
{
    m_originText = text;
}

void UsbInfoLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    QString displayText = Utils::wordwrapText(QFontMetrics(font()), m_originText, width());
    setText(displayText);
}

bool UsbInfoLabel::eventFilter(QObject *obj, QEvent *event)
{
    if (this == obj && QEvent::Type::FontChange == event->type()) {
        QString displayText = Utils::wordwrapText(QFontMetrics(font()), m_originText, width());
        setText(displayText);
    }
    return QWidget::eventFilter(obj, event); //其他事件按照原来处理
}

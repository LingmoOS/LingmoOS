// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "buttonwidget.h"
#include <DWarningButton>
#include <DSuggestButton>
#include <DVerticalLine>
#include <DStyleHelper>

buttonwidget::buttonwidget(QWidget *parent)
    : DWidget(parent)
    , buttonLayout(new QHBoxLayout())
    , clickedButtonIndex(0)
{
    buttonLayout->setMargin(0);
    buttonLayout->setContentsMargins(BTWidget::BUTTON_LAYOUT_LEFT_MARGIN,
                                     BTWidget::BUTTON_LAYOUT_TOP_MARGIN,
                                     BTWidget::BUTTON_LAYOUT_RIGHT_MARGIN,
                                     BTWidget::BUTTON_LAYOUT_BOTTOM_MARGIN);
    setLayout(buttonLayout);
}

int buttonwidget::addbutton(const QString &text, bool isDefault, buttonwidget::ButtonType type)
{
    int index = buttonCount();
    insertButton(index, text, isDefault, type);
    return index;
}

void buttonwidget::insertButton(int index, const QString &text, bool isDefault, buttonwidget::ButtonType type)
{
    QAbstractButton *button;
    switch (type) {
    case ButtonWarning:
        button = new DWarningButton(this);
        break;
    case ButtonRecommend:
        button = new DSuggestButton(this);
        break;
    default:
        button = new DPushButton(this);
        break;
    }
    button->setText(text);
    button->setObjectName("ActionButton");
    insertButton(index, button, isDefault);
}

void buttonwidget::insertButton(int index, QAbstractButton *button, bool isDefault)
{
    DVerticalLine *line = new DVerticalLine(this);
    line->setObjectName("VLine");
    line->setFixedHeight(30);

    buttonLayout->insertWidget(index * 2, line);
    buttonLayout->insertWidget(index * 2 + 1, button);
    buttonList << button;

    line->show();
    buttonLayout->itemAt(0)->widget()->hide();

    connect(button, &QAbstractButton::clicked, this, &buttonwidget::onButtonClicked);

    if (isDefault) {
        QPushButton *pushbutton = qobject_cast<QPushButton *>(button);
        pushbutton->setDefault(pushbutton);
    }

    const QString &text = button->text();

    if (text.count() == 2) {
        for (const QChar &ch : text) {
            switch (ch.script()) {
            case QChar::Script_Han:
            case QChar::Script_Katakana:
            case QChar::Script_Hiragana:
            case QChar::Script_Hangul:
                break;
            default:
                return;
            }
        }
        button->setText(QString().append(text.at(0)).append(QChar::Nbsp).append(text.at(1)));
    }
}

int buttonwidget::buttonCount() const
{
    return buttonList.count();
}

void buttonwidget::onButtonClicked(bool clicked)
{
    Q_UNUSED(clicked);
    QAbstractButton *button = qobject_cast<QAbstractButton *>(this->sender());
    clickedButtonIndex = buttonList.indexOf(button);
    emit buttonClicked(clickedButtonIndex, button->text());
}

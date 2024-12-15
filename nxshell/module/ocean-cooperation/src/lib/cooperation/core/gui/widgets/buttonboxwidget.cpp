// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buttonboxwidget.h"

using namespace cooperation_core;

ButtonBoxWidget::ButtonBoxWidget(QWidget *parent)
    : QWidget(parent),
      mainLayout(new QHBoxLayout)
{
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);
    setLayout(mainLayout);
}

int ButtonBoxWidget::addButton(const QIcon &icon, const QString &toolTip, ButtonStyle style)
{
    CooperationIconButton *btn = new CooperationIconButton(this);
    switch (style) {
    case kNormal:
#ifndef linux
        btn->setStyleSheet(
                    ".QToolButton {"
                    "background-color: rgba(0,0,0,0.1);"
                    "border-radius: 16px;"
                    "}"
                    "QToolTip {"
                    "background-color: white;"
                    "}");
#endif
        break;
    case kHighLight:
#ifdef linux
        btn->setBackgroundRole(QPalette::Highlight);
#else
        btn->setStyleSheet(
            ".QToolButton {"
            "background-color: #0098FF;"
            "border-radius: 16px;"
            "}"
            "QToolTip {"
            "background-color: white;"
            "}");
#endif
        break;
    }

#ifdef linux
    btn->setEnabledCircle(true);
#endif
    btn->setToolTip(toolTip);
    btn->setFixedSize(32, 32);
    btn->setIconSize({ 16, 16 });
    btn->setIcon(icon);

    int index = mainLayout->count();
    mainLayout->addWidget(btn);
    connect(btn, &CooperationIconButton::clicked, this, [this, index] {
        emit this->buttonClicked(index);
    });

    return index;
}

QAbstractButton *ButtonBoxWidget::button(int index)
{
    if (index >= mainLayout->count())
        return nullptr;

    auto item = mainLayout->itemAt(index);
    auto btn = item->widget();

    return qobject_cast<QAbstractButton *>(btn);
}

void ButtonBoxWidget::setButtonVisible(int index, bool visible)
{
    auto btn = button(index);
    if (btn)
        btn->setVisible(visible);
}

void ButtonBoxWidget::setButtonClickable(int index, bool clickable)
{
    auto btn = button(index);
    if (btn)
        btn->setEnabled(clickable);
}

void ButtonBoxWidget::clear()
{
    const int count = mainLayout->count();
    for (int i = 0; i != count; ++i) {
        QLayoutItem *item = mainLayout->takeAt(i);
        QWidget *w = item->widget();
        if (w) {
            w->setParent(nullptr);
            w->deleteLater();
        }

        delete item;
    }
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorstylewidget.h"
#include "colorsettingbutton.h"

const int MIN_WIDTH = 83;
ColorStyleWidget::ColorStyleWidget(QWidget *parent): AttributeWgt(-1, parent)
{
    initUi();
}

void ColorStyleWidget::setVar(const QVariant &var)
{
    QSignalBlocker block(this);
    QVariantList l = var.toList();
    QColor c = var.value<QColor>();

    m_fillColor->setVar(c);
    m_fillColorEdit->setText(c.name());
    if (!var.isValid()) {
        m_fillColorEdit->setText("#0");
    }
}

void ColorStyleWidget::setTitleText(QString text)
{
    m_titleLabel->setText(text);
}

void ColorStyleWidget::setColorFill(int fillStyle)
{
    m_fillColor->setColorFill(static_cast<CColorSettingButton::EColorFill>(fillStyle));
}

void ColorStyleWidget::setColorTextVisible(bool bVisible)
{
    m_fillColorEdit->setVisible(bVisible);
}

void ColorStyleWidget::addWidget(QWidget *w, int stretch)
{
    m_lFillColor->addWidget(w, stretch);
    m_addWidgets << w;
}

void ColorStyleWidget::addTitleWidget(QWidget *w, Qt::Alignment alignment)
{
    m_titleLayout->addWidget(w);
    m_titleLayout->setAlignment(w, alignment);
}

void ColorStyleWidget::setContentEnable(bool enable)
{
    enableColorEdit(enable);
}

DLineEdit *ColorStyleWidget::lineEdit()
{
    return m_fillColorEdit;
}

void ColorStyleWidget::initUi()
{
    m_fillColor = new CColorSettingButton(tr("Color"), this, false);
    m_fillColor->setColorFill(CColorSettingButton::EFillArea);
    m_fillColorEdit = new DLineEdit(this);
    m_fillColorEdit->setClearButtonEnabled(false);
    m_fillColorEdit->setMinimumWidth(MIN_WIDTH);
    QVBoxLayout *fillLayout = new QVBoxLayout(this);
    fillLayout->setMargin(0);
    fillLayout->setContentsMargins(0, 0, 0, 0);
    m_lFillColor = new QHBoxLayout(this);
    m_lFillColor->setMargin(0);
    m_lFillColor->setContentsMargins(0, 0, 0, 0);
    m_lFillColor->addWidget(m_fillColor, 3);
    m_lFillColor->addWidget(m_fillColorEdit, 2);

    m_titleLayout = new QHBoxLayout(this);
    m_titleLabel = new DLabel(tr("Fill"));
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->setAlignment(m_titleLabel, Qt::AlignLeft);

    fillLayout->addLayout(m_titleLayout);
    fillLayout->addLayout(m_lFillColor);
    setLayout(fillLayout);
    m_fillColor->show();

    addHSeparateLine();

    connect(m_fillColor, &ColorSettingButton::colorChanged, this, [ = ](const QColor & _t1, int _t2) {
        m_fillColorEdit->setText(_t1.name());
        emit colorChanged(_t1, _t2);
    });

    m_fillColorEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp("[#]?[0-9A-Fa-f]{6}"), nullptr));
    m_fillColorEdit->lineEdit()->setMaxLength(7);
    m_fillColorEdit->setText("ffffff");
    connect(m_fillColorEdit, &DLineEdit::textChanged, this, [ = ](const QString & colorStr) {
        QColor c;
        QString colorStrname;
        if (colorStr.size() == 6) {
            colorStrname = "#" + colorStr;
        } else if (colorStr.size() == 7) {
            colorStrname = colorStr;
        }
        c.setNamedColor(colorStrname);
        if (c.isValid()) {
            // 去除透明度，当前颜色不同才会更新显示颜色
            if (c.rgb() ==  m_fillColor->getColor().rgb()) {
                return;
            }

            m_fillColor->setColor(c);
            QSignalBlocker block(this);
            emit colorChanged(c, EChanged);
        }
    });
}

void ColorStyleWidget::enableColorEdit(bool bEnable)
{
    m_fillColorEdit->setEnabled(bEnable);
    m_fillColor->setEnabled(bEnable);
    for (QWidget *w : m_addWidgets) {
        w->setEnabled(bEnable);
    }
}

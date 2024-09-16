// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "csidewidthwidget.h"
#include "pagecontext.h"
#include "pageview.h"


#include <QComboBox>

#include <QDebug>

#ifdef USE_DTK
#include <DGuiApplicationHelper>
DGUI_USE_NAMESPACE
#endif

SideWidthWidget::SideWidthWidget(QWidget *parent)
    : QWidget(parent)
    , m_comboxHeight(28)
{
    initUI();
    initConnection();

    bool b = connect(this, &SideWidthWidget::widthChanged, this, [ = ](int w, bool b) {});
}

void SideWidthWidget::setText(const QString &text)
{
    _textLabel->setVisible(!text.isEmpty());
    _textLabel->setText(text);
}

void SideWidthWidget::setSpace(int space)
{
    m_layout->setSpacing(space);
}

void SideWidthWidget::setWidth(int width)
{
    if (width >= 0) {
        QString current_px = QString::number(width) + "px";
        m_menuComboBox->setCurrentText(current_px);
        m_maskLable->setVisible(false);
    } else {
        setValid(false);
    }
}

QComboBox *SideWidthWidget::menuComboBox()
{
    return m_menuComboBox;
}

QSize SideWidthWidget::sizeHint() const
{
    return QWidget::sizeHint();
}

void SideWidthWidget::setValid(bool valid)
{
    if (!valid) {
        m_menuComboBox->setCurrentIndex(-1);
    }
    m_maskLable->setVisible(!valid);
}

void SideWidthWidget::initUI()
{
    setWgtAccesibleName(this, "CSideWidthWidget");
    m_layout = new QHBoxLayout(this);
    _textLabel = new QLabel(this);
    m_menuComboBox = new QComboBox(this);
    m_menuComboBox->setFocusPolicy(Qt::NoFocus);
    m_maskLable = new QLabel(m_menuComboBox);
    m_maskLable->setText("— —");
    m_maskLable->move(6, 6);
    m_maskLable->setFixedSize(35, 20);
    m_maskLable->setVisible(true);
    m_maskLable->setFont(m_menuComboBox->font());

    m_menuComboBox->/*setMaximumWidth*/setMinimumWidth(/*100*/90);

    initLineWidthToCombox();
    m_layout->addWidget(_textLabel);
    m_layout->addWidget(m_menuComboBox);
    m_layout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(m_layout);

    _textLabel->hide();
}

void SideWidthWidget::initConnection()
{
    connect(m_menuComboBox, &QComboBox::currentTextChanged, this, [ = ](const QString & text) {
        if (text.contains("px")) {
            // 判断并且获取当前线宽度
            bool flag = false;
            int lineWidth = text.trimmed().toLower().replace("px", "").toInt(&flag);

            if (flag) {
                emit widthChanged(lineWidth, false);
            }
            this->setValid(true);
        }
    });

    // 设置默认2px的线宽度
    m_menuComboBox->setCurrentIndex(1);
}

void SideWidthWidget::initLineWidthToCombox()
{
    m_menuComboBox->addItem("0px");
    m_menuComboBox->addItem("1px");
    m_menuComboBox->addItem("2px");
    m_menuComboBox->addItem("4px");
    m_menuComboBox->addItem("8px");
    m_menuComboBox->addItem("10px");
}



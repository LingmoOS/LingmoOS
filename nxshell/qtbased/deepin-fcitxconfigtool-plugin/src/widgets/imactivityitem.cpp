// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imactivityitem.h"
#include "publisher/publisherdef.h"
#include "window/immodel/immodel.h"
#include <DToolButton>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <QTimer>
#include <QPainterPath>
#include <QGSettings>
#include "window/settingsdef.h"
#include "window/gsettingwatcher.h"
using namespace Dtk::Widget;
namespace dcc_fcitx_configtool {
namespace widgets {

FcitxIMActivityItem::FcitxIMActivityItem(FcitxQtInputMethodItem item, itemPosition index, QWidget *parent)
    : FcitxSettingsItem(parent)
    , m_item(item)
    , m_index(index)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 10, 0);
    m_labelText = new FcitxShortenLabel("", this);
    DFontSizeManager::instance()->bind(m_labelText, DFontSizeManager::T6);
    m_labelText->setShortenText("    " + item.name());
    m_labelText->setAccessibleName(item.name());
    m_labelText->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_layout->addWidget(m_labelText);
    m_upBtn = new DToolButton(this);
    m_downBtn = new DToolButton(this);
    m_configBtn = new DToolButton(this);
    m_deleteLabel = new ClickLabel (this);
    m_upBtn->setIcon(QIcon::fromTheme("arrow_up"));
    m_upBtn->setAccessibleName(item.name()+":arrow_up");
    m_downBtn->setIcon(QIcon::fromTheme("arrow_down"));
    m_downBtn->setAccessibleName(item.name()+":arrow_down");
    m_configBtn->setIcon(QIcon::fromTheme("setting"));
    m_configBtn->setAccessibleName(item.name()+":setting");
    m_deleteLabel->setIcon(DStyle::standardIcon(QApplication::style(), DStyle::SP_DeleteButton));
    m_deleteLabel->setAccessibleName(item.name()+":delete");
    m_deleteLabel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);


    m_layout->addWidget(m_downBtn);
    m_layout->addWidget(m_upBtn);
    m_layout->addWidget(m_configBtn);
    m_layout->addWidget(m_deleteLabel, 0, Qt::AlignRight);

    m_deleteLabel->hide();
    m_upBtn->hide();
    m_configBtn->hide();
    m_downBtn->hide();

    connect(m_upBtn, &DToolButton::clicked, this, &FcitxIMActivityItem::onUpItem);
    connect(m_downBtn, &DToolButton::clicked, this, &FcitxIMActivityItem::onDownItem);
    connect(m_configBtn, &DToolButton::clicked, this, &FcitxIMActivityItem::onConfigItem);
    connect(m_deleteLabel, &ClickLabel::clicked, this, &FcitxIMActivityItem::onDeleteItem);

    this->setFixedHeight(40);
    this->setLayout(m_layout);
}

FcitxIMActivityItem::~FcitxIMActivityItem()
{
}

void FcitxIMActivityItem::editSwitch(const bool &flag)
{
    m_isEdit = flag;
    if (m_isEdit) {
        m_deleteLabel->show();
        m_configBtn->hide();
        m_upBtn->hide();
        m_downBtn->hide();
    } else {
        m_deleteLabel->hide();
    }
}

void FcitxIMActivityItem::paintEvent(QPaintEvent *event)
{
    QPainter painter( this);
    //painter.fillRect(this->rect(), DGuiApplicationHelper::instance()->applicationPalette().background());
    const int radius = 8;
    QRect paintRect = this->rect();
    QPainterPath path;
    if(m_index == firstItem) {
        path.moveTo(paintRect.bottomRight());
        path.lineTo(paintRect.topRight() + QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                         QSize(radius * 2, radius * 2)), 0, 90);
        path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
        path.lineTo(paintRect.bottomLeft());
        path.lineTo(paintRect.bottomRight());
    } if(m_index == lastItem) {
        path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
        path.lineTo(paintRect.topRight());
        path.lineTo(paintRect.topLeft());
        path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                         QSize(radius * 2, radius * 2)), 180, 90);
        path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                         QSize(radius * 2, radius * 2)), 270, 90);
    } if(m_index == otherItem) {
        path.moveTo(paintRect.bottomRight());
        path.lineTo(paintRect.topRight());
        path.lineTo(paintRect.topLeft());
        path.lineTo(paintRect.bottomLeft());
        path.lineTo(paintRect.bottomRight());
    } if(m_index == onlyoneItem) {
        path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
        path.lineTo(paintRect.topRight() + QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                         QSize(radius * 2, radius * 2)), 0, 90);
        path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
        path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
        path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                         QSize(radius * 2, radius * 2)), 180, 90);
        path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
        path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                         QSize(radius * 2, radius * 2)), 270, 90);
    }
    if(m_isEnter) {
        QColor color = DGuiApplicationHelper::instance()->applicationPalette().light().color();
        if(isDraged()) {
            color.setAlpha(80);
        }
        painter.fillPath(path, color);
    } else {
        DPalette p;
        QColor color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
        if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            color = QColor("#323232");
            color.setAlpha(230);
        } else {
            color = DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color();
        }
        if(isDraged()) {
            color = DGuiApplicationHelper::instance()->applicationPalette().light().color();
            color.setAlpha(80);
        }
        painter.fillPath(path, color);
    }
    //qDebug() << "isDraged = " << isDraged();
    return FcitxSettingsItem::paintEvent(event);
}

void FcitxIMActivityItem::mouseMoveEvent(QMouseEvent *e)
{
    this->update(rect());
    return FcitxSettingsItem::mouseMoveEvent(e);
}

void FcitxIMActivityItem::setSelectStatus(const bool &isEnter)
{
//    if (!m_bgGroup)
//        return;
    QGSettings *gsetting = new QGSettings("com.deepin.fcitx-config", QByteArray(), this);
    QString value = gsetting->get(GSETTINGS_ADJUST_BUTTON).toString();
    QString config = gsetting->get(GSETTINGS_SETTING_BUTTON).toString();
    if (isEnter)
        m_isEnter = true;
    else {
        m_isEnter = false;
    }
    if (!m_isEdit && isEnter) {
        int index = IMModel::instance()->getIMIndex(m_item);
        int count = IMModel::instance()->getCurIMList().count();


        if (count <= 1) {
            m_upBtn->setEnabled(false);
            m_downBtn->setEnabled(false);
        }else if (index == 0 && "Enabled" == value) {
            m_upBtn->setEnabled(false);
            m_downBtn->setEnabled(true);
        } else if (index == count - 1 && "Enabled" == value) {
            m_upBtn->setEnabled(true);
            m_downBtn->setEnabled(false);
        } else {
            if("Enabled" == value) {
                m_upBtn->setEnabled(true);
                m_downBtn->setEnabled(true);
            }
        }
        m_configBtn->show();
        m_upBtn->show();
        m_downBtn->show();
        m_upBtn->setVisible("Hidden" != value);
        m_downBtn->setVisible("Hidden" != value);
        m_configBtn->setVisible("Hidden" != config);
        update();
    } else {
        m_configBtn->hide();
        m_upBtn->hide();
        m_downBtn->hide();
    }

    if ("Disabled" == value) {
        m_upBtn->setEnabled(false);
        m_downBtn->setEnabled(false);
    }
    if ("Disabled" == config) {
        m_configBtn->setEnabled(false);
    } else if ("Enabled" == config) {
        m_configBtn->setEnabled(true);
    }
    this->update(rect());
}

void FcitxIMActivityItem::onUpItem()
{
    emit upBtnClicked(m_item);
}

void FcitxIMActivityItem::onDownItem()
{
    emit downBtnClicked(m_item);
}

void FcitxIMActivityItem::onConfigItem()
{
    emit configBtnClicked(m_item);
}

void FcitxIMActivityItem::onDeleteItem()
{
    emit deleteBtnClicked(m_item);
}

void FcitxIMActivityItem::enterEvent(QEvent *event)
{
    setSelectStatus(true);
    FcitxSettingsItem::enterEvent(event);
}

void FcitxIMActivityItem::leaveEvent(QEvent *event)
{
    setSelectStatus(false);
    FcitxSettingsItem::leaveEvent(event);
}

void ToolButton::paintEvent(QPaintEvent *e)
{
    if (isEnabled()) {
        QToolButton::paintEvent(e);
    } else {
        QPainter p(this);
        p.drawPixmap({(width() - 16) / 2, (height() - 16) / 2, 16, 16}, icon().pixmap(16, 16, QIcon::Mode::Disabled));
    }
}



} // namespace widgets
} // namespace dcc_fcitx_configtool

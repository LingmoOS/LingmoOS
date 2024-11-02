/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "devremovedialog.h"

DevRemoveDialog::DevRemoveDialog(REMOVE_INTERFACE_TYPE mode,QWidget *parent):QDialog(parent)
{
    this->m_mode = mode;
    initGsettings();
    this->setFixedSize(380,200);
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    initUI();
    initGsettings();
}

DevRemoveDialog::~DevRemoveDialog()
{
    gsettings->deleteLater();
}

void DevRemoveDialog::initUI()
{
    struct_pos pos;

    if(REMOVE_MANY_TIMES_CONN_FAIL_DEV == this->m_mode)
    {
        title_icon = new QLabel(this);
        title_icon->setPixmap(QIcon::fromTheme(TITLE_ICON_BLUETOOTH).pixmap(20,22));

        pos = getWidgetPos(8,8,20,22, this->width());
        title_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

        title_text = new QLabel(tr("Bluetooth Connections"),this);

        pos = getWidgetPos(36,7,320,20, this->width());
        title_text->setGeometry(pos.x, pos.y, pos.width, pos.high);

        title_text->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    }

    tipLabel = new QLabel(this);

    pos = getWidgetPos(56,25,320,60, this->width());
    tipLabel->setGeometry(pos.x, pos.y, pos.width, pos.high);

    tipLabel->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    tipLabel->setWordWrap(true);
    tipLabel->setFocus();

    if (REMOVE_HAS_PIN_DEV == this->m_mode)
    {
        txtLabel = new QLabel(this);

        pos = getWidgetPos(55,80,320,65, this->width());
        txtLabel->setGeometry(pos.x, pos.y, pos.width, pos.high);

        txtLabel->setAlignment(Qt::AlignTop|Qt::AlignLeft);
        txtLabel->setWordWrap(true);

        QPalette txtPal;
        txtPal.setColor(QPalette::WindowText,QColor("#818181"));
        txtLabel->setPalette(txtPal);

        QString txtStr = tr("After it is removed, the PIN code must be matched for the next connection.");
        QString newTxtStr = QFontMetrics(this->font()).elidedText(txtStr,Qt::ElideMiddle,txtLabel->width());
        QFont txtFont ;
        txtFont.setPointSize(this->fontInfo().pointSize());
        txtLabel->setFont(txtFont);
        txtLabel->setText(newTxtStr);
        if (newTxtStr != txtStr)
            txtLabel->setToolTip(tr("After it is removed, the PIN code must be matched for the next connection."));
    }

    iconLabel = new QLabel(this);

    pos = getWidgetPos(16,45,22,22, this->width());
    iconLabel->setGeometry(pos.x, pos.y, pos.width, pos.high);

    iconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(22,22));

    closeBtn = new QPushButton(this);

    pos = getWidgetPos(350,8,20,20, this->width());
    closeBtn->setGeometry(pos.x, pos.y, pos.width, pos.high);


    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setFlat(true);
    closeBtn->setToolTip(tr("close"));
    closeBtn->setProperty("isWindowButton",0x2);
    closeBtn->setProperty("useIconHighlihtEffect",0x8);
    connect(closeBtn,&QPushButton::clicked,this,[=]{
        this->close();
    });

    acceptBtn = new QPushButton(this);

    pos = getWidgetPos(242,148,120,36, this->width());
    acceptBtn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    acceptBtn->setText(tr("Remove"));
    connect(acceptBtn,&QPushButton::clicked,this,[=]{
        emit accepted();
        this->close();
    });

    rejectBtn = new QPushButton(this);

    pos = getWidgetPos(110,148,120,36, this->width());
    rejectBtn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    rejectBtn->setText(tr("Cancel"));
    connect(rejectBtn,&QPushButton::clicked,this,[=]{
        this->close();
    });
}

void DevRemoveDialog::initGsettings()
{
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        gsettings = new QGSettings("org.lingmo.style");

        if(gsettings->get("style-name").toString() == "lingmo-default"
           || gsettings->get("style-name").toString() == "lingmo-light")
            isblack = false;
        else
            isblack = true;
    }

    connect(gsettings,&QGSettings::changed,this,&DevRemoveDialog::gsettingsSlot);
}

void DevRemoveDialog::setDialogText(const QString &str)
{
    QString txtStr;
    if (REMOVE_MANY_TIMES_CONN_FAIL_DEV == this->m_mode)
        txtStr = QString(tr("Connection failed! Please remove it before connecting."));
    else
        txtStr = QString(tr("Are you sure to remove %1 ?")).arg(str);

    QString newTxtStr = QFontMetrics(this->font()).elidedText(txtStr,Qt::ElideMiddle,tipLabel->width());
    QFont tipFont ;
    tipFont.setPointSize(this->fontInfo().pointSize());
    tipLabel->setFont(tipFont);
    tipLabel->setText(newTxtStr);

    if (newTxtStr != txtStr)
        tipLabel->setToolTip(txtStr);
}

void DevRemoveDialog::gsettingsSlot(const QString &key)
{
    if (key == "styleName") {
        QPalette palette;
        if(gsettings->get("style-name").toString() == "lingmo-default" ||
           gsettings->get("style-name").toString() == "lingmo-light") {

            palette.setBrush(QPalette::Base,QBrush(Qt::white));
            palette.setColor(QPalette::Text,QColor(Qt::black));

            isblack = false;

        } else {

            palette.setBrush(QPalette::Base,QBrush(Qt::black));
            palette.setColor(QPalette::Text,QColor(Qt::white));

            isblack = true;
        }

        this->setPalette(palette);
    }
}

void DevRemoveDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(Qt::transparent);

    if (isblack)
        painter.setBrush(Qt::black);
    else
        painter.setBrush(Qt::white);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.drawRoundedRect(this->rect(),12,12);
}

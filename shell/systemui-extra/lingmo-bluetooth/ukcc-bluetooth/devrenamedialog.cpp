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

#include "devrenamedialog.h"

DevRenameDialog::DevRenameDialog(QWidget *parent):QDialog(parent)
{
    this->setFixedSize(480,192);
    this->adjustSize();
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    initUI();
    initGsettings();
}

DevRenameDialog::~DevRenameDialog()
{
    gsettings->deleteLater();
}

void DevRenameDialog::setRenameInterface(DEV_RENAME_DIALOG_TYPE mode)
{
    this->_mMode = mode;

    if (DEVRENAMEDIALOG_ADAPTER == mode)
    {
        textLabel->setVisible(true);
        titleLabel->setText(tr("Rename"));

    }
    else if (DEVRENAMEDIALOG_BT_DEVICE == mode)
    {
        textLabel->setVisible(false);

        struct_pos pos;
        pos = getWidgetPos(25,55,435,36, this->width());
        lineEdit->setGeometry(pos.x, pos.y, pos.width, pos.high);

        titleLabel->setFixedSize(300,30);
        titleLabel->setText(tr("Rename device"));
        pos = getWidgetPos(20,94,435,25, this->width());
        tipLabel->setGeometry(pos.x, pos.y, pos.width, pos.high);
    }
}

void DevRenameDialog::setDevName(const QString &str)
{
    lineEdit->setText(str);
    adapterOldName = str;
}

void DevRenameDialog::initGsettings()
{
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        gsettings = new QGSettings("org.lingmo.style");

        if(gsettings->get("style-name").toString() == "lingmo-default"
           || gsettings->get("style-name").toString() == "lingmo-light")
            isblack = false;
        else
            isblack = true;

        _fontSize = gsettings->get("system-font-size").toString().toInt();

    }

    connect(gsettings,&QGSettings::changed,this,&DevRenameDialog::gsettingsSlot);
}

void DevRenameDialog::gsettingsSlot(const QString &key)
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

void DevRenameDialog::initUI()
{
    struct_pos pos;
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon::fromTheme(TITLE_ICON_BLUETOOTH).pixmap(20,20));

    pos = getWidgetPos(10,11,20,20, this->width());
    iconLabel->setGeometry(pos.x, pos.y, pos.width, pos.high);

    titleLabel = new QLabel(this);

    pos = getWidgetPos(36,5,120,30, this->width());
    titleLabel->setGeometry(pos.x, pos.y, pos.width, pos.high);

    titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    closeBtn = new QPushButton(this);

    pos = getWidgetPos(453,8,20,20, this->width());
    closeBtn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setFlat(true);
    closeBtn->setToolTip(tr("close"));
    closeBtn->setProperty("isWindowButton",0x2);
    closeBtn->setProperty("useIconHighlihtEffect",0x8);
    connect(closeBtn,&QPushButton::clicked,this,[=]{
        this->close();
    });

    textLabel = new QLabel(this);

    pos = getWidgetPos(24,64,60,20, this->width());
    textLabel->setGeometry(pos.x, pos.y, pos.width, pos.high);

    textLabel->setText(tr("Name"));
    textLabel->setAlignment(Qt::AlignHCenter|Qt::AlignRight);
    textLabel->adjustSize();

    int wigth = textLabel->width();
    wigth = wigth - 60;
    lineEdit = new QLineEdit(this);
    lineEdit->setFocus();
    //lineEdit->setMaxLength(INPUT_STRNAME_MAX);
    pos = getWidgetPos(100 + wigth,55,355 - wigth,36, this->width());
    lineEdit->setGeometry(pos.x, pos.y, pos.width, pos.high);
    //输入变化时进行长度提示
    connect(lineEdit,&QLineEdit::textEdited,this,&DevRenameDialog::lineEditSlot);

    tipLabel = new QLabel(this);

    pos = getWidgetPos(96 + wigth,94,400,25, this->width());
    tipLabel->setGeometry(pos.x, pos.y, pos.width, pos.high);

    //tipLabel->setText(tr("Cannot be null"));
    tipLabel->setText(tr("The value contains 1 to 32 characters"));
    tipLabel->setVisible(false);
    tipLabel->setStyleSheet("color: rgba(255, 0, 0, 0.85);\
                            opacity: 1;");

    acceptBtn = new QPushButton(tr("OK"),this);

    pos = getWidgetPos(359,130,96,36, this->width());
    acceptBtn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    connect(acceptBtn,&QPushButton::clicked,this,[=]{

        qWarning()<< Q_FUNC_INFO << "acceptBtn clicked !" << __LINE__;
        if ((lineEdit->text().length() >= INPUT_STRNAME_MIN) &&
            (lineEdit->text().length() <= INPUT_STRNAME_MAX) &&
            (lineEdit->text() != adapterOldName))
            emit nameChanged(lineEdit->text());

        this->close();
    });

    rejectBtn = new QPushButton(tr("Cancel"),this);

    pos = getWidgetPos(247,130,96,36, this->width());
    rejectBtn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    connect(rejectBtn,&QPushButton::clicked,this,[=]{
        this->close();
    });
}


void DevRenameDialog::lineEditSlot(const QString &str)
{
    if (str.length() >= INPUT_STRNAME_MIN && str.length() <= INPUT_STRNAME_MAX) {
        acceptBtn->setDisabled(false);
        tipLabel->setVisible(false);
    } else if (INPUT_STRNAME_MIN > str.length()) {
        acceptBtn->setDisabled(true);
        tipLabel->setVisible(true);
    }else {
        //输入字符超过32，则显示32个字符并提示用户
        QString showText = str.left(32);
        lineEdit->setText(showText);
        acceptBtn->setDisabled(false);
        tipLabel->setVisible(true);
    }
}

void DevRenameDialog::paintEvent(QPaintEvent *event)
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

void DevRenameDialog::keyPressEvent(QKeyEvent * event)
{
    Q_UNUSED(event)
    switch(event->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        //if(acceptBtn->isEnabled())//setEnable状态后，emit click 信号不相应,无需判断
            emit acceptBtn->click();
        break;
    case Qt::Key_Escape:
        //if(rejectBtn->isEnabled())//setEnable状态后，emit click 信号不相应,无需判断
            emit rejectBtn->click();
        break;
    }

}

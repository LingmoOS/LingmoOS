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

#include "bluetoothnamelabel.h"

BluetoothNameLabel::BluetoothNameLabel(QWidget *parent, int x, int y):
    QWidget(parent)
{
    this->setAutoFillBackground(true);
    this->setObjectName("BluetoothNameLabel");
    this->setStyleSheet("QWidget{border: none;border-radius:2px;}");
//    this->setMouseTracking(true);

    this->setMinimumSize(x,y);

    hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(5,0,5,0);
    hLayout->setSpacing(0);

    m_label = new QLabel(this);
    m_label->setContentsMargins(1,0,1,0);
    m_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    hLayout->addWidget(m_label);
    hLayout->addStretch(0);

    const auto ratio = qApp->devicePixelRatio();
    icon_pencil = new QLabel(this);
    icon_pencil->setFixedSize(QSize(ICON_PENCIL_W*ratio,ICON_PENCIL_H*ratio));
    icon_pencil->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(ICON_PENCIL_W*ratio,ICON_PENCIL_H*ratio));
    icon_pencil->setToolTip(tr("Click to change the device name"));
    hLayout->addWidget(icon_pencil);
    hLayout->addStretch(20);

    if(QGSettings::isSchemaInstalled("org.lingmo.style")){
        settings = new QGSettings("org.lingmo.style");
        if(settings->get("style-name").toString() == "lingmo-black" || settings->get("style-name").toString() == "lingmo-dark")
        {
            style_flag = true;
            icon_pencil->setPixmap(ukccbluetoothconfig::loadSvg(QIcon::fromTheme("document-edit-symbolic").pixmap(ICON_PENCIL_W*ratio,ICON_PENCIL_H*ratio),ukccbluetoothconfig::WHITE));
        }
        else
        {
            style_flag = false;
            icon_pencil->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(ICON_PENCIL_W*ratio,ICON_PENCIL_H*ratio));
        }
        connect(settings,&QGSettings::changed,this,&BluetoothNameLabel::settings_changed);
    }

}

BluetoothNameLabel::~BluetoothNameLabel()
{
    delete settings;
    settings = nullptr;
    if(nullptr != renameDialog)
    {
        renameDialog->deleteLater();
    }
}

void BluetoothNameLabel::set_dev_name(const QString &dev_name)
{
    device_name = dev_name;

    setMyNameLabelText(device_name);
//    QFontMetrics fontWidth(m_label->font());//得到QLabel字符的度量
//    QString elidedNote = fontWidth.elidedText(dev_name, Qt::ElideMiddle, m_label->width());//获取处理后的文本
//    m_label->setText(elidedNote);
//    m_label->setToolTip(tr("Can now be found as \"%1\"").arg(dev_name));
//    m_label->update();
}

void BluetoothNameLabel::showDevRenameDialog()
{
    if(nullptr != renameDialog)
    {
        delete renameDialog;
        renameDialog = nullptr;
    }

    renameDialog = new DevRenameDialog(this);
    renameDialog->setDevName(device_name);
    renameDialog->setRenameInterface(DevRenameDialog::DEVRENAMEDIALOG_ADAPTER);

    connect(renameDialog,&DevRenameDialog::nameChanged,this,[=](QString name){
        m_label->setText(name);
        device_name = name;
        emit sendAdapterName(name);
    });

    renameDialog->exec();
}

void BluetoothNameLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(m_label->geometry().contains(this->mapFromGlobal(QCursor::pos())) || icon_pencil->geometry().contains(this->mapFromGlobal(QCursor::pos())))
        showDevRenameDialog();
}

void BluetoothNameLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(m_label->geometry().contains(this->mapFromGlobal(QCursor::pos())) || icon_pencil->geometry().contains(this->mapFromGlobal(QCursor::pos())))
        showDevRenameDialog();
}

//void BluetoothNameLabel::mouseMoveEvent(QMouseEvent *event)
//{
//    Q_UNUSED(event);
//    if(m_label->geometry().contains(this->mapFromGlobal(QCursor::pos())))
//    {
//        KyWarning() << "===================" << "m_label";
//    }

//    if(icon_pencil->geometry().contains(this->mapFromGlobal(QCursor::pos())))
//    {

//        KyWarning() << "==================="<< "icon_pencil";
//    }
//}

void BluetoothNameLabel::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
}

void BluetoothNameLabel::enterEvent(QEvent *event){
    Q_UNUSED(event);

    if(style_flag)
        this->setStyleSheet("QWidget#BluetoothNameLabel{background-color:black;border:none;border-radius:2px;}");
    else
        this->setStyleSheet("QWidget#BluetoothNameLabel{background-color:white;border:none;border-radius:2px;}");


}
void BluetoothNameLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (nullptr != m_label)
    {
        setMyNameLabelText(device_name);

//        QFontMetrics fontWidth(m_label->font());//得到QLabel字符的度量
//        QString elidedNote = fontWidth.elidedText(device_name, Qt::ElideMiddle, m_label->width());//获取处理后的文本
//        m_label->setText(elidedNote);
//        m_label->update();
    }
}

void BluetoothNameLabel::settings_changed(const QString &key)
{
    qDebug() << Q_FUNC_INFO <<key;
    const auto ratio = qApp->devicePixelRatio();

    if(key == "styleName"){
        if(settings->get("style-name").toString() == "lingmo-black" || settings->get("style-name").toString() == "lingmo-dark")
        {
            style_flag = true;
            //icon_pencil->setPixmap(ImageUtil::drawSymbolicColoredPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(ICON_PENCIL_W_H),"white"));
            icon_pencil->setPixmap(ukccbluetoothconfig::loadSvg(QIcon::fromTheme("document-edit-symbolic").pixmap(ICON_PENCIL_W*ratio,ICON_PENCIL_H*ratio),ukccbluetoothconfig::WHITE));
        }
        else
        {
            style_flag = false;
            icon_pencil->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(ICON_PENCIL_W*ratio,ICON_PENCIL_H*ratio));
        }
    }
    else if(key == "systemFontSize"){
//        qWarning() << Q_FUNC_INFO <<key << __LINE__;

        setMyNameLabelText(device_name);
//        QFontMetrics fontWidth(m_label->font());//得到QLabel字符的度量
//        qWarning() << Q_FUNC_INFO  << m_label->width();

//        QString elidedNote = fontWidth.elidedText(device_name, Qt::ElideMiddle, m_label->width());//获取处理后的文本
//        qWarning() << Q_FUNC_INFO <<elidedNote << __LINE__;

//        m_label->setText(elidedNote);
//        m_label->update();
    }
}


void BluetoothNameLabel::setMyNameLabelText(QString name_str)
{
    QFontMetrics fontMe(m_label->font());//得到QLabel字符的度量
    int fontWidth = fontMe.width(name_str);
    qWarning() << Q_FUNC_INFO  << m_label->width() << fontWidth ;

    QString elidedNote = name_str ;

    if (fontWidth > 200)
        elidedNote = fontMe.elidedText(name_str, Qt::ElideMiddle, 200);//获取处理后的文本

    qWarning() << Q_FUNC_INFO <<elidedNote << __LINE__;

    m_label->setText(elidedNote);
    m_label->setToolTip(tr("Can now be found as \"%1\"").arg(name_str));

//    m_label->update();

}


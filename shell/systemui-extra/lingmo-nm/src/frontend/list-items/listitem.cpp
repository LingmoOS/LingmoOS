/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "listitem.h"
#include <QDebug>

#define MAIN_LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_SPACING 0
#define ITEM_FRAME_MARGINS 12,6,16,6

#define ITEM_FRAME_SPACING 8
#define FRAME_WIDTH 404
#define INFO_ICON_WIDTH 16
#define INFO_ICON_HEIGHT 16
#define LIGHT_HOVER_COLOR QColor(240,240,240,255)
#define DARK_HOVER_COLOR QColor(15,15,15,255)

#define CONNECT_BUTTON_WIDTH 96
#define PWD_AREA_HEIGHT 36

#define FREQLABLE_HIGHT 18
#define FREQLABLE_MARGINS 4,0,4,0
#define LOADIMG_SIZE 16,16

FreqLabel::FreqLabel(QWidget *parent) : QLabel(parent)
{
    const QByteArray id("org.lingmo.style");
    QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
    if(QGSettings::isSchemaInstalled(id)){
        connect(fontSetting, &QGSettings::changed,[=](QString key) {
            if ("systemFontSize" ==key) {
                changedFontSlot();
            }
        });
    }
    changedFontSlot();
}

void FreqLabel::changedFontSlot()
{
    const QByteArray id("org.lingmo.style");
    if(QGSettings::isSchemaInstalled(id)){
        QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
        QVariant fontVariant =  fontSetting->get("systemFontSize");
        QFont font;
        font.setPointSize(fontVariant.toInt()*0.85);
        this->setFont(font);
    }
}

void FreqLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); //抗锯齿效果
    auto rect = this->rect();
    painter.drawRoundedRect(rect, 6, 6);
    QLabel::paintEvent(event);
}

FixPushButton::FixPushButton(QWidget *parent) :
    QPushButton(parent)
{
    const QByteArray id("org.lingmo.style");
    QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
    if(QGSettings::isSchemaInstalled(id)){
        connect(fontSetting, &QGSettings::changed,[=](QString key) {
            if ("systemFont" == key || "systemFontSize" ==key) {
                changedLabelSlot();
            }
        });
    }
}


void FixPushButton::setButtonText(QString text) {

    mStr = text;
    changedLabelSlot();
}

QString FixPushButton::getText(){
    return mStr;
}

void FixPushButton::changedLabelSlot() {
    QFontMetrics  fontMetrics(this->font());
    int fontSize = fontMetrics.width(mStr);
    if (fontSize > 65) {
        setText(fontMetrics.elidedText(mStr, Qt::ElideRight, 65));
        setToolTip(mStr);
    } else {
        setText(mStr);
        setToolTip("");
    }
}

ListItem::ListItem(QWidget *parent) : QFrame(parent)
{
    m_connectState = UnknownState;

    initUI();
    initConnection();
    connect(qApp, &QApplication::paletteChanged, this, &ListItem::onPaletteChanged);
//    m_itemFrame->installEventFilter(this);
    const QByteArray id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings * styleGsettings = new QGSettings(id, QByteArray(), this);
        connect(styleGsettings, &QGSettings::changed, this, [=](QString key){
            if ("themeColor" == key) {
                onPaletteChanged();
            }
        });
    }
    onPaletteChanged();
}

ListItem::~ListItem()
{
    if (nullptr != m_netButton) {
        delete m_netButton;
        m_netButton = nullptr;
    }

//    if (nullptr != m_infoButton) {
//        delete m_infoButton;
//        m_infoButton = nullptr;
//    }

}

void ListItem::setName(const QString &name)
{
    m_nameLabel->setLabelText(name);
}

//仅无线调用，有线自己获取
void ListItem::setActive(const bool &isActive)
{
    m_netButton->setActive(isActive);
    m_isActive = isActive;
}

void ListItem::setConnectState(ConnectState state)
{
    m_connectState = state;
}

void ListItem::showDesktopNotify(const QString &message, QString soundName)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QStringList actions;  //跳转动作
    actions.append("default");
    actions.append("lingmo-nm");          //默认动作：点击消息体时打开灵墨录音
    QMap<QString, QVariant> hints;
    if (!soundName.isEmpty()) {
        hints.insert("sound-name",soundName); //添加声音
    }
    QList<QVariant> args;
    args<<(tr("Lingmo NM"))
       <<((unsigned int) 0)
      <<QString("gnome-dev-ethernet")
     <<tr("lingmo network applet desktop message") //显示的是什么类型的信息
    <<message //显示的具体信息
    <<actions
    <<hints
    <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}

void ListItem::mousePressEvent(QMouseEvent *event)
{
    qDebug()<<"[ListItem]"<<"mousePressEvent";
    if (event->button() == Qt::RightButton) {
        onRightButtonClicked();
    }
    return QFrame::mousePressEvent(event);
}

void ListItem::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug()<<"[ListItem]"<<"mouseReleaseEvent";
    if (event->button() == Qt::LeftButton) {
        onNetButtonClicked();
    }
    return QFrame::mouseReleaseEvent(event);
}

void ListItem::enterEvent(QEvent *event)
{
//    QPalette pal = qApp->palette();
//    QColor baseColor = qApp->palette().base().color();
//    if (baseColor.red() > MIDDLE_COLOR) {
//        pal.setColor(QPalette::Window, LIGHT_HOVER_COLOR);
//    } else {
//        pal.setColor(QPalette::Window, DARK_HOVER_COLOR);
//    }
//    this->setPalette(pal);
//    return QFrame::enterEvent(event);
}

void ListItem::leaveEvent(QEvent *event)
{
//    QPalette pal = qApp->palette();
//    pal.setColor(QPalette::Window, qApp->palette().base().color());
//    this->setPalette(pal);
//    return QFrame::leaveEvent(event);
}

void ListItem::paintEvent(QPaintEvent *event)
{
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing); //反锯齿
//    painter.setBrush(this->palette().brush(QPalette::Window));
//    painter.setPen(Qt::transparent);
//    painter.drawRoundedRect(this->rect(), 6, 6);
//    return QFrame::paintEvent(event);
}

void ListItem::initUI()
{
    m_menu = new QMenu(this);//右键菜单
//    m_menu->setStyleSheet("QMenu::item{border:3px; border-radius:3px}");
//    m_menu->setStyleSheet("QMenu{border-radius:6px; margin:6px 6px 6px 6px}");
    connect(m_menu, &QMenu::triggered, this, &ListItem::onMenuTriggered);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    m_mainLayout->setSpacing(MAIN_LAYOUT_SPACING);
    this->setLayout(m_mainLayout);

    m_itemFrame = new QFrame(this);
    m_itemFrame->setFixedWidth(FRAME_WIDTH);

    m_hItemLayout = new QHBoxLayout(m_itemFrame);
    m_hItemLayout->setContentsMargins(ITEM_FRAME_MARGINS);
    m_hItemLayout->setSpacing(0);
    m_hItemLayout->setAlignment(Qt::AlignHCenter);

    m_netButton = new RadioItemButton(m_itemFrame);
    m_freq = new FreqLabel(m_itemFrame);
    m_freq->setEnabled(false);
    m_freq->setText("...");
    m_freq->setAlignment(Qt::AlignCenter);
    m_freq->hide();
    m_freq->setFixedHeight(FREQLABLE_HIGHT);
    m_freq->setContentsMargins(FREQLABLE_MARGINS);
    m_nameLabel = new NameLabel(m_itemFrame);
    m_hoverButton = new FixPushButton(m_itemFrame);
    m_hoverButton->setProperty("needTranslucent", true);
    m_hoverButton->setFixedSize(CONNECT_BUTTON_WIDTH, PWD_AREA_HEIGHT);
//    m_infoButton = new InfoButton(m_itemFrame);
//    m_infoButton->setIconSize(QSize(INFO_ICON_WIDTH,INFO_ICON_HEIGHT));

    m_lbLoadUp = new QLabel(m_itemFrame);
    m_lbLoadUp->setAlignment(Qt::AlignCenter);
    m_lbLoadDown = new QLabel(m_itemFrame);
    m_lbLoadDown->setAlignment(Qt::AlignCenter);
    m_lbLoadDownImg = new QLabel(m_itemFrame);
    m_lbLoadUpImg = new QLabel(m_itemFrame);
    m_lbLoadUp->hide();
    m_lbLoadDown->hide();
    m_lbLoadDownImg->hide();
    m_lbLoadUpImg->hide();
    m_lbLoadDownImg->setFixedSize(LOADIMG_SIZE);
    m_lbLoadDownImg->setAlignment(Qt::AlignCenter);
    m_lbLoadUpImg->setFixedSize(LOADIMG_SIZE);
    m_lbLoadUpImg->setAlignment(Qt::AlignCenter);
    QFont font;
    font.setPointSize(10);
    m_lbLoadUp->setFont(font);
    m_lbLoadDown->setFont(font);
    m_lbLoadUp->setText("0KB/s");
    m_lbLoadDown->setText("0KB/s");
    m_lbLoadDownImg->setPixmap(QPixmap(QLatin1String(":/res/x/load-down.png")));
    m_lbLoadUpImg->setPixmap(QPixmap(QLatin1String(":/res/x/load-up.png")));

    m_hItemLayout->addWidget(m_netButton);
    m_hItemLayout->addSpacing(10);
    m_hItemLayout->addWidget(m_nameLabel);
    m_hItemLayout->addSpacing(6); //设计稿间距为8 nameLabel宽度另+2
    m_hItemLayout->addWidget(m_freq);
    m_hItemLayout->addStretch();
    m_hItemLayout->addSpacing(8);
    m_hItemLayout->addWidget(m_lbLoadUpImg);
    m_hItemLayout->addWidget(m_lbLoadUp);
    m_hItemLayout->addSpacing(2);
    m_hItemLayout->addWidget(m_lbLoadDownImg);
    m_hItemLayout->addWidget(m_lbLoadDown);
    m_hItemLayout->addWidget(m_hoverButton);
//    m_hItemLayout->addWidget(m_infoButton);

    m_mainLayout->addWidget(m_itemFrame);

    m_hoverButton->hide();
//    this->setAutoFillBackground(true);
//    this->setBackgroundRole(QPalette::Base);
//    QPalette pal = qApp->palette();
//    pal.setColor(QPalette::Window, qApp->palette().base().color());
//    this->setPalette(pal);
}


void ListItem::initConnection()
{
    connect(this->m_netButton, &RadioItemButton::clicked, this, &ListItem::onNetButtonClicked);
    //    connect(this->m_infoButton, &InfoButton::clicked, this, &ListItem::onInfoButtonClicked);
}

void ListItem::onPaletteChanged()
{
//    QPalette pal = qApp->palette();
//    pal.setColor(QPalette::Window, qApp->palette().base().color());
//    this->setPalette(pal);
    QPalette pal = qApp->palette();
    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id, QByteArray(), this);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = themePalette(true, this);
       }
    }
    pal.setColor(QPalette::Base, pal.color(QPalette::Base)); //解决Wayland环境this->setPalette(pal)不生效问题

    this->setPalette(pal);

    if (m_menu != nullptr) {
        pal.setColor(QPalette::Text, pal.color(QPalette::Text));
        m_menu->setPalette(pal);
    }
}

NameLabel::NameLabel(QWidget *parent)
    :QLabel(parent)
{
    const QByteArray id("org.lingmo.style");
    QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
    if(QGSettings::isSchemaInstalled(id)){
        connect(fontSetting, &QGSettings::changed,[=](QString key) {
            if ("systemFont" == key || "systemFontSize" ==key) {
                changedLabelSlot();
            }
        });
    }
}

void NameLabel::setLabelText(QString text)
{
    m_name = text;
    changedLabelSlot();
}

void NameLabel::setLabelMaximumWidth(int width)
{
    m_maximumWidth = width;
    this->setMaximumWidth(m_maximumWidth);
    if (m_name != nullptr) {
        changedLabelSlot();
    }
}

void NameLabel::changedLabelSlot()
{
    QFontMetrics  fontMetrics(this->font());
    int fontSize = fontMetrics.width(m_name);
    if (fontSize > m_maximumWidth) {
        this->setFixedWidth(m_maximumWidth - 2);
        setText(fontMetrics.elidedText(m_name, Qt::ElideRight, m_maximumWidth));
        setToolTip(m_name);
    } else {
        this->setFixedWidth(fontMetrics.width(m_name) + 2);
        setText(m_name);
        setToolTip("");
    }
}

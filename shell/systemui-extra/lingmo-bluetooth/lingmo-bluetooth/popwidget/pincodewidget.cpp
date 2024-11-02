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

#include "pincodewidget.h"
#include "config/xatom-helper.h"
#include "config/config.h"

PinCodeWidget::PinCodeWidget(QString name, QString pin, bool flag, QWidget *parent)
   : dev_name(name),
     PINCode(pin),
     show_flag(flag)
{
    if (envPC == Environment::LAIKA || envPC == Environment::MAVIS)
        is_Intel = true;
    else
        is_Intel = false;

    if (envPC == Environment::HUAWEI
            && QGSettings::isSchemaInstalled("org.lingmo.SettingsDaemon.plugins.xsettings"))
        scale = QGSettings("org.lingmo.SettingsDaemon.plugins.xsettings").get("scaling-factor").toDouble();

    if(QGSettings::isSchemaInstalled("org.lingmo.style")){
        settings = new QGSettings("org.lingmo.style");
        connect(settings,&QGSettings::changed,this,&PinCodeWidget::GSettingsChanges);
    }
//    setProperty("useSystemStyleBlur", true);

    const KWindowInfo info(this->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager))
        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);

    this->setWindowModality(Qt::WindowModality::WindowModal);

    //窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_CLOSE;
    hints.decorations = MWM_DECOR_ALL;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    if (is_Intel)
        this->setWindowTitle(tr("Bluetooth Connection"));
    else
        this->setWindowTitle(tr("Bluetooth pairing"));
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    QFont ft_14, ft_16, ft_20, ft_24, ft_40;
    double ft_size = 12;
    if(QGSettings::isSchemaInstalled("org.lingmo.style"))
        ft_size = settings->get("system-font-size").toDouble() - 11;
    ft_14.setPixelSize((14 + ft_size) * scale);
    ft_16.setPixelSize((16 + ft_size) * scale);
    ft_20.setPixelSize((20 + ft_size) * scale);
    ft_24.setPixelSize((24 + ft_size) * scale);
    ft_40.setPixelSize((40 + ft_size) * scale);
    tip_label = new QLabel(this);
    tip_label->setFont(ft_14);
    tip_label->setWordWrap(true);
//    tip_label->setToolTip(dev_name);

    PIN_label = new QLabel(this);
    PIN_label->setFont(ft_24);

    accept_btn = new QPushButton(tr("Connect"),this);
    accept_btn->setFocusPolicy(Qt::NoFocus);

    refuse_btn = new QPushButton(tr("Refuse"),this);
    refuse_btn->setFocusPolicy(Qt::NoFocus);

    if(show_flag){
        connect(accept_btn,&QPushButton::clicked,this,&PinCodeWidget::onClick_accept_btn);
        connect(refuse_btn,&QPushButton::clicked,this,&PinCodeWidget::onClick_refuse_btn);
    }else{
        connect(accept_btn,&QPushButton::clicked,this,&PinCodeWidget::onClick_close_btn);
        accept_btn->setText(tr("Confirm"));
        accept_btn->adjustSize();
        //accept_btn->setddVisible(false);
        refuse_btn->setVisible(false);
    }


    struct_pos pos;

    if (is_Intel) {
        this->setWindowFlags(Qt::Dialog);
        this->setFixedSize(510, 239);
        tip_label->setFont(ft_16);
        PIN_label->setFont(ft_40);

        pos = Config::getWidgetPos(24,24,450,50, this->width());
        tip_label->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(24,88,462,47, this->width());
        PIN_label->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(374,159,112,56, this->width());
        accept_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(246,159,112,56, this->width());
        refuse_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);
    } else {
        this->setFixedSize(520 * scale ,172 * scale);

        pos = Config::getWidgetPos(25 * scale,58 * scale,207 * scale,36 * scale, this->width());
        PIN_label->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(25 * scale,16 * scale,437 * scale,48 * scale, this->width());
        tip_label->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(388 * scale,112 * scale,108 * scale,36 * scale, this->width());
        accept_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(264 * scale,112 * scale,108 * scale,36 * scale, this->width());
        refuse_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);
    }
    updateUIInfo(name, pin);
    QList<QScreen *> list = QGuiApplication::screens();
    this->move(list.at(0)->size().width()/2-this->width()/2,list.at(0)->size().height()/2-this->height()/2);
}

PinCodeWidget::~PinCodeWidget()
{
    if (refuse_btn->isVisible()) {
        emit this->rejected();
    }
    emit msgShowedOver();
    KyDebug() << dev_name << PINCode << show_flag;
}

void PinCodeWidget::pairFailureShow()
{
    KyDebug();
    if (is_Intel) {
        warn_icon = new QLabel(this);
        warn_icon->setFixedSize(64 * scale,64 * scale);
        KyDebug();
        struct_pos pos;
        warn_icon->setPixmap(QPixmap("/usr/share/lingmo-bluetooth/connection-failed.svg"));

        pos = Config::getWidgetPos(223 * scale,42 * scale,64 * scale,64 * scale, this->width());
        warn_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

        warn_icon->setVisible(true);
        tip_label->setText(QString(tr("Bluetooth Connect Failed")));

        pos = Config::getWidgetPos(210 * scale,108 * scale,190 * scale,28 * scale, this->width());
        tip_label->setGeometry(pos.x, pos.y, pos.width, pos.high);

        tip_label->setStyleSheet("QLabel{\
                                         font-family: NotoSansCJKsc;\
                                         font-size: 16px;\
                                         line-height: 19px;\
                                         text-align: center;\
                                         color: #FB5050;}");
        PIN_label->setVisible(false);
        accept_btn->setVisible(false);
        refuse_btn->setText(tr("Close"));

        pos = Config::getWidgetPos(374 * scale,159 * scale,112 * scale,56 * scale, this->width());
        refuse_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

        return;
    }
    KyDebug()<< this ;
    this->setHidden(true);
    emit msgShowedOver();
}

void PinCodeWidget::updateUIInfo(const QString &name, const QString &pin)
{
    KyDebug() << this->isActiveWindow() ;
    PINCode = pin;
    PIN_label->setText(pin);
    PIN_label->update();
    QFontMetrics fontMetrics(tip_label->font());
    QString Name = fontMetrics.elidedText(name, Qt::ElideRight, 240);
    QString tip_text;
    if(show_flag)
        if (is_Intel)
            tip_text = tr("If the PIN on \'")+Name+tr("\' is the same as this PIN. Please press \'Connect\'");
        else
            tip_text = tr("If \'")+Name+tr("\' the PIN on is the same as this PIN. Please press \'Connect\'.");
    else
        tip_text = QString(tr("Please enter the following PIN code on the bluetooth device '%1' and press enter to pair:")).arg(Name);
//    QString show_text = fontMetrics.elidedText(tip_text, Qt::ElideMiddle, tip_label->width());
    if (Name.length() != name.length())
        tip_label->setToolTip(name);
    tip_label->setText(tip_text);
    tip_label->update();

    if (!this->isActiveWindow())
        this->setHidden(false);
}

void PinCodeWidget::onClick_close_btn(bool)
{
    KyDebug();
    if (show_flag) {
        emit this->rejected();
    }
    this->close();
}

void PinCodeWidget::onClick_accept_btn(bool)
{
    KyDebug() << "pincodewindget.cpp : accpet";
    emit this->accepted();
    this->close();
}

void PinCodeWidget::onClick_refuse_btn(bool)
{
    emit this->rejected();
    this->close();
}

void PinCodeWidget::GSettingsChanges(const QString &key)
{
    KyDebug()<< key;
    if(key == "styleName"){
        this->update();
    }
    if (key == "systemFontSize") {
        double size = settings->get("system-font-size").toDouble() - 11;
        KyDebug()<< size;
        QFont ft, ft1;
        ft.setPixelSize((14 + size) * scale);
        tip_label->setFont(ft);
        ft1.setPixelSize((24 + size) * scale);
        PIN_label->setFont(ft1);
    }
}

QPushButton* PinCodeWidget::getButton() {
    switch (pressCnt) {
    case 1:
        if (!accept_btn->isVisible() || !refuse_btn->isVisible())
            ++pressCnt;
        if(accept_btn->isVisible())
            return accept_btn;
        else if (refuse_btn->isVisible()){
            return refuse_btn;
        }
        return nullptr;
    case 2:
        return refuse_btn;
    case 3:
//        return close_btn;
    default:
        return nullptr;
    }
}

void PinCodeWidget::paintEvent(QPaintEvent *event) {
    QPainter pt(this);
    QColor color = QColor("#818181");

    pt.setPen(color);
    selectedBtn = getButton();
    if (selectedBtn == nullptr)
        return;
    QRect rect = selectedBtn->geometry();
    pt.setRenderHint(QPainter::Antialiasing);
    pt.drawRect(rect);
    QWidget::paintEvent(event);
}

void PinCodeWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Tab:
        KyDebug() << "press Tab";
        ++pressCnt;
        if (pressCnt == 4)
            pressCnt = 1;
        this->update();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        KyDebug() << "press Enter";
        if (selectedBtn == nullptr)
            return;
        selectedBtn->clicked();
        break;
    case Qt::Key_Escape:
        KyDebug() << "press Esc";
        if (pressCnt == 0)
            this->close();
        pressCnt = 0;
        this->update();
        break;
    default:
        break;
    }
}

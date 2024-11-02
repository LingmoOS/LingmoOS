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

#include "qdevitem.h"

QDevItem::QDevItem(QString address, QMap<QString, QVariant> devAttr, QWidget *parent):
    QWidget(parent)
{
    initMap();

    if(QGSettings::isSchemaInstalled("org.lingmo.style")) {
        styleGSetting = new QGSettings("org.lingmo.style");
        connect(styleGSetting,&QGSettings::changed,this,[=]{
            if (m_Battery != -1 && _MDev.value("Connected").toBool()) {
                if(styleGSetting->get("style-name").toString() != "lingmo-light") {
                    m_pBatteyIconLabel->setProperty("useIconHighlightEffect", 0x10);
                } else {
                    m_pBatteyIconLabel->setProperty("useIconHighlightEffect", 0x2);
                }
                QString batteryIcon = QString("battery-level-") + QString::number(m_Battery / 10 * 10) + QString("-symbolic");
                QPixmap batteryPixmap = QIcon::fromTheme(batteryIcon).pixmap(20, 20);
                m_pBatteyIconLabel->setPixmap(batteryPixmap);
            }
            this->update();
        });
    }
    const QByteArray _transparencyGsetting(TRANSPARENCY_SETTINGS);
    if(QGSettings::isSchemaInstalled(_transparencyGsetting)){
        transparencyGsetting = new QGSettings(_transparencyGsetting);
        tran=transparencyGsetting->get(TRANSPARENCY_KEY).toDouble()*255;
    }

    const QByteArray _powerGsetting(POWERMANAGER_SCHEMA);
    if(QGSettings::isSchemaInstalled(_powerGsetting)){
        powerGsetting = new QGSettings(_powerGsetting);
        powerPercentage = powerGsetting->get(PERCENTAGE_LOW).toInt();
        connect(powerGsetting, &QGSettings::changed, this, [=](QString key) {
            if (key == PERCENTAGE_LOW_KEY) {
                powerPercentage = powerGsetting->get(PERCENTAGE_LOW).toInt();
            }
        });
    }
    m_pBatteyIconLabel = new QLabel(this);
    if(styleGSetting->get("style-name").toString() != "lingmo-light") {
        m_pBatteyIconLabel->setProperty("useIconHighlightEffect", 0x10);
    } else {
        m_pBatteyIconLabel->setProperty("useIconHighlightEffect", 0x2);
    }
    m_pBatteyIconLabel->setGeometry(320,14,20,20);

    _MDev = devAttr;
    m_Battery = _MDev.value("Battery").toInt();
    if (m_Battery != -1 && _MDev.value("Connected").toBool()) {
        QString batteryIcon = QString("battery-level-") + QString::number(m_Battery / 10 * 10) + QString("-symbolic");
        QPixmap batteryPixmap = QIcon::fromTheme(batteryIcon).pixmap(20, 20);
        m_pBatteyIconLabel->setPixmap(batteryPixmap);
    }

    this->setFixedSize(404,50);
    this->setObjectName(address);
    _MStatus = Status::Nomal;
    _clicked = false;
    _pressFlag = false;

    _MDev = devAttr;
    InitMemberVariables();
}

QDevItem::~QDevItem()
{

}

void QDevItem::initMap() {
    typeMap = new QMap<int, QString>();
    typeMap->insert(Type::Phone, QString("phone-symbolic"));
    typeMap->insert(Type::Computer, QString("video-display-symbolic"));
    typeMap->insert(Type::Headset, QString("audio-headphones-symbolic"));     //    typeMap->insert(Type::Headset, QString("audio-headset-symbolic"));
    typeMap->insert(Type::Headphones, QString("audio-headphones-symbolic"));
    typeMap->insert(Type::AudioVideo, QString("audio-speakers-symbolic"));
    typeMap->insert(Type::Keyboard, QString("input-keyboard-symbolic"));
    typeMap->insert(Type::Mouse, QString("input-mouse-symbolic"));
    typeMap->insert(Type::Tablet, QString("tablet-symbolic"));
}

void QDevItem::attrChangedSlot(QMap<QString, QVariant> devAttr) {
    if (_MDev.value("Connected").toBool() != devAttr.value("Connected").toBool()) {
        _iconTimer->stop();
        _clicked = false;
    }

    bool isAlreadyLow = false;
    if (m_Battery <= powerPercentage)
        isAlreadyLow = true;

    _MDev = devAttr;
    if (_MDev.value("Battery").toInt() == -1) {
        update();
        return;
    }

    m_Battery = _MDev.value("Battery").toInt();
    if (m_Battery <= powerPercentage && !isAlreadyLow) {
        QDBusInterface iface(MEDIA_SOUND_SERVICE,
                             MEDIA_SOUND_PATH,
                             MEDIA_SOUND_INTERFACE,
                             QDBusConnection::sessionBus());
        if (iface.isValid())
            iface.call("playAlertSound", "battery-low");
    }
    update();
    KyDebug();
}

void QDevItem::InitMemberVariables()
{
    _iconTimer = new QTimer(this);

    connect(_iconTimer,&QTimer::timeout,this,[=]{
        if (iconFlag == 0)
            iconFlag = 7;
        iconFlag--;
        update();
    });
}

void QDevItem::setFocusStyle(bool val) {
    _focused = val;
    if (val)
        _MStatus = Status::Hover;
    else
        _MStatus = Status::Nomal;
    this->update();
}

void QDevItem::pressEnterCallback() {
    _MStatus = Status::Click;
    this->update();
    MouseClickedFunc();
    _MStatus = Status::Hover;
    this->update();
}

void QDevItem::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (_focused)
        return;
    _leaved = false;
    _MStatus = Status::Hover;
    update();
}

void QDevItem::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (_focused)
        return;
    _leaved = true;
    _MStatus = Status::Nomal;
    update();
}

void QDevItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !_clicked) {
        _pressFlag = true;
        _MStatus = Status::Click;
        this->update();
    }
}

void QDevItem::mouseReleaseEvent(QMouseEvent *event)
{
    KyDebug();
    if (_MStatus != Status::Click)
        return;
    if (event->button() == Qt::LeftButton && _pressFlag) {
        if (_leaved)
            _MStatus = Status::Nomal;
        else
            _MStatus = Status::Hover;
        MouseClickedFunc();
    }
}

void QDevItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(Qt::transparent));

    DrawBackground(painter);
    DrawStatusIcon(painter);
    DrawText(painter);
    DrawBattery(painter);

    if (!_focused)
        return;
    QPainter pt(this);
    QColor color = QColor("#818181");
    pt.setPen(color);
    QRect rect = this->rect();
    rect.setWidth(this->rect().width());
    rect.setHeight(this->rect().height());
    pt.setRenderHint(QPainter::Antialiasing);
    pt.drawRoundedRect(rect,8,8,Qt::AbsoluteSize);
}

/************************************************
 * @brief  根据设备状态返回自定义控件的背景色
 * @param  null
 * @return 返回类型为QColor
*************************************************/
QColor QDevItem::getPainterBrushColor()
{
    QColor color;
    switch (_MStatus) {
    case Status::Nomal:
        if(styleGSetting->get("style-name").toString() != "lingmo-light"){
            color = QColor(QPalette::Base);
            color.setAlpha(0);
        }
        else
            color = QColor(255,255,255,0);
        break;
    case Status::Hover:
        if(styleGSetting->get("style-name").toString() != "lingmo-light")
            color = QColor(55, 55, 59, tran);
        else
            color = QColor(217,217,217,tran);
        break;
    case Status::Click:
        if(styleGSetting->get("style-name").toString() != "lingmo-light")
            color = QColor(102, 102, 102, tran);
        else
            color = QColor(209, 209, 209,tran);
        break;
    default:
        if(styleGSetting->get("style-name").toString() != "lingmo-light"){
            color = QColor(QPalette::Base);
            color.setAlpha(0);
        } else
            color = QColor(223,223,223,tran);
        break;
    }

    return color;
}

/************************************************
 * @brief  根据设备状态返回自定义控件的设备状态的颜色
 * @param  null
 * @return 返回类型为QColor
*************************************************/
QColor QDevItem::getDevStatusColor()
{
    QColor color;
    if (_MDev.value("Connected").toBool()) {
        if (_MStatus == Status::Click)
            color = this->palette().color(QPalette::Active, QPalette::Highlight).darker(105);
        else
            color = this->palette().color(QPalette::Active, QPalette::Highlight);
    } else {
        if(styleGSetting->get("style-name").toString() != "lingmo-light") {
            if (_MStatus == Status::Click)
                color = QColor("#444444");
            else
                color = QColor("#666666");
        } else {
            if (_MStatus == Status::Click)
                color = QColor("#E0E0E0");
            else
                color = QColor("#DFDFDF");
        }
        color.setAlpha(tran);
    }

    return color;
}

/************************************************
 * @brief  根据设备类型返回对应的主题图标
 * @param  null
 * @return 返回类型为QPixmap的图标
*************************************************/
QPixmap QDevItem::getDevTypeIcon()
{
    QIcon icon;
    if (_clicked) {
        icon = QIcon::fromTheme("lingmo-loading-" + QString::number(iconFlag) + "-symbolic");
    } else {
        if (_MDev.contains("Type") && typeMap->contains(_MDev.value("Type").toInt()))
            icon = QIcon::fromTheme(typeMap->value(_MDev.value("Type").toInt()));
        else
            icon = QIcon::fromTheme("bluetooth-symbolic");
    }
    return icon.pixmap(18,18);
}

/************************************************
 * @brief  绘制自定义控件的背景色
 * @param  painter
 * @return null
*************************************************/
void QDevItem::DrawBackground(QPainter &painter)
{
    painter.save();
    painter.setBrush(getPainterBrushColor());
    painter.drawRoundedRect(this->rect(),8,8,Qt::AbsoluteSize);
    painter.restore();
}

/************************************************
 * @brief  绘制自定义控件的设备状态图标
 * @param  painter
 * @return null
*************************************************/
void QDevItem::DrawStatusIcon(QPainter &painter)
{
    QPixmap devpixmap;

    painter.save();
    painter.setBrush(getDevStatusColor());
    painter.drawEllipse(16,7,36,36);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if(styleGSetting->get("style-name").toString() != "lingmo-light") {
        painter.setPen(QColor(Qt::white));
        devpixmap = Config::loadSvg(getDevTypeIcon(), Config::PixmapColor::WHITE);
    } else {
        painter.setPen(QColor(Qt::black));
        if (_MDev.value("Connected").toBool())
            devpixmap = Config::loadSvg(getDevTypeIcon(), Config::PixmapColor::WHITE);
        else
            devpixmap = getDevTypeIcon();
    }

    style()->drawItemPixmap(&painter, QRect(25, 16, 18, 18), Qt::AlignCenter, devpixmap);
    painter.restore();
}

/************************************************
 * @brief  绘制自定义控件的文字
 * @param  painter
 * @return null
*************************************************/
void QDevItem::DrawText(QPainter &painter)
{
    painter.save();
    if(styleGSetting->get("style-name").toString() != "lingmo-light")
        painter.setPen(QColor(Qt::white));
    else
        painter.setPen(QColor(Qt::black));

    QString newName;
    double fontSize = styleGSetting->get("system-font-size").toDouble() - 11;
    QFont ft;
    ft.setPixelSize(18 + fontSize);
    QFontMetrics fontMetrics(ft);
    QString name;
    name = _MDev.value("Name").toString();
    if (_MDev.contains("ShowName"))
        if (_MDev.value("ShowName").toString() != QString(""))
            name = _MDev.value("ShowName").toString();
    newName = fontMetrics.elidedText(name, Qt::ElideRight, 280);
    if (name.size() != newName.size())
        this->setToolTip(name);

    if (fontSize >= 14)
        painter.drawText(62,8,280,30,Qt::AlignLeft|Qt::AlignVCenter, newName);
    else
        painter.drawText(62,12,280,28,Qt::AlignLeft|Qt::AlignVCenter, newName);

    painter.restore();
}

/************************************************
 * @brief  绘制电池
 * @param  painter
 * @return null
*************************************************/
void QDevItem::DrawBattery(QPainter &painter)
{
    if (m_Battery == -1 || !_MDev.value("Connected").toBool()) {
        m_pBatteyIconLabel->setVisible(false);
        return;
    }
    painter.save();
    QString batteryIcon = QString("battery-level-") + QString::number(m_Battery / 10 * 10) + QString("-symbolic");
    QString batteryString = QString::number(m_Battery) + QString("%");
    QPixmap batteryPixmap = QIcon::fromTheme(batteryIcon).pixmap(20, 20);
    m_pBatteyIconLabel->setPixmap(batteryPixmap);
    m_pBatteyIconLabel->setVisible(true);

//    style()->drawItemPixmap(&painter, QRect(320, 14, 20, 20), Qt::AlignVCenter | Qt::AlignLeft, batteryPixmap);

//    QLabel *a = new QLabel(this);
//    a->setPixmap(batteryPixmap);
//    a->setProperty("useIconHighlihtEffect", 0x2);
//    QVBoxLayout *lay = new QVBoxLayout(this);
//    lay->addWidget(a,0,Qt::AlignRight);

    if(styleGSetting->get("style-name").toString() != "lingmo-light") {
        painter.setPen(QColor(Qt::white));
        batteryPixmap = Config::loadSvg(QIcon::fromTheme(batteryIcon).pixmap(20,20), Config::PixmapColor::WHITE);
    } else {
        painter.setPen(QColor(Qt::black));
        batteryPixmap = Config::loadSvg(QIcon::fromTheme(batteryIcon).pixmap(20,20), Config::PixmapColor::BLACK);
    }

    if (styleGSetting->get("system-font-size").toDouble() >= 14)
        painter.drawText(344,8,52,28,Qt::AlignVCenter | Qt::AlignLeft, batteryString);
    else
        painter.drawText(344,12,52,28,Qt::AlignLeft, batteryString);
    //style()->drawItemPixmap(&painter, QRect(320, 14, 20, 20), Qt::AlignVCenter | Qt::AlignLeft, batteryPixmap);
    painter.restore();
}

/************************************************
 * @brief  当鼠标在该自定义控件点击时，执行该函数
 * @param  null
 * @return null
*************************************************/
void QDevItem::MouseClickedFunc()
{
    emit devConnect(!_MDev.value("Connected").toBool());
    startIconTimer();
}

void QDevItem::startIconTimer() {
    _clicked = true;
    _pressFlag = false;
    //设置_iconTimer定时器开始，来实现loading效果
    if (!_iconTimer->isActive()) {
        _iconTimer->start(110);

        //以下这个只执行一次的定时器，用来设置点击后加载动画的持续时间
        QTimer::singleShot(7000,this,[=]{
            if (_clicked) {
                _iconTimer->stop();
                _clicked = false;
                update();
            }
        });
    }
}

bool QDevItem::isConnected() {
    return _MDev.value("Connected").toBool();
}

bool QDevItem::event(QEvent *event) {
    QWidget::event(event);
    if(styleGSetting->get("style-name").toString() != "lingmo-default")
        return true;
    if (event->type() != QEvent::ToolTip)
        return true;
    QLabel *lToolTip = nullptr;
    lToolTip = this->findChild<QLabel*>("qtooltip_label");
    if (lToolTip != nullptr) {
        QPalette pal;
        pal.setColor(QPalette::ToolTipBase, QColor(38,38,38));
        pal.setColor(QPalette::ToolTipText, QColor(217,217,217));
        pal.setColor(QPalette::BrightText, QColor(255,255,255));
        lToolTip->setPalette(pal);
    }
    return true;
}

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

#include "activeconnectionwidget.h"
#include "config/xatom-helper.h"
#include "config/config.h"
#include <QPainterPath>

ActiveConnectionWidget::ActiveConnectionWidget(QString address, QString name, QString type, int rssi) :
    devAddr(address),
    devName(name),
    devType(type),
    devRSSI(rssi)
{
    if(QGSettings::isSchemaInstalled("org.lingmo.style")){
        settings = new QGSettings("org.lingmo.style");
        connect(settings,&QGSettings::changed,this,&ActiveConnectionWidget::GSettingsChanges);
    }

    const QByteArray transparency_id(TRANSPARENCY_SETTINGS);
    if(QGSettings::isSchemaInstalled(transparency_id)) {
        transparency = new QGSettings(transparency_id);
    }
    if(QGSettings::isSchemaInstalled(transparency_id)) {
        getTransparency();
        connect(transparency,&QGSettings::changed,this,&ActiveConnectionWidget::GSettingsChanges);
    }
    setProperty("useStyleWindowManager", false);
    setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground);
    //窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    this->setWindowTitle(tr("Bluetooth Connection"));

    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setFixedSize(420, 154);

    window_icon = new QLabel(this);
    window_icon->setPixmap(QIcon::fromTheme("bluetooth").pixmap(22,22));

    window_title = new QLabel(tr("Bluetooth Connections"),this);
    window_title->setFixedSize(300,20);

    window_close = new QPushButton(this);
    window_close->setFocusPolicy(Qt::NoFocus);
    window_close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    window_close->setProperty("isWindowButton", 0x2);
    window_close->setProperty("useIconHighlightEffect", 0x8);
    window_close->setFlat(true);
    connect(window_close, &QPushButton::clicked, this, &ActiveConnectionWidget::onClick_close_btn);

    tip_txt = new QLabel(this);
    tip_txt->setFixedWidth(380);
    QFontMetrics fontMetrics(tip_txt->font());
    QString tip = QString(tr("Found audio device \"") + devName + tr("\", connect it or not?"));
    QString _tip = fontMetrics.elidedText(tip,
                                         Qt::ElideMiddle,
                                         tip_txt->width());
    if (tip != _tip)
        tip_txt->setToolTip(tip);
    tip_txt->setText(_tip);

    checkBox_txt = new QLabel(this);
    checkBox_txt->setText(tr("No prompt"));
    actCnt_box = new QCheckBox(this);
    actCnt_box->setFocusPolicy(Qt::NoFocus);
    connect(actCnt_box, &QCheckBox::toggled, this, [=](bool checked) {
        if (QGSettings::isSchemaInstalled(GSETTING_SCHEMA_LINGMOBLUETOOH))
            QGSettings(GSETTING_SCHEMA_LINGMOBLUETOOH).set("active-connection", QVariant(!checked));
        if (mutex) {
            mutex = false;
            return;
        }
        mutex = true;
        QMap<QString, QVariant> attr;
        attr.insert("ActiveConnection", QVariant(!checked));
        Config::setDefaultAdapterAttr(attr);
    });

    connect_btn = new QPushButton(tr("Connect"),this);
    connect_btn->setFocusPolicy(Qt::NoFocus);
    connect(connect_btn, &QPushButton::clicked, this, &ActiveConnectionWidget::onClick_connect_btn);
    cancel_btn = new QPushButton(tr("Cancel"),this);
    cancel_btn->setFocusPolicy(Qt::NoFocus);
    connect(cancel_btn, &QPushButton::clicked, this, &ActiveConnectionWidget::onClick_cancel_btn);

    actCnt_box->setProperty("needTranslucent", true);
    connect_btn->setProperty("needTranslucent", true);
    cancel_btn->setProperty("needTranslucent", true);
    if(settings->get("style-name").toString() == "lingmo-default"){
        defaultStyleInit();
    }

    struct_pos pos;
    pos = Config::getWidgetPos(8,8,24,24, this->width());
    window_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(36, 10, 300, 20, this->width());
    window_title->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(388,8,24,24, this->width());
    window_close->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(24,48,380,28, this->width());
    tip_txt->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(24, 104, 18, 18, this->width());
    actCnt_box->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(48, 98, 200, 28, this->width());
    checkBox_txt->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(188,94,96,36, this->width());
    cancel_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(300,94,96,36, this->width());
    connect_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    QRect   desktop;
    QDesktopWidget *desktop_widget = QApplication::desktop();
    desktop = desktop_widget->availableGeometry();
    this->move(QPoint(desktop.right() - this->width() - 4,desktop.bottom() - this->height() - 4));
    this->setFocus();

    const KWindowInfo info(this->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager))
        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
}

ActiveConnectionWidget::~ActiveConnectionWidget()
{
    KyDebug();
}

void ActiveConnectionWidget::autoConnChanged(bool activate) {
    this->actCnt_box->setChecked(!activate);
    if (mutex) {
        mutex = false;
        return;
    }
    mutex = true;
}

void ActiveConnectionWidget::getTransparency() {
    double proportion = transparency->get(TRANSPARENCY_KEY).toDouble();
    proportion = ((proportion > 0.85) ? 1 : (proportion + 0.15));
    KyDebug() << proportion;
    tran = proportion * 255;
}

void ActiveConnectionWidget::onClick_close_btn()
{
    KyDebug() << "close";
    onClick_cancel_btn();
}

void ActiveConnectionWidget::onClick_connect_btn()
{
    KyDebug() << "accpet";
    emit replyActiveConnection(devAddr, true);
    this->hide();
    this->deleteLater();
}

void ActiveConnectionWidget::onClick_cancel_btn()
{
    KyDebug() << "cancel";
    emit replyActiveConnection(devAddr, false);
    this->hide();
    this->deleteLater();
}

void ActiveConnectionWidget::defaultStyleInit() {
    QPalette button_pl = connect_btn->palette();
    QPalette text_pl;
    QPalette close_pl = window_close->palette();
    QColor button_col = QColor(255,255,255);
    button_pl.setColor(QPalette::BrightText, button_col);
    button_pl.setColor(QPalette::Text, QColor(217, 217, 217));
    button_pl.setColor(QPalette::ButtonText, QColor(217, 217, 217));
    button_pl.setColor(QPalette::WindowText, QColor(217, 217, 217));
    text_pl.setColor(QPalette::WindowText, QColor(217, 217, 217));
    close_pl.setColor(QPalette::WindowText, QColor(217,217,217));
    connect_btn->setPalette(button_pl);
    cancel_btn->setPalette(button_pl);
    actCnt_box->setPalette(button_pl);
    window_title->setPalette(text_pl);
    window_close->setPalette(close_pl);
    tip_txt->setPalette(text_pl);
    checkBox_txt->setPalette(text_pl);
}

void ActiveConnectionWidget::GSettingsChanges(const QString &key)
{
    KyDebug() << key;
    if(key == "styleName" || key == QString(TRANSPARENCY_KEY)){
        getTransparency();
        if(settings->get("style-name").toString() == "lingmo-default"){
            if (this->hasFocus())
                defaultStyleInit();
            else {
                QPalette button_pl = connect_btn->palette();
                QPalette text_pl;
                QPalette close_pl = window_close->palette();
                QColor button_col = QColor(255,255,255);
//                button_col.setAlphaF(0.5);
                button_pl.setColor(QPalette::BrightText, button_col);
                button_pl.setColor(QPalette::ButtonText, QColor(217, 217, 217));
                text_pl.setColor(QPalette::WindowText, QColor(255, 255, 255, 255 * 0.55));
                close_pl.setColor(QPalette::WindowText, QColor(255, 255, 255, 255 * 0.55));
                connect_btn->setPalette(button_pl);
                cancel_btn->setPalette(button_pl);
                actCnt_box->setPalette(button_pl);
                window_title->setPalette(text_pl);
                window_close->setPalette(close_pl);
                tip_txt->setPalette(text_pl);
                checkBox_txt->setPalette(text_pl);
            }
        } else {
            QPalette pl;
            connect_btn->setPalette(pl);
            cancel_btn->setPalette(pl);
            actCnt_box->setPalette(pl);
            window_title->setPalette(pl);
            window_close->setPalette(pl);
            tip_txt->setPalette(pl);
            checkBox_txt->setPalette(pl);
        }
        this->update();
    }
    if (key == "systemFontSize") {
        QFontMetrics fontMetrics(tip_txt->font());
        QString tip = fontMetrics.elidedText(QString(tr("Found audio device \"") + devName + tr("\", connect it or not?")),
                                             Qt::ElideMiddle,
                                             tip_txt->width());
        tip_txt->setText(tip);
    }
}

QPushButton* ActiveConnectionWidget::getButton() {
    switch (pressCnt) {
    case 1:
        return connect_btn;
    case 2:
        return cancel_btn;
    case 3:
        return window_close;
    default:
        return nullptr;
    }
}

void ActiveConnectionWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    QColor col;
    if(settings->get("style-name").toString() == "lingmo-default"){
        if (hasFocus())
            col = QColor(35, 36, 38);
        else
            col = QColor(26, 26, 26);
    } else {
        col = this->palette().color(QPalette::Active, QPalette::Window);
    }
    QPainterPath rectPath;
    col.setAlpha(tran);
    rectPath.addRoundedRect(this->rect(),12,12);
    painter.setBrush(col);
    painter.drawPath(rectPath);

    QPainter pt(this);
    QColor color = QColor("#818181");
    pt.setPen(color);
    selectedBtn = getButton();
    if (selectedBtn == nullptr) {
        QWidget::paintEvent(event);
        return;
    }
    QRect rect = selectedBtn->geometry();
    pt.setRenderHint(QPainter::Antialiasing);
    pt.drawRect(rect);
    QWidget::paintEvent(event);
}

void ActiveConnectionWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Tab:
        KyDebug()<< "press Tab";
        ++pressCnt;
        if (pressCnt == 4)
            pressCnt = 1;
        this->update();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        KyDebug()<< "press Enter";
        if (selectedBtn == nullptr)
            return;
        selectedBtn->clicked();
        break;
    case Qt::Key_Escape:
        KyDebug()<< "press Esc";
        if (pressCnt == 0)
            this->close();
        pressCnt = 0;
        this->update();
        break;
    default:
        break;
    }
}

void ActiveConnectionWidget::focusOutEvent(QFocusEvent *event) {
    KyDebug() << "focus in event";
    if(settings->get("style-name").toString() == "lingmo-default"){
        QPalette button_pl = connect_btn->palette();
        QPalette text_pl;
        QPalette close_pl = window_close->palette();
        QColor button_col = QColor(255,255,255);
//        button_col.setAlphaF(0.5);
        button_pl.setColor(QPalette::BrightText, button_col);
        button_pl.setColor(QPalette::ButtonText, QColor(217, 217, 217));
        text_pl.setColor(QPalette::WindowText, QColor(255, 255, 255, 255 * 0.55));
        close_pl.setColor(QPalette::WindowText, QColor(255, 255, 255, 255 * 0.55));
        connect_btn->setPalette(button_pl);
        cancel_btn->setPalette(button_pl);
        actCnt_box->setPalette(button_pl);
        window_title->setPalette(text_pl);
        window_close->setPalette(close_pl);
        tip_txt->setPalette(text_pl);
        checkBox_txt->setPalette(text_pl);
    }
    this->update();
    QWidget::focusOutEvent(event);
}

void ActiveConnectionWidget::focusInEvent(QFocusEvent *event) {
    KyDebug() << "focus out event";
    if(settings->get("style-name").toString() == "lingmo-default"){
        defaultStyleInit();
    }
    QWidget::focusInEvent(event);

}

void ActiveConnectionWidget::dragEnterEvent(QDragEnterEvent *event) {
    return;
}

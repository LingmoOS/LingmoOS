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
#include "singlepage.h"
#include <qsettings.h>
#include <QDBusInterface>
#include <QLabel>
#include <QApplication>
#include <QDBusReply>
#include <QPainterPath>
#include <KWindowEffects>

#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME  "styleName"

SinglePage::SinglePage(QWidget *parent) : QWidget(parent)
{
    setThemePalette();
    initUI();
    initWindowProperties();
    initTransparency();
    initWindowTheme();
}

SinglePage::~SinglePage()
{
}

void SinglePage::initUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    m_mainLayout->setSpacing(MAIN_LAYOUT_SPACING);
    this->setLayout(m_mainLayout);

    m_titleFrame = new QFrame(this);
    m_titleFrame->setFixedHeight(TITLE_FRAME_HEIGHT);
    m_titleLayout = new QHBoxLayout(m_titleFrame);
    m_titleLayout->setContentsMargins(TITLE_LAYOUT_MARGINS);
    m_titleFrame->setLayout(m_titleLayout);
    m_titleLabel = new QLabel(m_titleFrame);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();

    m_netDivider = new Divider(true, this);

    m_listFrame = new QFrame(this);
    m_listLayout = new QVBoxLayout(m_listFrame);
    m_listLayout->setContentsMargins(NET_LAYOUT_MARGINS);
    m_listFrame->setLayout(m_listLayout);
    m_listWidget = new QListWidget(m_listFrame);
    m_listLayout->addWidget(m_listWidget);
    m_listWidget->setProperty("needTranslucent", true);

    m_setDivider = new Divider(true, this);

    m_settingsFrame = new QFrame(this);
    m_settingsFrame->setFixedHeight(TITLE_FRAME_HEIGHT);
    m_settingsLayout = new QHBoxLayout(m_settingsFrame);
    m_settingsLayout->setContentsMargins(SETTINGS_LAYOUT_MARGINS);
    m_settingsLabel = new KyLable(m_settingsFrame);
    m_settingsLabel->setCursor(Qt::PointingHandCursor);
    m_settingsLabel->setText(tr("Settings"));
    m_settingsLabel->setScaledContents(true);
    m_settingsLayout->addWidget(m_settingsLabel);
    m_settingsLayout->addStretch();
    m_settingsFrame->setLayout(m_settingsLayout);

    m_mainLayout->addWidget(m_titleFrame);
    m_mainLayout->addWidget(m_netDivider);
    m_mainLayout->addWidget(m_listFrame);
    m_mainLayout->addWidget(m_setDivider);
    m_mainLayout->addWidget(m_settingsFrame);
}

void SinglePage::initWindowProperties()
{
    QPalette pal = m_listFrame->palette();
    pal.setBrush(QPalette::Base, QColor(0,0,0,0));        //背景透明
    m_listFrame->setPalette(pal);

    this->setFixedWidth(MAX_WIDTH);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setProperty("useStyleWindowManager", false); //禁用拖动
    //绘制毛玻璃特效
    QString platform = QGuiApplication::platformName();
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
    {
        QPainterPath path;
        auto rect = this->rect();
        path.addRect(rect);
        KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(path.toFillPolygon().toPolygon()));   //背景模糊
    }
}

/**
 * @brief SinglePage::initWindowTheme 初始化窗口主题并创建信号槽
 */
void SinglePage::initWindowTheme()
{
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
        m_styleGsettings = new QGSettings(style_id, QByteArray(), this);
        connect(m_styleGsettings, &QGSettings::changed, this, &SinglePage::onThemeChanged);
    } else {
        qWarning() << "Gsettings interface \"org.lingmo.style\" is not exist!" << Q_FUNC_INFO << __LINE__;
    }
}

void SinglePage::showDesktopNotify(const QString &message, QString soundName)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QStringList actions;  //跳转动作
    actions.append("default");
    actions.append("lingmo-vpn");          //默认动作：点击消息体时打开灵墨录音
    QMap<QString, QVariant> hints;
    if (!soundName.isEmpty()) {
        hints.insert("sound-name",soundName); //添加声音
    }
    QList<QVariant> args;
    args<<(tr("Lingmo VPN"))
       <<((unsigned int) 0)
       <<QString("gnome-dev-ethernet")
       <<tr("lingmo vpn applet desktop message") //显示的是什么类型的信息
       <<message //显示的具体信息
       <<actions
       <<hints
       <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}

void SinglePage::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    QColor col = this->palette().window().color();

    QPainterPath rectPath;

    col.setAlphaF(m_transparency);
    rectPath.addRect(this->rect());

    painter.setBrush(col);
    painter.drawPath(rectPath);
    KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(rectPath.toFillPolygon().toPolygon()));   //背景模糊
}

void SinglePage::initTransparency()
{
    if(QGSettings::isSchemaInstalled(QByteArray(TRANSPARENCY_GSETTINGS))) {
        m_transGsettings = new QGSettings(QByteArray(TRANSPARENCY_GSETTINGS));
        if(m_transGsettings->keys().contains(TRANSPARENCY)) {
            m_transparency = m_transGsettings->get(TRANSPARENCY).toDouble() + 0.15;
            m_transparency = (m_transparency > 1) ? 1 : m_transparency;
            connect(m_transGsettings, &QGSettings::changed, this, &SinglePage::onTransChanged);
        }
    }
}

void SinglePage::onTransChanged()
{
    m_transparency = m_transGsettings->get("transparency").toDouble() + 0.15;
    m_transparency = (m_transparency > 1) ? 1 : m_transparency;
    paintWithTrans();
}

void SinglePage::onThemeChanged(const QString &key)
{
    if (key == COLOR_THEME) {
        setThemePalette();
        paintWithTrans();
        Q_EMIT qApp->paletteChanged(qApp->palette());
    }  else if ("themeColor" == key) {
        setThemePalette();
    }
}

void SinglePage::setThemePalette()
{
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
    pal.setColor(QPalette::Background, pal.base().color());
    this->setPalette(pal);
}

void SinglePage::paintWithTrans()
{
    QPalette pal = this->palette();
    QColor color = this->palette().base().color();
    color.setAlphaF(m_transparency);
    pal.setColor(QPalette::Window, color);
    this->setPalette(pal);
}

void SinglePage::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        this->hide();
    }
    return QWidget::keyPressEvent(event);
}

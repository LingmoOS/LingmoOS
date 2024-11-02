/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kleftsideitem.h"

#include <QIcon>
#include <QPixmap>
#include <QVariant>
#include <QEvent>

#include "macro.h"
#include "util.h"
#include "imageutil.h"

KLeftSideItem::KLeftSideItem(QWidget *parent)
    : QPushButton(parent)
{
    m_strIcon = "";
    m_strText = "";
    this->setFlat(true);
    initUI();
}

KLeftSideItem::KLeftSideItem(QString strText, QString strIcon, QWidget *parent)
    : QPushButton(parent)
{
    m_strIcon = strIcon;
    m_strText = strText;
    this->setFlat(true);
    initUI();
}

KLeftSideItem::~KLeftSideItem()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void KLeftSideItem::initUI()
{
    installEventFilter(this);
    setProperty("useButtonPalette", true);
    m_layoutMain = new QHBoxLayout();
    m_layoutMain->setContentsMargins(16,6,16,6);
    m_layoutMain->setSpacing(8);

    m_labelIcon = new QLabel();
    m_layoutMain->addWidget(m_labelIcon);

    m_labelText = new QLabel();
    setText(m_strText);
    m_layoutMain->addWidget(m_labelText);

    m_layoutMain->addStretch();
    this->setLayout(m_layoutMain);
    initStyleTheme();
    setIcon(m_strIcon);


     if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
         QGSettings *qtSettings = new QGSettings("org.lingmo.style", QByteArray(), this);
         if (qtSettings->keys().contains("styleName")) {
             hoverColor = btnHoverColor(qtSettings->get("style-name").toString(), true);
             clickColor = btnHoverColor(qtSettings->get("style-name").toString(), false);
             this->setStyleSheet(QString("KLeftSideItem:hover{background-color:%1;border-radius: 6px;}"
                                         "KLeftSideItem:pressed{background-color:%2;border-radius: 6px;}").arg(hoverColor).arg(clickColor));
        
             connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
                 if (key == "styleName") {
                     hoverColor = this->btnHoverColor(qtSettings->get("style-name").toString(), true);
                     clickColor = btnHoverColor(qtSettings->get("style-name").toString(), false);
                     if (!this->isChecked())
                        this->setStyleSheet(QString("KLeftSideItem:hover{background-color:%1;border-radius: 6px;}"
                                                     "KLeftSideItem:pressed{background-color:%2;border-radius: 6px;}").arg(hoverColor).arg(clickColor));
                 } else if (key == "themeColor" && this->isChecked()) {
                     this->toggled(true);
                 }
             });
         }
     }

     connect(this, &QPushButton::toggled, this, [=](bool checked) {
         if (checked) {
             this->setStyleSheet("KLeftSideItem:checked{background-color: palette(highlight);border-radius: 6px;}");
             m_labelText->setStyleSheet("color: white");
             m_isNormal = false;
         } else {
             this->setStyleSheet(QString("KLeftSideItem:hover{background-color:%1;border-radius: 6px;}"
                                         "KLeftSideItem:pressed{background-color:%2;border-radius: 6px;}").arg(hoverColor).arg(clickColor));
             if (!m_isHover) {
                 m_isNormal = true;
             }
             m_labelText->setStyleSheet("color: palette(windowtext)");
         }
         setIcon(m_strIcon);
     });
}

void KLeftSideItem::initStyleTheme()
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        m_styleSettings = new QGSettings(idd);
    }
    if (m_styleSettings) {
        connect(m_styleSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                auto styleNameValue = m_styleSettings->get("styleName");
                if (styleNameValue.isValid()) {
                    m_strThemeName = styleNameValue.toString();
                    setIcon(m_strIcon);
                }
            } else if (key == "systemFontSize" || key == "systemFont") {
                auto styleFontSizeValue = m_styleSettings->get("systemFontSize");
                if (styleFontSizeValue.isValid()) {
                    this->setText(m_strText);
                }
                QFont fontTitle = this->font();
                m_labelText->setFont(fontTitle);
            }
        });
        auto styleNameValue = m_styleSettings->get("styleName");
        if (styleNameValue.isValid()) {
            m_strThemeName = styleNameValue.toString();
        }
        auto styleFontSizeValue = m_styleSettings->get("systemFontSize");
        if (styleFontSizeValue.isValid()) {
            this->setText(m_strText);
        }
    }
}

void KLeftSideItem::setText(QString &strText)
{
    m_strText = strText;
    if (!m_strText.isEmpty()) {
        QString ShowValue = getElidedText(m_labelText->font(), m_strText, width()-60); // 16+16+8+16 + 4
        m_labelText->setText(ShowValue);
        if (ShowValue != m_strText) {
            setToolTip(m_strText);
        } else {
            setToolTip("");
        }
        m_labelText->show();
    } else {
        m_labelText->hide();
    }
}

void KLeftSideItem::setIcon(QString &strIcon)
{
    m_strIcon = strIcon;
    QPixmap pixmap;
    if (m_strThemeName == "lingmo-dark" || m_strThemeName == "lingmo-black") {
        if (m_isNormal) {
            pixmap = ImageUtil::loadSvg(m_strIcon, "white", 16);
        } else {
            pixmap = ImageUtil::loadSvg(m_strIcon, "white", 16);
        }
    } else { // "lingmo-default" "lingmo-white" "lingmo-light" "lingmo"
        if (m_isNormal) {
            pixmap = ImageUtil::loadSvg(m_strIcon, "black", 16);
        } else {
            pixmap = ImageUtil::loadSvg(m_strIcon, "white", 16);
        }
    }
    if (!pixmap.isNull()) {
        m_labelIcon->setPixmap(pixmap);
        m_labelIcon->show();
    } else {
        m_labelIcon->hide();
    }
}

bool KLeftSideItem::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this) {
        switch (event->type()) {
        case QEvent::HoverEnter:
            m_isHover = true;
            setIcon(m_strIcon);
            break;
        case QEvent::HoverLeave:
            m_isHover = false;
            setIcon(m_strIcon);
            break;
        default:
            break;
        }
    }
    return QPushButton::eventFilter(obj, event);
}

QString KLeftSideItem::btnHoverColor(QString styleName, bool hoverFlag)
{
    QColor color1 = palette().color(QPalette::Active, QPalette::Button);
    QColor color2 = palette().color(QPalette::Active, QPalette::BrightText);
    QColor color;
    qreal r,g,b,a;
    QString hoverColor;
    if (((styleName.contains("dark") || styleName.contains("black")) && hoverFlag) ||
        ((!styleName.contains("dark") && !styleName.contains("black")) && !hoverFlag)) {
        r = color1.redF() * 0.8 + color2.redF() * 0.2;
        g = color1.greenF() * 0.8 + color2.greenF() * 0.2;
        b = color1.blueF() * 0.8 + color2.blueF() * 0.2;
        a = color1.alphaF() * 0.8 + color2.alphaF() * 0.2;
    } else {
        r = color1.redF() * 0.95 + color2.redF() * 0.05;
        g = color1.greenF() * 0.95 + color2.greenF() * 0.05;
        b = color1.blueF() * 0.95 + color2.blueF() * 0.05;
        a = color1.alphaF() * 0.95 + color2.alphaF() * 0.05;
    }
    color = QColor::fromRgbF(r, g, b, a);
    hoverColor = QString("rgba(%1, %2, %3, %4)").arg(color.red())
                                                .arg(color.green())
                                                .arg(color.blue())
                                                .arg(color.alpha());
    return hoverColor;
}

/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "infowidget.h"
#include "xatomhelper.h"
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QDesktopServices>
#include <QGSettings>
#define LINGMO_STYLE_SCHEMA          "org.lingmo.style"
#define STYLE_NAME                 "styleName"
#define STYLE_NAME_KEY_DARK        "lingmo-dark"
#define STYLE_NAME_KEY_DEFAULT     "lingmo-default"
#define STYLE_NAME_KEY_BLACK       "lingmo-black"
#define STYLE_NAME_KEY_LIGHT       "lingmo-light"
#define STYLE_NAME_KEY_WHITE       "lingmo-white"

infoWidget::infoWidget(QWidget *parent) :
    QWidget(parent)
{
    // setWindowFlags(Qt::FramelessWindowHint);
// setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(388, 384);
    m_exitButton = new QPushButton(this);
    m_exitButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_exitButton->setFixedSize(24, 24);
    m_exitButton->move(356, 8);
    connect(m_exitButton, &QPushButton::clicked,
            this, &infoWidget::close);

    m_exitButton->setProperty("isWindowButton", 0x2);
    m_exitButton->setProperty("useIconHighlightEffect", 0x8);
    m_exitButton->setFlat(true);
    m_appIcon = new  QLabel(this);
    m_appIcon->setPixmap(QPixmap(QIcon::fromTheme("lingmo-screenshot").pixmap(QSize(96, 96))));
    m_appIcon->setAlignment(Qt::AlignHCenter);
    m_appIcon->setFixedSize(388, 100);
    m_appIcon->move(0, 56);

    m_appName = new QLabel(this);
    font.setFamily("Noto Sans CJK SC Bold");
    font.setBold(true);
    font.setPixelSize(20);
    m_appName->setFont(font);
    m_appName->setText(tr("screenshot"));
    m_appName->setAlignment(Qt::AlignHCenter);
    m_appName->setFixedSize(388, 30);
    m_appName->move(0, 154);

    m_appVersion = new QLabel(this);
    font.setFamily("Noto Sans CJK SC Regular");
    // font.setWeight(14);
    font.setBold(false);
    font.setPixelSize(14);
    m_appVersion->setFont(font);
    m_appVersion->setText(tr("version:v1.0.0"));
    m_appVersion->setFixedSize(388, 25);
    m_appVersion->move(0, 190);
    m_appVersion->setAlignment(Qt::AlignHCenter);

    m_Descript = new QLabel(this);
    font.setFamily("Noto Sans CJK SC Regular");
    font.setBold(false);
    font.setPixelSize(14);
    m_Descript->setFont(font);
    m_Descript->setText(tr("Screenshot is an easy to use application."
                           "that supports the basic screenshot function,"
                           "but also provides the draw rectangle tool, "
                           "draw a circle tool, blur, add annotations, "
                           "add text and other functions"));
    QString locale = QLocale::system().name();
    m_Descript->setAlignment(Qt::AlignJustify);
    m_Descript->setWordWrap(true);
    m_Descript->setMinimumWidth(324);
    m_Descript->setFrameShape(QFrame::NoFrame);
    m_Descript->move(32, 238);

    m_EmailInfo = new QLabel(this);
    connect(m_EmailInfo, &QLabel::linkActivated, this, [=](const QString url) {
        QDesktopServices::openUrl(QUrl(url));
    });
    m_EmailInfo->setFixedSize(350, 32);
    m_EmailInfo->setFont(font);
    if (locale == "zh_CN") {
        m_Descript->setFixedHeight(120);
        m_EmailInfo->move(32, 310);
    } else {
        m_Descript->setFixedHeight(140);
        setFixedSize(388, 410);
        m_EmailInfo->move(32, 350);
    }

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);
    listenToGsettings();
}

void infoWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // 反锯齿;
    painter.setBrush(QBrush(this->palette().color(QPalette::Base).light(150)));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    painter.drawRoundedRect(rect, 6, 6);
    // 也可用QPainterPath 绘制代替 painter.drawRoundedRect(rect, 15, 15);
}

void infoWidget::listenToGsettings()
{
    const QByteArray styleID(LINGMO_STYLE_SCHEMA);
    QStringList stylelist;
    if (QGSettings::isSchemaInstalled(styleID)) {
        QGSettings *styleLINGMO = new QGSettings(styleID, QByteArray(), this);
        stylelist << STYLE_NAME_KEY_DARK << STYLE_NAME_KEY_BLACK;
        // <<STYLE_NAME_KEY_DEFAULT;
        if (stylelist.contains(styleLINGMO->get(STYLE_NAME).toString())) {
            m_EmailInfo->setText(tr("SUPPORT:%1").arg(
                                     "<a href= \"mailto://support@kylinos.cn\" style=\"color:white\">support@kylinos.cn</a>"));
        } else {
            m_EmailInfo->setText(tr("SUPPORT:%1").arg(
                                     "<a href= \"mailto://support@kylinos.cn\" style=\"color:black\">support@kylinos.cn</a>"));
        }
        connect(styleLINGMO, &QGSettings::changed, this, [=](const QString &key)
        {
            if (key == STYLE_NAME) {
                if (stylelist.contains(styleLINGMO->get(
                                           STYLE_NAME).toString())) {
                    m_EmailInfo->setText(tr("SUPPORT:%1").arg(
                                             "<a href= \"mailto://support@kylinos.cn\" style=\"color:white\">support@kylinos.cn</a>"));
                } else {
                    m_EmailInfo->setText(tr("SUPPORT:%1").arg(
                                             "<a href= \"mailto://support@kylinos.cn\" style=\"color:black\">support@kylinos.cn</a>"));
                }
            }
        });
    }
}

infoWidget::~infoWidget()
{
}

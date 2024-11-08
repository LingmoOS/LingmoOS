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
#include "shortcutwidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QPainter>
#include <QPushButton>
#define ORG_LINGMO_STYLE            "org.lingmo.style"

ShortCutWidget::ShortCutWidget(QWidget *parent) :
    QWidget(parent)
{
    setWindowIcon(QIcon::fromTheme("lingmo-screenshot"));
    const QByteArray style_id(ORG_LINGMO_STYLE);
    if(QGSettings::isSchemaInstalled(style_id)){
        m_styleSettings = new QGSettings(style_id);
    }
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);

    m_titleIcon_label = new  QLabel(this);
    m_titleIcon_label->setPixmap(QPixmap(QIcon::fromTheme("lingmo-screenshot").pixmap(QSize(24,
                                                                                           24))));
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ"){
        m_titleIcon_label->move(356, 8);
    }else{
        m_titleIcon_label->move(8, 8);
    }
    m_titleName_label = new QLabel(this);
    m_titleName_label->setText(tr("Screenshot"));
    m_titleName_label->setAlignment(Qt::AlignLeft);
    m_titleName_label->setFixedSize(170, 25);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_titleName_label->move(186, 8);
    }else{
        m_titleName_label->move(40, 8);
    }
    m_exit_btn = new QPushButton(this);
    m_exit_btn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_exit_btn->setToolTip(tr("Close"));
    m_exit_btn->setFixedSize(24, 24);
    m_exit_btn->setIconSize(QSize(16, 16));
    m_exit_btn->setProperty("isWindowButton", 0x2);
    m_exit_btn->setProperty("useIconHighlightEffect", 0x8);
    m_exit_btn->setFlat(true);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_exit_btn->move(8, 8);
    }else{
        m_exit_btn->move(368, 8);
    }
    connect(m_exit_btn, &QPushButton::clicked,
            this, &ShortCutWidget::close);
    m_min_btn = new QPushButton(this);
    m_min_btn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_min_btn->setToolTip(tr("Min"));
    m_min_btn->setFixedSize(24, 24);
    m_min_btn->setIconSize(QSize(16, 16));
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_min_btn->move(40, 8);
    }else{
        m_min_btn->move(336, 8);
    }
    m_min_btn->setProperty("isWindowButton", 0x1);
    m_min_btn->setProperty("useIconHighlightEffect", 0x2);
    m_min_btn->setFlat(true);
    connect(m_min_btn, &QPushButton::clicked,
            // this, &ShortCutWidget::hide);
            this, &ShortCutWidget::showMinimized);
    tableName = new  QLabel(this);
    tableName->setText(tr("ShortCut"));
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        tableName->move(266, 56);
    }else{
        tableName->move(25, 56);
    }
    tableName->setFixedSize(100, 30);
    setFixedSize(400, 546);
    initLabel();
    setFont();
    connect(m_styleSettings,&QGSettings::changed,this,[=](const QString &key){
        if (key == "systemFontSize" || key == "systemFont") {
            setFont();
        }
    });
}

void ShortCutWidget:: initLabel()
{
    for (int i = 0; i < m_keys.size(); i++) {
        QLabel *lb = new  QLabel(this);
        lb->setFixedSize(352, 38);
        QLabel *labelchild1 = new QLabel;
        QLabel *labelchild2 = new QLabel;
        labelchild1->setText(tr(m_description.at(i)));
        labelchild1->setAlignment(Qt::AlignLeft);
        labelchild2->setText(tr(m_keys.at(i)));
        QHBoxLayout *m_layout = new  QHBoxLayout;
        labelchild1->setFixedSize(208, 38);
        m_layout->addWidget(labelchild1);
        m_layout->addWidget(labelchild2);
        lb->setLayout(m_layout);
        labels.append(lb);

        QString fontFamily = m_styleSettings->get("systemFont").toString();
        double fontSize = m_styleSettings->get("systemFontSize").toDouble();
        QFont f(fontFamily);
        f.setPointSizeF(fontSize);
        labelchild1->setFont(f);
        labelchild2->setFont(f);
        connect(m_styleSettings,&QGSettings::changed,this,[=](const QString &key){
           if (key == "systemFontSize" || key == "systemFont") {
               QString fontFamily = m_styleSettings->get("systemFont").toString();
               double fontSize = m_styleSettings->get("systemFontSize").toDouble();
               QFont f(fontFamily);
               f.setPointSizeF(fontSize);
               labelchild1->setFont(f);
               labelchild2->setFont(f);
           }
        });
    }
    for (int i = 0; i < m_keys.size(); i++) {
        labels.at(i)->move(25, 96+i*38);
    }
}

QVector<const char *> ShortCutWidget::m_keys =
{
    QT_TR_NOOP("Keypress"),
    "PrtSc",
    "Ctrl + PrtSc",
    "Shift + PrtSc",
    "←↓↑→",
    "SHIFT + ←↓↑→",
    "ESC",
    "CTRL + C",
    "CTRL + S",
    "CTRL + Z",
    QT_TR_NOOP("Mouse Wheel")
};

QVector <const char *> ShortCutWidget::m_description =
{
    QT_TR_NOOP("Description"),
    QT_TR_NOOP("Capture Full Screen"),
    QT_TR_NOOP("Capture Top Screen"),
    QT_TR_NOOP("Capture Screen selection"),
    QT_TR_NOOP("Move selection 1px"),
    QT_TR_NOOP("Resize selection 1px"),
    QT_TR_NOOP("Quit capture"),
    QT_TR_NOOP("Copy to clipboard"),
    QT_TR_NOOP("Save selection as a file"),
    QT_TR_NOOP("Undo the last modification"),
    QT_TR_NOOP("Change the tool's thickness")
};

void ShortCutWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(QBrush(QColor(95, 95, 95)));
    painter.setOpacity(0.10);
    painter.setPen(QColor(95, 95, 95));
    context.style_settings = new QGSettings("org.lingmo.style");
    int borderRadius =  "classical" == context.style_settings->get("widgetThemeName").toString() ? 0 : 6;
    for (int i = 0; i < m_keys.size(); i++) {
        if (0 != i%2) {
            painter.drawRoundedRect(QRect(labels.at(i)->geometry()), borderRadius, borderRadius, Qt::AbsoluteSize);
        }
    }
}

ShortCutWidget::~ShortCutWidget()
{
}

void ShortCutWidget::setFont()
{
    QString fontFamily = m_styleSettings->get("systemFont").toString();
    double fontSize = m_styleSettings->get("systemFontSize").toDouble();
    QFont f(fontFamily);
    f.setPointSizeF(fontSize + 2);
    tableName->setFont(f);
    f.setPointSizeF(fontSize);
    m_titleName_label->setFont(f);

}

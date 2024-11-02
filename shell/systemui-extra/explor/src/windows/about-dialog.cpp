/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "about-dialog.h"
#include "ui_about-dialog.h"
#include "xatom-helper.h"
#include <locale.h>
#include <libintl.h>
#include <QDebug>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

#ifdef LINGMO_SDK_WAYLANDHELPER
#include <lingmostylehelper/lingmostylehelper.h>
#else
#include <QX11Info>
#include "xatom-helper.h"
#endif

#ifdef LINGMO_SDK_QT_WIDGETS
AboutDialog::AboutDialog(QWidget *parent) :
    kdk::KAboutDialog(parent)
{
    setAppIcon(QIcon::fromTheme("system-file-manager"));
    setAppName(tr("Peony"));
    setAppSupport(tr("Service & Support: ") + "<a href=\"mailto://support@kylinos.cn\" style=\"color:"
                  + convertRGB16HexStr(palette().color(QPalette::ButtonText))
                  + ";\">support@kylinos.cn</a><br/>");
    setAppVersion(QString(tr("Version number: %1")).arg(getCurrentVersion()));
    setBodyText(tr("Peony is a graphical software to help users manage system files. "
                   "It provides common file operation functions for users, such as file viewing, "
                   "file copy, paste, cut, delete, rename, file selection, application opening, "
                   "file search, file sorting, file preview, etc. it is convenient for users to "
                   "manage system files intuitively on the interface."));
    setBodyTextVisiable(true);
}

#else
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent)
{
    this->initUI();
}

#endif

AboutDialog::~AboutDialog()
{
#ifndef LINGMO_SDK_QT_WIDGETS
    delete ui;
#endif
}

void AboutDialog::initUI()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setFixedWidth(420);

#ifdef LINGMO_SDK_WAYLANDHELPER
    kdk::LingmoUIStyleHelper::self()->removeHeader(this);
#else
    if (QX11Info::isPlatformX11()) {
        XAtomHelper::getInstance()->setLINGMODecoraiontHint(this->winId(), true);
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
#endif
    ui = new Ui::AboutDialog;
    ui->setupUi(this);

    ui->logoLabel->setPixmap(QIcon::fromTheme("system-file-manager").pixmap(24,24));

    ui->titleLabel->setText(tr("Peony"));

    ui->closeBtn->setFlat(true);
    ui->closeBtn->setProperty("isIcon", true);
    ui->closeBtn->setFixedSize(QSize(30, 30));
    ui->closeBtn->setProperty("isWindowButton", 2);
    ui->closeBtn->setProperty("useIconHighlightEffect", 0x8);
    ui->closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));

    ui->iconLabel->setPixmap(QIcon::fromTheme("system-file-manager").pixmap(96,96));
    auto font = qApp->font();
    QFont namefont = font;
    namefont.setBold(true);
    namefont.setPointSize(font.pointSize()*1.28);

    ui->nameLabel->setFont(namefont);
    ui->nameLabel->setText(tr("Peony"));

    ui->versionLabel->setFont(font);
    ui->versionLabel->setText(QString(tr("Version number: %1")).arg(getCurrentVersion()));

	ui->briefTextedit->setReadOnly(true);

    QTextCursor textCursor;
    QTextBlockFormat blockFormat;
    blockFormat.setLineHeight(font.pointSize()+10, QTextBlockFormat::SingleHeight);//font-line-hight

	textCursor = ui->briefTextedit->textCursor();	//before the setText().

    //bug#101112 关于界面自适应大小
    QTextDocument* doc = new QTextDocument(ui->briefTextedit);
    doc->setPlainText(tr("Peony is a graphical software to help users manage system files. "
                                "It provides common file operation functions for users, such as file viewing, "
                                "file copy, paste, cut, delete, rename, file selection, application opening, "
                                "file search, file sorting, file preview, etc. it is convenient for users to "
                                "manage system files intuitively on the interface."));
    ui->briefTextedit->setDocument(doc);
    textCursor.setBlockFormat(blockFormat);
    ui->briefTextedit->setTextCursor(textCursor);
    doc->setTextWidth(420-32-32);

    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        m_gSettings = new QGSettings("org.lingmo.style", QByteArray(), this);
        connect(m_gSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                setSupportText();
            }
            else if("systemFontSize" == key)
            {
                auto appfont = qApp->font();
                QFont namefont = appfont;
                namefont.setBold(true);
                namefont.setPointSize(appfont.pointSize()*1.28);
                ui->nameLabel->setFont(namefont);

                resetSize();
            }
            else if("iconThemeName" == key)
            {
                ui->iconLabel->setPixmap(QIcon::fromTheme("system-file-manager").pixmap(96,96));
                ui->logoLabel->setPixmap(QIcon::fromTheme("system-file-manager").pixmap(24,24));
            }
        });
    }
    this->setSupportText();
    ui->openlinkLabel->setOpenExternalLinks(true);
}

void AboutDialog::setSupportText()
{
    QString text = tr("Service & Support: ") + "<a href=\"mailto://support@kylinos.cn\" style=\"color:"
                   + convertRGB16HexStr(palette().color(QPalette::ButtonText))
                   + ";\">support@kylinos.cn</a><br/>";
    ui->openlinkLabel->setText(text);
}

QString AboutDialog::convertRGB16HexStr(const QColor &color)
{
    if (color == Qt::transparent)
        return "#000000";//透明则显示黑色
    QString red = QString("%1").arg(color.red(),2,16,QChar('0'));
    QString green = QString("%1").arg(color.green(),2,16,QChar('0'));
    QString blue = QString("%1").arg(color.blue(),2,16,QChar('0'));
    QString hexStr = "#" + red + green + blue;
    return hexStr;
}

QString AboutDialog::getCurrentVersion()
{
    //use self define main version
    //return VERSION;

    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version = VERSION;

    //fix bug#125289, use dpkg query version instead of self define version
    pp = popen("dpkg -l explor", "r");
    if(NULL == pp)
        return version;

    while((read = getline(&line, &len, pp)) != -1){
        q = strrchr(line, '\n');
        *q = '\0';

        QString content = line;
        QStringList list = content.split(" ");

        list.removeAll("");

        if (list.size() >= 3)
            version = list.at(2);
    }

    free(line);
    pclose(pp);
    return version;
}

void AboutDialog::on_closeBtn_clicked()
{
    close();
}
void AboutDialog::resetSize()
{
    int minHeight =  324;
    int maxHeight =  560;

    int newHeight = ui->briefTextedit->document()->size().height();
    int changeHeight = newHeight - ui->briefTextedit->height();
    int finalHeight = this->height() + changeHeight + 10;
    if( finalHeight > maxHeight)
    {
        finalHeight = maxHeight;
    }
    else if(finalHeight < minHeight)
    {
        finalHeight = minHeight;
    }

    this->setFixedHeight(finalHeight);
    ui->verticalLayout_3->update();

}

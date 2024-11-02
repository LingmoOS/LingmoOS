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

#include "errormessagewidget.h"
#include <QTimer>
#include <kwindowsystem.h>
using namespace kdk;

ErrorMessageWidget::ErrorMessageWidget(QWidget *parent) : QWidget(parent)
{
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_CLOSE;
    hints.decorations = MWM_DECOR_ALL;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    KyDebug();
    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    this->setWindowTitle(tr("Bluetooth Message"));
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    closeInfoDialog =  new BeforeTurnOffHintWidget(this);
    connect(closeInfoDialog, &BeforeTurnOffHintWidget::buttonPressed, this,[=](bool confirm) {
        emit closeInfoDialogConfirmed(confirm);
        closeInfoDialog->hide();
    });

#if 0
    if(QGSettings::isSchemaInstalled("org.lingmo.style")){
        gsettings = new QGSettings("org.lingmo.style");
        connect(gsettings,&QGSettings::changed,this,&ErrorMessageWidget::GSettingsChanges);
    }

    this->setFixedSize(520,160);

    warn_icon = new QLabel(this);
    warn_icon->setFixedSize(22, 22);
    warn_icon->setGeometry(23, 20, 22, 22);
    warn_icon->setPixmap(QIcon::fromTheme("lingmo-dialog-warning").pixmap(22,22));
    if (warn_icon->pixmap()->isNull())
        warn_icon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(22,22));
    warn_icon->setVisible(false);

    warn_title = new QLabel(this);
    warn_title->setVisible(false);

    warn_txt = new QLabel(this);
    warn_txt->setVisible(false);

    ok_btn = new QPushButton(this);
    ok_btn->setVisible(false);
#endif
    QDesktopWidget *deskdop = QApplication::desktop();
//    move((deskdop->width()-this->width())/2, (deskdop->height()-this->height())/2);
}

ErrorMessageWidget::~ErrorMessageWidget()
{
    KyDebug();
}

#if 0
void ErrorMessageWidget::GSettingsChanges(QString key) {
    if (key == "systemFontSize" || key == "styleName")
        this->update();
}
#endif

void ErrorMessageWidget::ErrCodeAnalysis(QString dev, int errCode) {
    switch (errCode) {
    case ConnectionErrMsg::ERR_BREDR_CONN_TIMEOUT:
    case ConnectionErrMsg::ERR_BREDR_CONN_PAGE_TIMEOUT:
        TimeOutLayoutInit(dev);
        break;
    default:
        break;
    }
}

void ErrorMessageWidget::TimeOutLayoutInit(QString dev) {
    QString txt = QString(tr("Please confirm if the device '"))
                  + dev
                  + QString(tr("' is in the signal range or the Bluetooth is enabled."));
    showMessageBox(QString(tr("Bluetooth Message")), QString(tr("Connection Time Out!")), txt);
}

void ErrorMessageWidget::showMessageBox(QString title, QString text, QString information) {
    KyDebug() << "";
    if (nullptr != mbox) {
        mbox->accept();
        mbox->deleteLater();
    }

    mbox = new QMessageBox(QMessageBox::Warning,title, text, QMessageBox::Ok,this);
    mbox->setInformativeText(information);
    Config::setKdkGeometry(mbox->windowHandle(), mbox->width(), mbox->height(), true);
    mbox->show();
    KyDebug() << "setSkipTaskBar true";
    WindowManager::setSkipTaskBar(mbox->windowHandle(), true);

    int ret = mbox->exec();
    KyDebug() << ret;

    mbox->deleteLater();
    mbox = nullptr;
}

void ErrorMessageWidget::showCloseInfoDialog() {
    closeInfoDialog->show();
}

void ErrorMessageWidget::closeMessageWidget()
{
    KyInfo() << "";
    if (nullptr != mbox)
        mbox->accept();
}

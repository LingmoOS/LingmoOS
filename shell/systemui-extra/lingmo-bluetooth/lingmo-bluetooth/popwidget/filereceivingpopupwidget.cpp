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

#include "filereceivingpopupwidget.h"
#include "config/xatom-helper.h"
#include "config/config.h"
#include <file-utils.h>
using namespace Peony;
FileReceivingPopupWidget::FileReceivingPopupWidget(QString address, QString devname,
                                                   QString filename, QString type, quint64 size):
    target_address(address),
    target_name(devname),
    target_source(filename),
    target_type(type),
    target_size(size),
    org_name(devname),
    org_source(filename)
{
    if (envPC == Environment::LAIKA || envPC == Environment::MAVIS)
        isIntel = true;
    else
        isIntel = false;
    // 添加窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_CLOSE;
    hints.decorations = MWM_DECOR_ALL;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    if(QGSettings::isSchemaInstalled("org.lingmo.style")){
        StyleSettings = new QGSettings("org.lingmo.style");
        connect(StyleSettings,&QGSettings::changed,this,&FileReceivingPopupWidget::GSettingsChanges);
    }
    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    this->setWindowTitle(tr("Bluetooth File"));
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    if(QGSettings::isSchemaInstalled(GSETTING_SCHEMA_LINGMOBLUETOOH)){
        settings = new QGSettings(GSETTING_SCHEMA_LINGMOBLUETOOH);
        file_path = settings->get("file-save-path").toString();
        connect(settings, &QGSettings::changed,this,&FileReceivingPopupWidget::GSettings_value_chanage);
    }else{
        file_path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }

    file_source = new QLabel(this);
    file_source->setAlignment(Qt::AlignTop);

    QIcon fileicon = getFileIconFromType(target_type);
    file_icon = new QLabel(this);
    file_icon->setPixmap(fileicon.pixmap(64,64));
    file_icon->setAlignment(Qt::AlignCenter);

    QFontMetrics fontMetrics(file_source->font());
    QString fileName = fontMetrics.elidedText(target_source, Qt::ElideMiddle, 280);
    file_name = new QLabel(fileName,this);
    file_name->setToolTip(target_source);
    file_name->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

    QString fileSizeStr;
    int unitcount = 0;   //将单位B转化为对应的KB、MB等单位
    double calc_size = target_size;
    while(calc_size >= 1024) {
        calc_size /= 1024;
        unitcount++;
    }
    fileSizeStr = QString::asprintf("%.1f ", calc_size) + unitString[unitcount];

    file_size = new QLabel(this);
    file_size->setText(fileSizeStr);
    file_size->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    file_size->setWordWrap(true);

    transfer_progress = new QProgressBar(this);
    transfer_progress->setFixedHeight(15);
    transfer_progress->setTextVisible(false);
    transfer_progress->setVisible(false);
    transfer_progress->setMinimum(0);
    transfer_progress->setMaximum(100);

    cancel_btn = new QPushButton(tr("Cancel"),this);
    cancel_btn->setFocusPolicy(Qt::NoFocus);
    connect(cancel_btn,&QPushButton::clicked,this,[=]{
        Q_EMIT this->rejected();

        this->close();
    });

    accept_btn = new QPushButton(tr("Accept"),this);
    accept_btn->setFocusPolicy(Qt::NoFocus);
    connect(accept_btn,&QPushButton::clicked,this,[=]{
        OnClickedAcceptBtn();
    });

    warn_icon = new QLabel(this);
    warn_icon->setPixmap(QIcon::fromTheme("lingmo-dialog-warning").pixmap(22,22));
    if (warn_icon->pixmap()->isNull())
        warn_icon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(22,22));
    warn_icon->setVisible(false);

    view_btn = new QPushButton(tr("View"),this);
    view_btn->setFocusPolicy(Qt::NoFocus);
    view_btn->setVisible(false);

    receivedFiles.clear();
    receiveTimer = new QTimer(this);
    connect(receiveTimer,SIGNAL(timeout()), this, SLOT(transferCompelete()));

    if (isIntel)
        initIntelLayout();
    else
        initLayout();
    fileNums = 0;
    QDesktopWidget *desktop_widget = QApplication::desktop();
    desktop = desktop_widget->availableGeometry();
    move((desktop.width()-this->width())/2, (desktop.height()-this->height())/2);

    window_pop_up_animation();

    const KWindowInfo info(this->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager))
        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);

    this->activateWindow();
    this->show();
    this->setFocusPolicy(Qt::NoFocus);
}

FileReceivingPopupWidget::~FileReceivingPopupWidget()
{
    if (fileStatus == Active) {
        Q_EMIT this->cancel();
        isCanceled = true;
    } else {
        Q_EMIT this->rejected();
    }
    delete settings;
}

void FileReceivingPopupWidget::initLayout() {
    this->setFixedSize(520,236);

    struct_pos pos;
    pos = Config::getWidgetPos(390,175,120,36, this->width());
    view_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,12,460,54, this->width());
    file_source->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,64,64,64, this->width());
    file_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(112,71,293,28, this->width());
    file_name->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(112,96,293,20, this->width());
    file_size->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,144,454,10, this->width());
    transfer_progress->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(278,175,96,36, this->width());
    cancel_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(390,175,96,36, this->width());
    accept_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,14,22,22, this->width());
    warn_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

    double size = 12;
    if(QGSettings::isSchemaInstalled("org.lingmo.style"))
        size = StyleSettings->get("system-font-size").toDouble() - 11;
    QFont ft;
    ft.setPixelSize(18 + size);
    file_source->setFont(ft);
    file_source->setToolTip(target_name);
    QString fileSource;
    QFontMetrics fontMetrics(file_source->font());
    if (ft.pixelSize() > 17)
        target_name = fontMetrics.elidedText(target_name, Qt::ElideMiddle, 140);
    else
        target_name = fontMetrics.elidedText(target_name, Qt::ElideMiddle, 160);
    QString fileName = fontMetrics.elidedText(target_source, Qt::ElideMiddle, 280);
    fileSource = fontMetrics.elidedText(QString(tr("File from \"")+target_name+tr("\", waiting for receive.")), Qt::ElideMiddle, file_source->width());
    file_source->setText(fileSource);
    file_name->setText(fileName);
    ft.setPixelSize(14 + size);
    file_name->setFont(ft);
    file_size->setFont(ft);
}

void FileReceivingPopupWidget::initIntelLayout() {
    this->setWindowFlags(Qt::Dialog);
    this->setFixedSize(478,280);
    QFont ft;
    ft.setPixelSize(16);
    file_size->setFont(ft);
    file_name->setFont(ft);
    QFontMetrics fontMetrics(file_source->font());
    QString filesource = fontMetrics.elidedText(QString(tr("File from \"")+target_name+tr("\", waiting for receive...")), Qt::ElideMiddle, file_source->width());
    file_source->setText(filesource);

    struct_pos pos;
    pos = Config::getWidgetPos(342,200,112,56, this->width());
    accept_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(214,200,112,56, this->width());
    cancel_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(104,75,293,23, this->width());
    file_name->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(104,107,293,23, this->width());
    file_size->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(342,200,120,36, this->width());
    view_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(24,24,430,54, this->width());
    file_source->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(24,71,64,64, this->width());
    file_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(24,159,430,8, this->width());
    transfer_progress->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,34,22,22, this->width());
    warn_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);
}

void FileReceivingPopupWidget::updateUI(QString name, quint64 size, QString type) {
    if (fileStatus == Status::Error)
        return;
    fileNums += 1;
    KyDebug() << name << size << type;
    QFontMetrics fontMetrics(file_source->font());
    QString fileName = fontMetrics.elidedText(name, Qt::ElideMiddle, 280);
    QString fileSource = QString(tr("File from \"") +
                                 target_name +
                                 tr("\", is receiving... (has received ") +
                                 QString::number(fileNums) +
                                 tr(" files)"));

    QString fileSizeStr;
    int unitcount = 0;   //将单位B转化为对应的KB、MB等单位
    double calc_size = size;

    while(calc_size >= 1024) {
        calc_size /= 1024;
        unitcount++;
    }
    fileSizeStr = QString::asprintf("%.1f", calc_size) + unitString[unitcount];

    target_size = size;
    target_type = type;
    target_source = name;

    QIcon fileicon = getFileIconFromType(target_type);
    file_size->setText(fileSizeStr);
    file_icon->setPixmap(fileicon.pixmap(64,64));
    file_name->setText(fileName);
    file_name->setToolTip(name);
    file_source->setWordWrap(true);
    file_source->setText(fileSource);

    file_source->setToolTip(target_name);
    if (file_source->font().pixelSize() > 17) {
        struct_pos pos;
        pos = Config::getWidgetPos(32,0,460,66, this->width());
        file_source->setGeometry(pos.x, pos.y, pos.width, pos.high);
    }

    this->update();
}

void FileReceivingPopupWidget::window_pop_up_animation()
{
    KyDebug() << desktop << desktop.right() << desktop.bottom() <<this->geometry();

//    QPropertyAnimation *window_action = new QPropertyAnimation(this,"geometry");
//    window_action->setDuration(100);
//    QRect this_rect = this->rect();
//    this_rect.setHeight(0);
//    this_rect.setWidth(0);
//    window_action->setStartValue(QRect(desktop.right()-1,desktop.bottom()-this->height(),desktop.right(),desktop.bottom()));
//    window_action->setEndValue(QRect(desktop.right()-this->width(),desktop.bottom()-this->height(),desktop.right(),desktop.bottom()));
//    window_action->setStartValue(QPoint(desktop.right()-1,desktop.bottom()-this->height()));
//    window_action->setEndValue(QPoint(desktop.right()-this->width(),desktop.bottom()-this->height()));
//    KyDebug() << this_rect << this->geometry() << QRect(desktop.right()-1,desktop.bottom()-this->height(),desktop.right(),desktop.bottom()) <<QRect(desktop.right()-this->width(),desktop.bottom()-this->height(),desktop.right(),desktop.bottom());
//    window_action->start();
}

void FileReceivingPopupWidget::OnClickedAcceptBtn()
{
    transfer_progress->setVisible(true);
    accept_btn->setVisible(false);
    this->accepted();
}

void FileReceivingPopupWidget::update_transfer_progress_bar(quint64 value)
{
    if (target_size == 0 && fileStatus != Status::Active)
        return;
    transfer_progress->setValue(value);
    transfer_progress->repaint();
}

void FileReceivingPopupWidget::showFiles() {
    if(receivedFiles.isEmpty())
        return;
    QProcess *process = new QProcess(this);
    QString cmd = "explor";
    QStringList arg;
    KyDebug();
    arg <<  "--show-items";
    for (int i = 0; i < receivedFiles.size(); i++)
    {
        QString mDest = "file://" + receivedFiles.at(i);
        if (FileUtils::isFileExsit(mDest))
            arg << receivedFiles.at(i);
    }
    process->startDetached(cmd,arg);
}

void FileReceivingPopupWidget::transferCompelete() {
    cancel_btn->setVisible(false);

    Config::soundComplete();
    showFiles();
    this->close();
}

void FileReceivingPopupWidget::statusChangedSlot(QMap<QString, QVariant> status) {
    if (!status.value("transportType").toInt())
        return;
    if (status.contains("dev")) {
        if (status.value("dev").toString() != target_address)
            return;
    }
    if(status.contains("status"))
        if (status.value("status").toInt() != fileStatus)
        {
            if (status.value("status").toInt() == 3
                    && !receivedFiles.contains(status.value("savepath").toString()))
                receivedFiles.append(status.value("savepath").toString());
            KyDebug()<< receivedFiles << __LINE__;
            file_transfer_completed(status.value("status").toULongLong());
        }
    if (status.contains("progress"))
        update_transfer_progress_bar(status.value("progress").toInt());
    if(status.contains("fileseq"))
        if (status.value("fileseq").toInt() > fileNums)
        updateUI(status.value("filename").toString(),
                 status.value("filesize").toULongLong(),
                 status.value("filetype").toString());
}

void FileReceivingPopupWidget::file_transfer_completed(int status)
{
    if (isCanceled) {
        this->close();
        this->deleteLater();
        return;
    }

    struct_pos pos;

    KyDebug() << "status : " << status << "   inside status:" << fileStatus << "    value:" << transfer_progress->value();
    if(status == Status::Active){
        if (receiveTimer->isActive()) {
            receiveTimer->stop();
        } else {
            QFontMetrics fontMetrics(file_source->font());
            QString fileSource;
            fileSource = fontMetrics.elidedText(QString(tr("File from \"")+target_name+tr("\", is receiving...")), Qt::ElideMiddle, file_source->width());
            file_source->setText(fileSource);
        }
        this->update();

        cancel_btn->disconnect();
        cancel_btn->connect(cancel_btn,&QPushButton::clicked,this,[=]{
            Q_EMIT this->cancel();
            isCanceled = true;
        });



        if (!isIntel) {
            pos = Config::getWidgetPos(390,175,96,36, this->width());
        }
        else
            pos = Config::getWidgetPos(342,200,112,56, this->width());

        cancel_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    }else if(status == Status::Complete){
        receiveTimer->start(2000);
        KyDebug() << __LINE__ ;
    }else if(status == Status::Error || (status == Status::Static && transfer_progress->value() != 100)){
        if (fileStatus == Status::Complete || fileStatus == Status::Error) {
            fileStatus = status;
            return;
        }
        if (!isIntel) {
            Config::SendNotifyMessage(QString(tr("File Receive Failed!")),
                                      QString(tr("Receive file '") + org_source + tr("' from '") + org_name + tr("'")),
                                      QString("dialog-error"));
            showFiles();
            this->close();
            fileStatus = status;
            return;
        }
        accept_btn->setVisible(true);
        accept_btn->setText(tr("OK"));
        accept_btn->disconnect();
        accept_btn->connect(accept_btn,&QPushButton::clicked,this,[=]{
            this->close();
        });
        cancel_btn->setVisible(false);
        view_btn->setVisible(false);
        transfer_progress->setVisible(false);
        file_icon->setVisible(false);
        file_name->setVisible(false);
        file_size->setVisible(false);

        file_source->setAlignment(Qt::AlignTop);

        pos = Config::getWidgetPos(62,12,430,60, this->width());
        file_source->setGeometry(pos.x, pos.y, pos.width, pos.high);

        QFontMetrics fontMetrics(file_source->font());
        QString fileSource = fontMetrics.elidedText(QString(tr("File from \"")+target_name+tr("\", received failed !")), Qt::ElideMiddle, file_source->width());
        file_source->setText(fileSource);
        warn_icon->setVisible(true);
        warn_icon->setFixedSize(64,64);
        warn_icon->setPixmap(QPixmap("/usr/share/lingmo-bluetooth/file-transfer-failed.svg"));
        file_source->setText(tr("File Transmission Failed !"));
        file_source->setStyleSheet("QLabel{\
                                             font-family: NotoSansCJKsc;\
                                             font-size: 16px;\
                                             line-height: 19px;\
                                             text-align: center;\
                                             color: #FB5050;}");
        pos = Config::getWidgetPos(207,17,64,64, this->width());
        warn_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(191,89,200,24, this->width());
        file_source->setGeometry(pos.x, pos.y, pos.width, pos.high);

        this->activateWindow();
        this->show();
    }
    fileStatus = status;
}

void FileReceivingPopupWidget::GSettings_value_chanage(const QString &key)
{
    if(key == "file-save-path"){
        file_path = settings->get("file-save-path").toString();
    }
}

QIcon FileReceivingPopupWidget::getFileIconFromType(QString type) {
    QIcon fileicon;
    if (type.split("/").at(0) == "image"){
        fileicon = QIcon::fromTheme("image");
        if (fileicon.isNull()) {
            fileicon = QIcon::fromTheme("text");
        }
    }else if (type.split("/").at(0) == "video")
        fileicon = QIcon::fromTheme("video-x-generic");
    else if (type.split("/").at(0) == "audio" ||
             type.split("/").at(0) == "application" ||
             type.split("/").at(0) == "text" ||
             type.split("/").at(0) == "inode"){
        if (QIcon::hasThemeIcon(type.split("/").join("-"))) {
            fileicon = QIcon::fromTheme(type.split("/").join("-"));
        } else if (target_source.split(".").size() >= 2){
            if (fileicon.isNull())
                fileicon = QIcon::fromTheme(target_source.split(".").at(1));
            if (fileicon.isNull())
                fileicon = QIcon::fromTheme(QString("application-")+target_source.split(".").at(1));
            if (fileicon.isNull())
                fileicon = QIcon::fromTheme(QString("application-wps-office.")+target_source.split(".").at(1));
        }
    }

    if (fileicon.isNull() && !target_type.isEmpty()){
        QString icon = g_content_type_get_generic_icon_name(target_type.toUtf8().constData());
        fileicon = QIcon::fromTheme(icon);
    }
    if (fileicon.isNull()) {
        fileicon = QIcon::fromTheme("text");
    }
    return fileicon;
}

void FileReceivingPopupWidget::GSettingsChanges(const QString &key)
{
    KyDebug() << key;
    if(key == "styleName"){
        this->update();
    }
    if (key == "systemFontSize") {
        double size = StyleSettings->get("system-font-size").toDouble() - 11;
        QFont ft;
        struct_pos pos;
        ft.setPixelSize(18 + size);
        if (ft.pixelSize() > 17)
            pos = Config::getWidgetPos(32,0,460,66, this->width());
        else
            pos = Config::getWidgetPos(32,12,460,54, this->width());

        file_source->setGeometry(pos.x, pos.y, pos.width, pos.high);

        file_source->setFont(ft);

        QFontMetrics fontMetrics(file_source->font());
        if (ft.pixelSize() > 17)
            target_name = fontMetrics.elidedText(target_name, Qt::ElideMiddle, 140);
        else
            target_name = fontMetrics.elidedText(target_name, Qt::ElideMiddle, 160);
        QString fileName = fontMetrics.elidedText(target_source, Qt::ElideMiddle, 280);
        file_name->setText(fileName);
        ft.setPixelSize(14 + size);
        file_name->setFont(ft);
        file_size->setFont(ft);
    }
}

QPushButton* FileReceivingPopupWidget::getButton() {
    switch (pressCnt) {
    case 1:
        if (!accept_btn->isVisible() || !cancel_btn->isVisible())
            ++pressCnt;
        if(accept_btn->isVisible())
            return accept_btn;
        else if (cancel_btn->isVisible()){
            return cancel_btn;
        }
        return nullptr;
    case 2:
        return cancel_btn;
    case 3:
//        return close_btn;
    default:
        return nullptr;
    }
}

void FileReceivingPopupWidget::paintEvent(QPaintEvent *event) {
    QPainter pt(this);
    QColor color = QColor("#818181");
    pt.setPen(color);
    QPushButton *btn = getButton();
    if (btn == nullptr)
        return;
    QRect rect = btn->geometry();
    pt.setRenderHint(QPainter::Antialiasing);
    pt.drawRect(rect);
    QWidget::paintEvent(event);
}

void FileReceivingPopupWidget::keyPressEvent(QKeyEvent *event) {
    QPushButton *bbtn = getButton();
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
        if (bbtn == nullptr)
            return;
        bbtn->clicked();
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

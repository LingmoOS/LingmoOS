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

#include "bluetoothfiletransferwidget.h"
#include "config/xatom-helper.h"

bool BluetoothFileTransferWidget::isShow = false;
BluetoothFileTransferWidget::BluetoothFileTransferWidget(QStringList files, QMap<QString, QMap<QString, QVariant>> devicelist, QString dev_address):
//    QWidget(parent),
    selectedFiles(files)
{
    isShow = true;
    send_state = _SEND_NONE;
    fileNums = selectedFiles.size();

    if (envPC == Environment::LAIKA || envPC == Environment::MAVIS)
        is_Intel = true;
    else
        is_Intel = false;

    if(QGSettings::isSchemaInstalled("org.lingmo.style")){
        GSettings = new QGSettings("org.lingmo.style");
        connect(GSettings,&QGSettings::changed,this,&BluetoothFileTransferWidget::GSettingsChanges);
    }

    //窗管协议
//    setAttribute(Qt::WA_TranslucentBackground);
//    setProperty("useSystemStyleBlur", true);
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_CLOSE;
    hints.decorations = MWM_DECOR_ALL;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    KyDebug() ;
    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    this->setWindowTitle(tr("Bluetooth File"));
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    const QByteArray transparency_id("org.lingmo.control-center.personalise");
    if(QGSettings::isSchemaInstalled(transparency_id)){
        transparency_gsettings = new QGSettings(transparency_id);
    }
    if(QGSettings::isSchemaInstalled(transparency_id)){
       tran=transparency_gsettings->get("transparency").toDouble()*255;
       connect(transparency_gsettings, &QGSettings::changed, this, [=] {
           tran=transparency_gsettings->get("transparency").toDouble()*255;
           this->update();
       });
    }

    QDesktopWidget *deskdop = QApplication::desktop();
    move((deskdop->width()-this->width())/2, (deskdop->height()-this->height())/2);

    tip_text = new QLabel(this);

    tip_text->setWordWrap(true);
    tip_text->setVisible(false);

    if (!initFileInfo())
        return;

    target_icon = new QLabel(this);
    target_icon->setPixmap(file_icon.at(0).pixmap(64,64));

    target_size = new QLabel(file_size.at(0),this);
    target_name = new QLabel(this);

    QFontMetrics fontMetrics(target_size->font());
    QString fileName;
    QString Name = getFileName(selectedFiles.at(0));
    if (fileNums > 1) {
        fileName = fontMetrics.elidedText("\""
                                          + Name
                                          +"\"", Qt::ElideMiddle, 280);
        if(fileName != Name)
            target_name->setToolTip(Name);
        fileName.append(tr(" and ")+QString::number(fileNums)+tr(" files more"));
    }else {
        fileName = fontMetrics.elidedText(Name, Qt::ElideMiddle, 280);
        if(fileName != Name)
            target_name->setToolTip(Name);
    }
    target_name->setText(fileName);

    transNum = new QLabel("1/"+QString::number(fileNums), this);
    transNum->setVisible(false);

    m_progressbar = new QProgressBar(this);
    m_progressbar->setOrientation(Qt::Horizontal);
    m_progressbar->setTextVisible(false);
    m_progressbar->setVisible(false);
    m_progressbar->setFocusPolicy(Qt::NoFocus);
    m_progressbar->setMinimum(0);
    m_progressbar->setMaximum(100);
    m_progressbar->setValue(0);

    Tiptop = new QLabel(tr("Select Device"),this);

    ok_btn = new QPushButton(tr("OK"),this);
    ok_btn->setFocusPolicy(Qt::NoFocus);
    ok_btn->setFixedSize(100,36);
    if(selectedDev == nullptr) {
        ok_btn->setEnabled(false);
    }
    connect(ok_btn,&QPushButton::clicked,this,&BluetoothFileTransferWidget::onClicked_OK_Btn);

    cancel_btn = new QPushButton(tr("Cancel"),this);
    cancel_btn->setFocusPolicy(Qt::NoFocus);
    cancel_btn->setFixedSize(100,36);
    connect(cancel_btn,&QPushButton::clicked,this,[=]{
        this->close();
        if (target_dev.isEmpty())
            return;
        if (this->send_state == _SEND_NONE)
            return;
        if (this->send_state != _SEND_COMPLETE || this->send_state != _SEND_FAILURE)
            emit stopSendFile(target_dev);
    });

    qtw = new QTreeWidget(this);
    qtw->setFocusProxy(this);
    qtw->setFixedSize(456,356);
    qtw->setHeaderHidden(true);
    qtw->addTopLevelItems(toolbutton_list);
    qtw->setAlternatingRowColors(true);
    qtw->setProperty("highlightMode", true);
    this->setTabOrder(this,qtw);
    connect(qtw, &QTreeWidget::itemSelectionChanged, this, [=](){
        if(qtw->selectedItems().size() == 0)
            return;
        int cnt = toolbutton_list.indexOf(qtw->selectedItems().at(0));
        if(selectedDev != nullptr) {
            if (QGSettings::isSchemaInstalled("org.lingmo.style") && GSettings->get("style-name").toString() != "lingmo-dark")
                selectedDev->setIcon(0, QIcon(Config::loadSvg(selectedDev->icon(0).pixmap(64,64), Config::PixmapColor::BLACK)));
            else
                selectedDev->setIcon(0, QIcon(Config::loadSvg(selectedDev->icon(0).pixmap(64,64), Config::PixmapColor::WHITE)));
        }
        selectedDev = toolbutton_list.value(cnt);
        if (!ok_btn->isEnabled())
            ok_btn->setEnabled(true);
    });
    connect(qtw, &QTreeWidget::itemPressed, this, [=](QTreeWidgetItem *item, int column){
        Q_UNUSED(column)
        if(selectedDev != nullptr) {
            if (GSettings->get("style-name").toString() != "lingmo-dark")
                selectedDev->setIcon(0, QIcon(Config::loadSvg(selectedDev->icon(0).pixmap(64,64), Config::PixmapColor::BLACK)));
            else
                selectedDev->setIcon(0, QIcon(Config::loadSvg(selectedDev->icon(0).pixmap(64,64), Config::PixmapColor::WHITE)));
        }
        selectedDev->setIcon(0, QIcon(Config::loadSvg(selectedDev->icon(0).pixmap(64,64), Config::PixmapColor::WHITE)));
        selectedDev = item;
        if (!ok_btn->isEnabled())
            ok_btn->setEnabled(true);
    });
    initQTreeWidget(devicelist,dev_address);

    tranfer_status_icon = new QLabel(this);
    tranfer_status_icon->setPixmap(QIcon::fromTheme("lingmo-dialog-warning").pixmap(22,22));
    if (tranfer_status_icon->pixmap()->isNull())
        tranfer_status_icon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(22,22));
    tranfer_status_icon->setFixedSize(22,22);
    tranfer_status_icon->setVisible(false);

    tranfer_status_text = new QLabel(this);
    tranfer_status_text->setText(tr("File Transfer Fail!"));
    tranfer_status_text->setVisible(false);

    if (is_Intel)
        initIntelLayout();
    else
        initLayout();

    this->setFocus();
}

BluetoothFileTransferWidget::~BluetoothFileTransferWidget()
{
    if (this->send_state == _SENDING || this->send_state == _SEND_NONE)
        emit stopSendFile(target_dev);
    isShow = false;
    oversizedFile.clear();
    hasemptyfile = false;
    closed = false;
}

void BluetoothFileTransferWidget::initQTreeWidget(QMap<QString, QMap<QString, QVariant>> devicelist, QString dev_address)
{
    for(auto addr : devicelist.keys()){
        BluezQt::Device::Type type = BluezQt::Device::stringToType(devicelist.value(addr).value("TypeName").toString());
        if((type == BluezQt::Device::Phone)||(type == BluezQt::Device::Computer)){
                QIcon icon;
                switch (type){
                case BluezQt::Device::Type::Phone:
                    icon = QIcon::fromTheme("phone-symbolic");
                    break;
                case BluezQt::Device::Type::Computer:
                    icon = QIcon::fromTheme("video-display-symbolic");
                    break;
                case BluezQt::Device::Type::Uncategorized:
                default:
                    icon = QIcon::fromTheme("bluetooth-symbolic");
                    break;
                }

                QTreeWidgetItem *item = new QTreeWidgetItem(qtw);
                item->setExpanded(true);
                item->setSizeHint(0,QSize(456, 50));
                item->setText(0, devicelist.value(addr).value("Name").toString());
                item->setToolTip(0, devicelist.value(addr).value("Name").toString());
                if (devicelist.value(addr).contains("ShowName"))
                    if (devicelist.value(addr).value("ShowName").toString() != QString(""))
                    {
                        item->setText(0, devicelist.value(addr).value("ShowName").toString());
                        item->setToolTip(0, devicelist.value(addr).value("ShowName").toString());
                    }
                if (GSettings->get("style-name").toString() != "lingmo-dark")
                    item->setIcon(0,QIcon(icon.pixmap(64,64)));
                else
                    item->setIcon(0, QIcon(Config::loadSvg(icon.pixmap(64,64), Config::PixmapColor::WHITE)));
                itemMap.insert(item, addr);
                toolbutton_list.append(item);
                if (dev_address == addr) {
                    item->setSelected(true);
                    item->setIcon(0, QIcon(Config::loadSvg(icon.pixmap(64,64), Config::PixmapColor::WHITE)));
                    selectedDev = item;
                    if (!ok_btn->isEnabled())
                        ok_btn->setEnabled(true);
                }
        }
    }
}


void BluetoothFileTransferWidget::initLayout() {
    this->setFixedSize(520,580);

    //布局
    qtw->resize(456,356);
    struct_pos pos;
    pos = Config::getWidgetPos(62,13,376,27, this->width());
    tranfer_status_text->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,15,22,22, this->width());
    tranfer_status_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(280,520,100,36, this->width());
    cancel_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(390,520,100,36, this->width());
    ok_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,135,456,356, this->width());
    qtw->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,104,Tiptop->width(), 0, this->width());
    Tiptop->move(pos.x, pos.y);

    pos = Config::getWidgetPos(112,48,88,30, this->width());
    target_size->setGeometry(pos.x, pos.y, pos.width, pos.high);

    tip_text->setFixedWidth(452);
    pos = Config::getWidgetPos(32, 12, tip_text->width(), 0, this->width());
    tip_text->move(pos.x, pos.y);


    pos = Config::getWidgetPos(32,16,64,64, this->width());
    target_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

    target_name->setMinimumWidth(100);
    pos = Config::getWidgetPos(112, 23, target_name->width(), 0, this->width());
    target_name->move(pos.x, pos.y);


    pos = Config::getWidgetPos(459,128,52,20, this->width());
    transNum->setGeometry(pos.x, pos.y, pos.width, pos.high);
    if (fileNums > 1) {
        pos = Config::getWidgetPos(32,135,406,10, this->width());
        m_progressbar->setGeometry(pos.x, pos.y, pos.width, pos.high);
    }
    else {
        pos = Config::getWidgetPos(32,145,456,10, this->width());
        m_progressbar->setGeometry(pos.x, pos.y, pos.width, pos.high);
    }


    //字体
    double size = GSettings->get("system-font-size").toDouble() - 11;
    QFont ft;
    ft.setPixelSize(14+size);
    target_size->setFont(ft);
    target_name->setFont(ft);
    transNum->setFont(ft);
    Tiptop->setFont(ft);
    ft.setPixelSize(18+size);
    tip_text->setFont(ft);
    tranfer_status_text->setFont(ft);

    const KWindowInfo info(this->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager))
        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
}

void BluetoothFileTransferWidget::initIntelLayout() {
    this->setWindowFlags(Qt::Dialog);
    this->setFixedSize(478,530);
    cancel_btn->setFixedSize(112,56);
    ok_btn->setFixedSize(112,56);
    target_name->setFixedHeight(30);
    target_size->setFixedWidth(88);
    qtw->resize(430,296);

    struct_pos pos;
    pos = Config::getWidgetPos(342,450,112,56, this->width());
    ok_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(214,450,112,56, this->width());
    cancel_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(62,33,376,27, this->width());
    tranfer_status_text->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,35,22,22, this->width());
    tranfer_status_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(24,139,430,296, this->width());
    qtw->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(32,111,100,20, this->width());
    Tiptop->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(104,60,88,30, this->width());
    target_size->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(24,24,430,56, this->width());
    tip_text->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(24,24,64,64, this->width());
    target_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(104,28,400,30, this->width());
    target_name->setGeometry(pos.x, pos.y, pos.width, pos.high);

    pos = Config::getWidgetPos(410,148,52,20, this->width());
    transNum->setGeometry(pos.x, pos.y, pos.width, pos.high);
    if (fileNums > 1)
        pos = Config::getWidgetPos(24,155,372,8, this->width());
    else
        pos = Config::getWidgetPos(24,155,420,8, this->width());
    m_progressbar->setGeometry(pos.x, pos.y, pos.width, pos.high);

    double size = GSettings->get("system-font-size").toDouble() - 11;
    QFont ft;
    ft.setPixelSize(14+size);
    target_size->setFont(ft);
    target_name->setFont(ft);
    transNum->setFont(ft);
    Tiptop->setFont(ft);
    ft.setPixelSize(18+size);
    tip_text->setFont(ft);
    tranfer_status_text->setFont(ft);
    this->show();
    this->activateWindow();
}

bool BluetoothFileTransferWidget::initFileInfo() {
    for (int i = 0; i < fileNums; i++) {
        Get_fie_type(i);
    }

    if (hasemptyfile) {
        QMessageBox *mbox = new QMessageBox(QMessageBox::Warning,tr("Warning"),tr("The selected file is empty, please select the file again !"),QMessageBox::Ok,this);
        mbox->show();
        Config::setKdkGeometry(mbox->windowHandle(),
                               mbox->width(),
                               mbox->height(),
                               true);
        mbox->exec();
        this->deleteLater();
        return false;
    }

    if (!oversizedFile.isEmpty()) {
        QString txt;
        for (auto item:oversizedFile.keys()) {
            QString size = oversizedFile.value(item);
            for (int i = 0; i < size.size(); i++) {
                if (i != 0 && size.at(i - 1) >= '0' && size.at(i - 1) <= '9'
                        && size.at(i + 1) >= 'A' && size.at(i + 1) <= 'Z')
                {
                    size.remove(i, 1);
                }
            }
            txt.append(getFileName(item) +"  " + size + "\n");
        }
        QMessageBox *mbox = new QMessageBox(QMessageBox::Warning,tr("Warning"),tr("The selected file is larger than 4GB, which is not supported transfer!"),QMessageBox::Ok,this);
        if (fileNums > 1) {
            mbox->setInformativeText(txt);
        }
        mbox->show();
        Config::setKdkGeometry(mbox->windowHandle(),
                               mbox->width(),
                               mbox->height(),
                               true);
        mbox->exec();
        this->deleteLater();
        return false;
    }
    for (int i = 0; i < fileNums; i++) {
        sendFilesList.append(selectedFiles.at(i) + QString("  ") + filetype.at(0));
    }
    return true;
}

QString BluetoothFileTransferWidget::getFileName(QString filePath) {
    return filePath.split("/").at(filePath.split("/").length()-1);
}

QString BluetoothFileTransferWidget::changeUnit(QString size) {
    bool isBytes = false;
    int ii = QString(size).indexOf('i');
    QString _tmp_size = size;
    _tmp_size.remove(ii, ii);
    if (ii != -1 && _tmp_size.at(_tmp_size.size() - 1) != 'B')
        _tmp_size.append("B");

    QStringList strSlipList = _tmp_size.split(" ");
    if (strSlipList.at(strSlipList.size() - 1) == "字节") {
        _tmp_size.remove("字节");
        isBytes = true;
    }
    if (strSlipList.at(strSlipList.size() - 1) == "bytes") {
        _tmp_size.remove("bytes");
        isBytes = true;
    }
    if(isBytes)
        _tmp_size.append(tr("Bytes"));

    return _tmp_size;
}

void BluetoothFileTransferWidget::Get_fie_type(int i)
{
    GError *error;
    KyDebug() << selectedFiles.at(i);
    GFile *file = g_file_new_for_path(selectedFiles.at(i).toStdString().c_str());
    GFileInfo *file_info = g_file_query_info(file,"*",G_FILE_QUERY_INFO_NONE,NULL,&error);
    QString s_size = changeUnit(g_format_size_full(g_file_info_get_size(file_info),G_FORMAT_SIZE_IEC_UNITS));
    quint64 ul_size = g_file_info_get_size(file_info);
    QString str = g_file_info_get_content_type(file_info);

    if (!hasemptyfile && ul_size == 0)
        hasemptyfile = true;

    if (ul_size >= 4250000000) {
        oversizedFile.insert(selectedFiles.at(i), s_size);
        return;
    }
    file_size.push_back(s_size);

    if (str.isEmpty() && g_file_info_has_attribute(file_info, "standard::fast-content-type"))
            str = g_file_info_get_attribute_string(file_info, "standard::fast-content-type");
    filetype.push_back(str);

    QIcon fileicon;
    if (str.split("/").at(0) == "image"){
        fileicon = QIcon(selectedFiles.at(i));
    }else if (str.split("/").at(0) == "video")
        fileicon = QIcon::fromTheme("video-x-generic");
    else if (str.split("/").at(0) == "audio" ||
             str.split("/").at(0) == "application" ||
             str.split("/").at(0) == "text" ||
             str.split("/").at(0) == "inode"){
        if (QIcon::hasThemeIcon(str.split("/").join("-"))) {
            fileicon = QIcon::fromTheme(str.split("/").join("-"));
        }
    }
    if (fileicon.isNull() && !str.isEmpty()){
        QString icon = g_content_type_get_generic_icon_name(str.toUtf8().constData());
        fileicon = QIcon::fromTheme(icon);
    }
    if (fileicon.isNull()) {
        fileicon = QIcon::fromTheme("text");
    }
    file_icon.push_back(fileicon);
}

void BluetoothFileTransferWidget::Initialize_and_start_animation()
{
    this->setFixedSize(520,236);

    QString nName;
    QString orgName;
    orgName = selectedDev->text(0);
    double size = GSettings->get("system-font-size").toDouble() - 11;
    QFont ft;
    ft.setPixelSize(18 + size);
    tip_text->setFont(ft);
    QFontMetrics fontMetrics(tip_text->font());
    nName = fontMetrics.elidedText(orgName, Qt::ElideMiddle, 280);
    QString fileSource = QString(tr("Transferring to \"")+nName+"\"");
    tip_text->setText(fileSource);
    if (orgName != nName)
        tip_text->setToolTip(orgName);
    tip_text->update();

    ok_btn->setVisible(false);
    cancel_btn->setVisible(true);
    m_progressbar->setVisible(true);
    transNum->setVisible(fileNums - 1);
    Tiptop->setVisible(false);
    qtw->setVisible(false);
    tip_text->setVisible(true);

    struct_pos pos;

    if (is_Intel) {
        pos = Config::getWidgetPos(342,450,112,56, this->width());
        cancel_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(24,67,64,64, this->width());
        target_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(104,71,400,30, this->width());
        target_name->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(104,103,88,30, this->width());
        target_size->setGeometry(pos.x, pos.y, pos.width, pos.high);
    } else {
        pos = Config::getWidgetPos(390, 173, 100, 36, this->width());
        cancel_btn->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(32,65,64,64, this->width());
        target_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(112,72,400,30, this->width());
        target_name->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(112,97,88,30, this->width());
        target_size->setGeometry(pos.x, pos.y, pos.width, pos.high);
    }
    if (is_Intel)
        this->setFixedSize(478,280);
    else
        this->setFixedSize(520,236);

    main_animation_group = new QParallelAnimationGroup(this);

    QPropertyAnimation *progress_action = new QPropertyAnimation(m_progressbar,"geometry");
    QRect transfer_progress_rect = m_progressbar->geometry();
    transfer_progress_rect.setWidth(0);
    progress_action->setStartValue(transfer_progress_rect);
    progress_action->setEndValue(m_progressbar->geometry());
    progress_action->setDuration(200);

    main_animation_group->addAnimation(progress_action);
    main_animation_group->start();
}

void BluetoothFileTransferWidget::insertNewFileList(QStringList fileList) {
    QMap<QString, QString> newoversizeFile;
    QList<QIcon>   fileIconList;
    QList<QString> fileSizeList;
    QList<QString> filetypeList;
    QList<QString> sendList;
    for (int i = 0; i < fileList.size(); i++) {
        GError *error;
        KyDebug() << fileList.at(i);
        GFile *file = g_file_new_for_path(fileList.at(i).toStdString().c_str());
        GFileInfo *file_info = g_file_query_info(file,"*",G_FILE_QUERY_INFO_NONE,NULL,&error);
        KyDebug()  << g_file_info_get_size(file_info) << g_file_info_get_content_type(file_info);
        KyDebug() ;

        QString s_size = changeUnit(g_format_size_full(g_file_info_get_size(file_info),G_FORMAT_SIZE_IEC_UNITS));
        quint64 ul_size = g_file_info_get_size(file_info);
        QString str = g_file_info_get_content_type(file_info);

        if (!hasemptyfile && ul_size == 0)
            hasemptyfile = true;

        if (ul_size >= 4250000000) {
            newoversizeFile.insert(fileList.at(i), s_size);
            continue;
        }

        fileSizeList.push_back(s_size);
        filetypeList.push_back(str);

        KyDebug() ;



        QIcon fileicon;
        if (str.split("/").at(0) == "image"){
            fileicon = QIcon(fileList.at(i));
            if (fileicon.isNull()) {
                fileicon = QIcon::fromTheme("text");
            }
        }else if (str.split("/").at(0) == "video")
            fileicon = QIcon::fromTheme("video-x-generic");
        else if (str.split("/").at(0) == "audio" ||
                 str.split("/").at(0) == "application" ||
                 str.split("/").at(0) == "text" ||
                 str.split("/").at(0) == "inode"){
            if (QIcon::hasThemeIcon(str.split("/").join("-"))) {
                fileicon = QIcon::fromTheme(str.split("/").join("-"));
            } else {
                fileicon = QIcon::fromTheme("text");
            }
        }else
            fileicon = QIcon::fromTheme("text");
        KyDebug() ;
        fileIconList.push_back(fileicon);
        sendList.append(fileList.back() + QString("  ") + filetypeList.back());
    }

    if (hasemptyfile) {
        QMessageBox *mbox = new QMessageBox(QMessageBox::Warning,tr("Warning"),tr("The selected file is empty, please select the file again !"),QMessageBox::Ok,this);
        mbox->show();
        Config::setKdkGeometry(mbox->windowHandle(),
                               mbox->width(),
                               mbox->height(),
                               true);
        mbox->exec();
        return;
    }

    if (!newoversizeFile.isEmpty()) {
        QString txt;
        for (auto item:newoversizeFile.keys()) {
            txt.append(getFileName(item) +"  " + newoversizeFile.value(item) + "\n");
        }
        QMessageBox *mbox = new QMessageBox(QMessageBox::Warning,tr("Warning"),tr("The selected file is larger than 4GB, which is not supported transfer!"),QMessageBox::Ok,this);
        if (fileList.size() > 1) {
            mbox->setInformativeText(txt);
        }
        mbox->show();
        Config::setKdkGeometry(mbox->windowHandle(),
                               mbox->width(),
                               mbox->height(),
                               true);
        mbox->exec();
        return;
    }
    filetype.append(filetypeList);
    file_icon.append(fileIconList);
    file_size.append(fileSizeList);
    selectedFiles.append(fileList);
    sendFilesList.append(sendList);
    fileNums += fileList.size();
    int transfing = fileNums - selectedFiles.size() + 1;
    emit sender_dev_name(target_dev, sendList);
    QFontMetrics fontMetrics(target_size->font());
    QString _filename = getFileName(selectedFiles.at(0));
    QString _text;
    if (selectedFiles.size() == 1) {
        _text = fontMetrics.elidedText(_filename, Qt::ElideMiddle, 280);
        target_name->setText(_text);
    }
    else if(selectedFiles.size() > 1)
    {
        _filename = "\"" + _filename + "\"";
        _text = fontMetrics.elidedText(_filename, Qt::ElideMiddle, 280);
        target_name->setText(_text.append(tr(" and ")+QString::number(fileNums)+tr(" files more")));
    }
    if (_text != _filename)
        target_name->setToolTip(_filename);
    transNum->setText(QString::number(transfing)+"/"+QString::number(fileNums));
    struct_pos pos;
    if (is_Intel)
        if (fileNums > 1)
            pos = Config::getWidgetPos(24,155,372,8, this->width());

        else
            pos = Config::getWidgetPos(24,155,420,8, this->width());
    else
        if (fileNums > 1)
            pos = Config::getWidgetPos(32,135,406,15, this->width());
        else
            pos = Config::getWidgetPos(32,145,454,15, this->width());

    m_progressbar->setGeometry(pos.x, pos.y, pos.width, pos.high);

    transNum->setVisible((fileNums - 1) && (this->send_state == _SENDING));
    this->update();
    this->show();
    this->activateWindow();
}

void BluetoothFileTransferWidget::statusChangedSlot(QMap<QString, QVariant> status) {
    if (status.value("transportType").toInt() == 1)
        return;
    if (status.value("dev").toString() != target_dev)
        return;
    if(status.contains("status"))
        get_transfer_status(status.value("status").toInt());
    if (status.contains("progress"))
        set_m_progressbar_value(status.value("progress").toInt());
    if (status.contains("fileFailedDisc"))
        tranfer_error();
}

void BluetoothFileTransferWidget::powerChanged(bool powered) {
    if (powered)
        return;
    if (send_state == _SEND_FAILURE)
        return;
    closed =true;
    emit stopSendFile(target_dev);
}

void BluetoothFileTransferWidget::devattrChanged(QString address, QMap<QString, QVariant> devAttr) {
    if (address != target_dev)
        return;
    if (send_state == _SEND_FAILURE)
        return;
    if (!devAttr.contains("Connected"))
        return;
    if (devAttr.value("Connected").toBool())
        return;
    emit stopSendFile(target_dev);
    closed =true;
    KyDebug() << devAttr;
    tranfer_error();
}

void BluetoothFileTransferWidget::get_transfer_status(int status)
{
    KyDebug() << status ;
    if(status == Status::Complete){
        if (selectedFiles.size() > 1) {
            selectedFiles.pop_front();
            file_size.pop_front();
            file_icon.pop_front();
            QFontMetrics fontMetrics(target_size->font());
            QString _filename = getFileName(selectedFiles.at(0));
            QString _text;
            if (selectedFiles.size() == 1) {
                _text = fontMetrics.elidedText(_filename, Qt::ElideMiddle, 280);
                target_name->setText(_text);
            }
            else if(selectedFiles.size() > 1)
            {
                _filename = "\"" + _filename + "\"";
                _text = fontMetrics.elidedText(_filename, Qt::ElideMiddle, 280);
                target_name->setText(_text.append(tr(" and ")+QString::number(fileNums)+tr(" files more")));
            }
            else
            {
                KyDebug();
                tranfer_error();
                this->show();
                this->activateWindow();
                this->send_state = _SEND_FAILURE ;
            }
            if (_text != _filename)
                target_name->setToolTip(_filename);

            target_size->setText(file_size.at(0));
            target_icon->setPixmap(file_icon.at(0).pixmap(64,64));
            int transfing = fileNums - selectedFiles.size() + 1;
            transNum->setText(QString::number(transfing)+"/"+QString::number(fileNums));
            this->show();
            this->activateWindow();
            return;
        }

        if(is_Intel) {
            tip_text->setVisible(false);
            m_progressbar->setVisible(false);
            transNum->setVisible(false);
            cancel_btn->setText(tr("OK"));
            tranfer_status_icon->setFixedSize(64,64);
            tranfer_status_icon->setPixmap(QPixmap("/usr/share/lingmo-bluetooth/file-transfer-success.svg"));
            tranfer_status_text->setText(tr("File Transmission Succeed!"));
            tranfer_status_text->setFixedWidth(500);
            QFont ft;
            ft.setPixelSize(16);
            tranfer_status_text->setFont(ft);
            struct_pos pos;
            pos = Config::getWidgetPos(207,57,64,64, this->width());
            tranfer_status_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

            pos = Config::getWidgetPos(191,129,200,24, this->width());
            tranfer_status_text->setGeometry(pos.x, pos.y, pos.width, pos.high);

            tranfer_status_icon->setVisible(true);
            tranfer_status_text->setVisible(true);
            target_icon->setVisible(false);
            target_name->setVisible(false);
            target_size->setVisible(false);
            cancel_btn->setText(tr("Close"));
            return;
        }
        if (this->send_state != _SEND_COMPLETE || this->send_state != _SEND_FAILURE)
            emit stopSendFile(target_dev);
        m_progressbar->setValue(100);
        m_progressbar->repaint();
        QString notify_msg = QString(tr("Transferred '")
                                     + getFileName(selectedFiles.at(0))
                                     + tr("' and ")
                                     + QString::number(fileNums)
                                     + tr(" files in all to '")
                                     + selectedDev->text(0)
                                     + tr("'"));
        Config::SendNotifyMessage(QString(tr("File Transfer Success!")),
                                  notify_msg,
                                  QString("complete"));
        selectedFiles.clear();
        file_size.clear();
        file_icon.clear();
        this->send_state = _SEND_COMPLETE ;
        this->close();
    }else if(status == Status::Active){
        this->send_state = _SENDING ;
    }else if(status == Status::Error){
        if (this->send_state == _SEND_FAILURE || send_state == _SEND_COMPLETE)
            return ;
        QDesktopWidget *deskdop = QApplication::desktop();
        move((deskdop->width()-this->width())/2, (deskdop->height()-this->height())/2);
        KyDebug() ;
        tranfer_error();
        this->send_state = _SEND_FAILURE ;
        this->show();
        this->activateWindow();
    } else if (status == Status::Static) {
        this->send_state = _SEND_NONE;
    }
}

void BluetoothFileTransferWidget::tranfer_error()
{
    if (this->send_state == _SEND_FAILURE || send_state == _SEND_COMPLETE)
        return ;
    if (is_Intel){
        tip_text->setVisible(false);
        m_progressbar->setVisible(false);
        transNum->setVisible(false);
        target_icon->setVisible(false);
        target_name->setVisible(false);
        target_size->setVisible(false);
        Tiptop->setVisible(false);
        qtw->setVisible(false);
        tranfer_status_icon->setFixedSize(64,64);
        tranfer_status_icon->setPixmap(QPixmap("/usr/share/lingmo-bluetooth/file-transfer-failed.svg"));
        tranfer_status_text->setFixedWidth(500);
        QFont ft;
        ft.setPixelSize(16);
        tranfer_status_text->setFont(ft);
        struct_pos pos;
        pos = Config::getWidgetPos(207,57,64,64, this->width());
        tranfer_status_icon->setGeometry(pos.x, pos.y, pos.width, pos.high);

        pos = Config::getWidgetPos(191,129,200,24, this->width());
        tranfer_status_text->setGeometry(pos.x, pos.y, pos.width, pos.high);

        cancel_btn->setText(tr("OK"));
        this->show();
        this->activateWindow();
    }

    KyDebug() << send_state;
    QString notify_msg = QString(tr("Transferring file '")
                                 + getFileName(selectedFiles.at(0))
                                 + tr("' to '")
                                 + selectedDev->text(0) + tr("'"));
    Config::SendNotifyMessage(QString(tr("File Transfer Fail!")),
                              notify_msg,
                              QString("dialog-error"));

    if (this->send_state == _SEND_NONE && !closed)
        return;
    if (target_dev.isEmpty())
        return;
    KyDebug() << send_state;
    emit stopSendFile(target_dev);
    this->close();
}

int BluetoothFileTransferWidget::get_send_data_state()
{
    return this->send_state;
}

void BluetoothFileTransferWidget::set_m_progressbar_value(int value)
{
    if (--active_flag > 0)
        return;
    m_progressbar->setValue(value);
    m_progressbar->repaint();
}

void BluetoothFileTransferWidget::GSettingsChanges(const QString &key)
{
    KyDebug() << key;
    if(key == "styleName"){
        if(GSettings->get("style-name").toString() == "lingmo-default" || GSettings->get("style-name").toString() == "lingmo-light"){
            target_icon->setProperty("setIconHighlightEffectDefaultColor", QColor(0,0,0));
            target_icon->setProperty("useIconHighlightEffect", 0x10);
        }else{
            target_icon->setProperty("setIconHighlightEffectDefaultColor", QColor(255,255,255));
            target_icon->setProperty("useIconHighlightEffect", 0x10);
        }
        this->update();
    }
    if (key == "systemFontSize") {
        double size = GSettings->get("system-font-size").toDouble() - 11;
        QFont ft;
        ft.setPixelSize(14+size);
        target_size->setFont(ft);
        target_name->setFont(ft);
        transNum->setFont(ft);
        Tiptop->setFont(ft);
        Tiptop->adjustSize();
        ft.setPixelSize(18+size);
        tip_text->setFont(ft);
        tranfer_status_text->setFont(ft);

        if (selectedDev == nullptr)
            return;
        QFontMetrics fontMetrics(tip_text->font());
        QString nName;
        nName = fontMetrics.elidedText(selectedDev->text(0), Qt::ElideMiddle, 280);
        QString fileSource = QString(tr("Transferring to \"")+nName+"\"");
        QString txt = fontMetrics.elidedText(fileSource, Qt::ElideMiddle, tip_text->width());
        tip_text->setText(txt);
        if (txt != fileSource)
            tip_text->setToolTip(selectedDev->text(0));
    }
}

void BluetoothFileTransferWidget::onClicked_OK_Btn()
{
    QString dev_addr = itemMap.value(selectedDev);
    KyDebug() << dev_addr;
    target_dev = dev_addr;
    emit this->sender_dev_name(dev_addr, sendFilesList);
    Initialize_and_start_animation();
    send_state = _SEND_ACTIVE;
    QDesktopWidget *deskdop = QApplication::desktop();
    move((deskdop->width()-this->width())/2, (deskdop->height()-this->height())/2);
}

QRect BluetoothFileTransferWidget::getButton() {
    if (pressTabCnt <= 0)
        return QRect();
    switch (pressTabCnt) {
    case 1:
        if (pressUDCnt == 0)
            pressUDCnt = 1;
        selectedBtn = nullptr;
        if (selectedDev != nullptr)
            selectedDev->setSelected(false);
        selectedItem = toolbutton_list.at(pressUDCnt - 1);
        selectedItem->setSelected(true);
        selectedDev = selectedItem;
        qtw->scrollToItem(selectedItem);
        if (!ok_btn->isEnabled())
            ok_btn->setEnabled(true);
        return QRect();
    case 2:
        selectedItem = nullptr;
        if (!ok_btn->isVisible() || !cancel_btn->isVisible())
            ++pressTabCnt;
        if(ok_btn->isVisible()) {
            selectedBtn = ok_btn;
            return ok_btn->geometry();
        } else if (cancel_btn->isVisible()){
            selectedBtn = cancel_btn;
            return cancel_btn->geometry();
        }
        return QRect();
        break;
    case 3:
        selectedItem = nullptr;
        selectedBtn = cancel_btn;
        return cancel_btn->geometry();
        break;
    case 4:
        selectedItem = nullptr;
//        selectedBtn = close_btn;
//        return close_btn->geometry();
        return QRect();
        break;
    default:
        break;
    }
    return QRect();
}

void BluetoothFileTransferWidget::paintEvent(QPaintEvent *event) {
    QPainter pt(this);
    QColor color = QColor("#818181");
//    color.setAlpha(tran);
    pt.setPen(color);
    QRect rect;
    rect = getButton();
    pt.setRenderHint(QPainter::Antialiasing);
    pt.drawRect(rect);
    QWidget::paintEvent(event);
}

void BluetoothFileTransferWidget::keyPressEvent(QKeyEvent *event) {
    int size = toolbutton_list.size();
    qDebug() << event->key();
    switch (event->key()) {
    case Qt::Key_Tab:
        ++pressTabCnt;
        if (pressTabCnt == 5)
            pressTabCnt = 1;
        if(send_state != _SEND_NONE && pressTabCnt == 1)
            ++pressTabCnt;
        KyDebug() << "press Tab" << pressTabCnt << send_state;
        this->update();
        break;
    case Qt::Key_Home:
    case Qt::Key_End:
        if (pressTabCnt != 1)
            return;
        pressUDCnt = (event->key() == Qt::Key_End) ?  size :  1;

        KyDebug() << "press Home/End" << pressTabCnt << pressUDCnt;

        this->update();
        break;
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
        if (pressTabCnt != 1)
            return;
        (event->key() == Qt::Key_PageDown) ? pressUDCnt += 6 : pressUDCnt -= 6;

        if (pressUDCnt > size)
            pressUDCnt = size;
        if (pressUDCnt < 1)
            pressUDCnt = 1;

        KyDebug() << "press page Down/Up" << pressTabCnt << pressUDCnt;

        this->update();
        break;
    case Qt::Key_Down:
    case Qt::Key_Up:
        if (pressTabCnt != 1)
            return;
        (event->key() == Qt::Key_Down) ? ++pressUDCnt : --pressUDCnt;

        if (pressUDCnt > size)
            pressUDCnt = 1;
        if (pressUDCnt < 1)
            pressUDCnt = size;

        KyDebug() << "press Down/Up" << pressTabCnt << pressUDCnt;

        this->update();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        KyDebug() << "press Enter";
        if (selectedBtn != nullptr && selectedBtn->isEnabled())
            selectedBtn->clicked();
        break;
    case Qt::Key_Escape:
        KyDebug() << "press Esc";
        if (pressTabCnt == 0) {
            this->close();
            if (!target_dev.isEmpty() && send_state == _SENDING)
                emit stopSendFile(target_dev);
        }
        qtw->clearSelection();
        qtw->clearFocus();
        ok_btn->setEnabled(false);
        pressTabCnt = 0;
        pressUDCnt = 0;
        selectedBtn = nullptr;
        if (selectedItem != nullptr)
            selectedItem->setSelected(false);
        selectedItem = nullptr;
        if (selectedDev != nullptr) {
            selectedDev->setSelected(false);
            if (GSettings->get("style-name").toString() != "lingmo-dark")
                selectedDev->setIcon(0, QIcon(Config::loadSvg(selectedDev->icon(0).pixmap(64,64), Config::PixmapColor::BLACK)));
            else
                selectedDev->setIcon(0, QIcon(Config::loadSvg(selectedDev->icon(0).pixmap(64,64), Config::PixmapColor::WHITE)));
        }
        this->update();
        break;
    default:
        break;
    }
}

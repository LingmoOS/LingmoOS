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

#ifndef BLUETOOTHFILETRANSFERWIDGET_H
#define BLUETOOTHFILETRANSFERWIDGET_H

#include <gio/gio.h>
#include <gio/gfile.h>
#include <gio/gfileinfo.h>
#include <gio/gioerror.h>
#include <KF5/BluezQt/bluezqt/device.h>

#include <QDesktopWidget>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QByteArray>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QScrollArea>
#include <QString>
#include <QFont>
#include <QDebug>
#include <QUrl>
#include <QDialog>
#include <QFileInfo>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QProgressBar>
#include <QGSettings>
#include <QFontMetrics>
#include <QPainter>
#include <QMessageBox>
#include <QAbstractAnimation>
#include <QTreeWidget>
#include <QKeyEvent>
#include "config/config.h"

#define TRANSPARENCY_SETTINGS       "org.lingmo.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"
#define QPROGRESSBAR_MAX_IMUM       1000

class BluetoothFileTransferWidget : public QWidget
{
    Q_OBJECT
public:

    explicit BluetoothFileTransferWidget(QStringList files, QMap<QString, QMap<QString, QVariant>> devicelist, QString dev_address = "");
    ~BluetoothFileTransferWidget();

    typedef enum _SEND_DATA_STATE
    {
        _SEND_NONE = 0,
        _SEND_ACTIVE,
        _SEND_COMPLETE,
        _SENDING,
        _SEND_FAILURE

    }SEND_DATA_STATE;

    static bool isShow;

    void Get_fie_type(int i);
    bool initFileInfo();
    void Get_file_size(float);
    void Initialize_and_start_animation();
    void tranfer_error();
    void insertNewFileList(QStringList fileList);

    int  get_send_data_state();

signals:
    void sender_dev_name(QString, QStringList);
    void stopSendFile(QString);
public slots:
    void onClicked_OK_Btn();
    void GSettingsChanges(const QString &key);
    void statusChangedSlot(QMap<QString, QVariant>);
    void powerChanged(bool powered);
    void devattrChanged(QString address, QMap<QString, QVariant> devAttr);

private:
    bool is_Intel = false;
    bool closed = false;
    bool hasemptyfile =false;
    double tran =1;
    int fileNums    = 0;
    int active_flag = 3;
    int pressTabCnt = 0;
    int pressUDCnt  = 0;

    QString target_dev;

    QGSettings *GSettings              = nullptr;
    QGSettings *transparency_gsettings = nullptr;

    QLabel *tip_text             = nullptr;
    QLabel *title_icon           = nullptr;
    QLabel *title_text           = nullptr;
    QLabel *target_icon          = nullptr;
    QLabel *target_name          = nullptr;
    QLabel *target_size          = nullptr;
    QLabel *Tiptop               = nullptr;
    QLabel *transNum             = nullptr;
    QLabel *tranfer_status_icon  = nullptr;
    QLabel *tranfer_status_text  = nullptr;

    QTreeWidget *qtw = nullptr;

    QParallelAnimationGroup *main_animation_group = nullptr;

    QProgressBar *m_progressbar = nullptr;

    QPushButton *close_btn   = nullptr;
    QPushButton *cancel_btn  = nullptr;
    QPushButton *ok_btn      = nullptr;
    QPushButton *selectedBtn = nullptr;

    QTreeWidgetItem *selectedItem = nullptr;

    QList<QIcon>   file_icon;
    QList<QString> file_size;
    QList<QString> filetype;
    QMap<QString, QString> oversizedFile;

    QStringList selectedFiles;
    QStringList sendFilesList;

    SEND_DATA_STATE send_state = _SEND_NONE;

    QTreeWidgetItem* selectedDev = nullptr;
    QList<QTreeWidgetItem*> toolbutton_list;
    QMap<QTreeWidgetItem*, QString> itemMap;
    void initLayout();
    void set_m_progressbar_value(int);
    void get_transfer_status(int);
    void initIntelLayout();
    QRect getButton();
    QString changeUnit(QString size);
    QString getFileName(QString filePath);
    void initQTreeWidget(QMap<QString, QMap<QString, QVariant> > devicelist, QString dev_address);

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // BLUETOOTHFILETRANSFERWIDGET_H

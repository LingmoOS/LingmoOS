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

#ifndef FILERECEIVINGPOPUPWIDGET_H
#define FILERECEIVINGPOPUPWIDGET_H
//extern "C" {
#include <gio/gio.h>
#include <gio/gfile.h>
#include <gio/gioerror.h>
#include <sys/stat.h>
#include <sys/types.h>
//}
#include <string>
#include <QStandardPaths>

#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QDialog>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QPalette>
#include <QString>
#include <QRect>
#include <QPoint>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QFile>
#include <QProcess>
#include <QGSettings>
#include <QHBoxLayout>
#include <QPainter>
#include <QKeyEvent>
#define TRANSPARENCY_SETTINGS       "org.lingmo.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"


class FileReceivingPopupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FileReceivingPopupWidget(QString address, QString devname = "", QString filename = "", QString type = "", quint64 size = 0);
    ~FileReceivingPopupWidget();
    void window_pop_up_animation();
public slots:
    void updateUI(QString name, quint64 size, QString type);
    void OnClickedAcceptBtn();
    void update_transfer_progress_bar(quint64);
    void file_transfer_completed(int status);
    void statusChangedSlot(QMap<QString, QVariant> status);
    void GSettings_value_chanage(const QString &key);
    void GSettingsChanges(const QString &key);
    void transferCompelete();
signals:
    void cancel();
    void rejected();
    void accepted();
private:
    bool isCanceled = false;
    bool isIntel = false;
    int fileStatus = 0xff;
    QGSettings   *StyleSettings = nullptr;
    QGSettings   *settings      = nullptr;
    QGSettings *transparency_gsettings = nullptr;

    QPushButton  *close_btn     = nullptr;
    QPushButton  *cancel_btn    = nullptr;
    QPushButton  *accept_btn    = nullptr;
    QPushButton  *view_btn      = nullptr;

    QLabel       *icon_label    = nullptr;
    QLabel       *file_source   = nullptr;
    QLabel       *window_Title  = nullptr;
    QLabel       *file_name     = nullptr;
    QLabel       *file_size     = nullptr;
    QLabel       *file_icon     = nullptr;
    QLabel       *warn_icon     = nullptr;
    QProgressBar *transfer_progress = nullptr;

    QRect   desktop;
    QString unitString[5] = {tr("Bytes"), "KB", "MB", "GB", "TB"};
    QString target_address;
    QString target_name;
    QString org_name;
    QString target_source;
    QString org_source;
    QString target_type;
    QString root_address;
    QString file_path;
    quint64 target_size;
    QTimer *receiveTimer;
    int fileNums = 0;
    QList<QString> receivedFiles;
    int pressCnt = 0;

    QPushButton *getButton();
    void showFiles();
    void initLayout();
    void initIntelLayout();
    QIcon getFileIconFromType(QString type);

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // FILERECEIVINGPOPUPWIDGET_H

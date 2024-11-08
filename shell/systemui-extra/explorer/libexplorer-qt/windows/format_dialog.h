/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ZJAY998 <zhangjun@kylinos.cn>
 *
 */

#ifndef FORMAT_DIALOG_H
#define FORMAT_DIALOG_H

#include <QTimer>
#include <errno.h>
#include <QDialog>
#include <QComboBox>
#include <QProgressBar>
#include <QCheckBox>
#include <QLabel>
#include <sys/stat.h>
#include <glib/gi18n.h>
#include <udisks/udisks.h>
#include <libnotify/notify.h>
#include <QProxyStyle>

#include "explorer-core_global.h"

//namespace Ui {
//class Format_Dialog;
//}

namespace Peony {
class SideBarAbstractItem;
}

class Format_Dialog;

using namespace Peony;


struct CreateformatData{
    UDisksObject *object,*drive_object;
    UDisksBlock *block,*drive_block;
    UDisksClient *client;
    const gchar *format_type;
    const gchar *device_name;
    const gchar *erase_type;
    const gchar *filesystem_name;
    int *format_finish;
    GVariantBuilder *builder;
    Format_Dialog *dl;
};


class PEONYCORESHARED_EXPORT Format_Dialog : public QDialog
{
    Q_OBJECT
    friend void volume_disconnect (GVolumeMonitor* vm, GDrive* v, gpointer data);
public:
    explicit Format_Dialog(const QString &uris,SideBarAbstractItem *m_item,QWidget *parent = nullptr);

    gboolean is_iso(const gchar *device_path);

    void ensure_unused_cb(CreateformatData *data);

    void ensure_format_cb (CreateformatData *data);

    void ensure_format_disk(CreateformatData *data);

    UDisksObject *get_object_from_block_device 	(UDisksClient *client,
                                                 const gchar *block_device);

    void kdisk_format(const gchar * device_name,const gchar *format_type,
                      const gchar * erase_type,const gchar * filesystem_name,int *format_finish);

    void cancel_format(const gchar* device_name);

    double get_format_bytes_done(const gchar * device_name);

    void format_ok_dialog();


    void format_err_dialog();

    bool format_makesure_dialog();
    
    ~Format_Dialog();

    void setBtnStatus(bool enable);
//    Ui::Format_Dialog *ui;

public:
    bool renameOK = true;

    QWidget *m_parent = nullptr;

    int m_cost_seconds = 0;

    int m_total_predict = 1200;

    double m_simulate_progress = 0;

protected:
    void closeEvent(QCloseEvent* );
    void resizeEvent(QResizeEvent *event);

Q_SIGNALS:
     void ensure_format(bool flags);

public Q_SLOTS:

    void acceptFormat (bool);
    void colseFormat(bool);
    void slot_format(bool enable);

    void formatloop();

private:
    static void volume_disconnect (GVolumeMonitor* vm, GDrive* v, gpointer data);
    static void format_cb (GObject *source_object, GAsyncResult *res ,gpointer user_data);

private:
    const int           mFixWidth = 424;
    const int           mFixHeight = 380;

    QTimer*             mTimer = nullptr;

    QComboBox*          mFSCombox = nullptr;
    QProgressBar*       mProgress = nullptr;
    QPushButton*        mCancelBtn = nullptr;
    QPushButton*        mFormatBtn = nullptr;
    QCheckBox*          mEraseCkbox = nullptr;
    QLineEdit*          mNameEdit = nullptr;
    QComboBox*          mRomSizeCombox = nullptr;

    QString             mVolumeName;
    GVolumeMonitor*     mVolumeMonitor = nullptr;


    QString fm_uris;
    SideBarAbstractItem *fm_item = nullptr;

};

class ButtonStyle : public QProxyStyle
{

public:
    static ButtonStyle *getStyle();

    ButtonStyle() : QProxyStyle() {}

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override;

    QRect subElementRect(SubElement element,
                         const QStyleOption *option,
                         const QWidget *widget = nullptr) const;
};

#endif // FORMAT_DIALOG_H

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

#ifndef TRAYWIDGET_H
#define TRAYWIDGET_H

#include <glib.h>
#include <gio/gio.h>
#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QLabel>
#include <QPalette>
#include <QStackedWidget>
#include <QPushButton>
#include <QToolButton>
#include <QScrollArea>
#include <QDesktopWidget>
#include <QGSettings>
#include <QScrollBar>
#include <QKeyEvent>
#include <QScreen>
#include <QStringList>
#include <QMessageBox>

#include "component/bluetoothsettinglabel.h"
#include "component/switchbutton.h"
#include "component/kyhline.h"
#include "component/qdevitem.h"
#include "config/config.h"
#include "config/xatom-helper.h"
#include "config/x11devices.h"
#include "dbus/bluetoothdbus.h"
#include "popwidget/trayicon.h"
#include "errormessagewidget.h"

#include "kswitchbutton.h"
using namespace kdk;

#define TRANSPARENCY_SETTINGS       "org.lingmo.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"

class TrayWidget : public QWidget
{
    Q_OBJECT
public:
    static TrayWidget * self(BluetoothDbus *dbus, bool intel = false);
    ~TrayWidget();

    void InitMemberVariables();
    void connectUISignalAndSlot();
    void setBluetoothOpen(bool v);
    void updatePairedDev();
    void layoutAddOnePairedDev(QMap<QString, QVariant> devAttr);

public slots:
    void GSettingsChanges(const QString &key);
    void showUI();
    void hideUI();
    void setTrayIconClickEnable(bool enable);

private slots:
    void openBluetoothSlot(bool open);
    void removePairedDevUIByAddress(QString address);
    void layoutAddAllPairedDev();
    void reBindMBtn(bool confirm);
    void manualOpenBt(bool open);

private:
    //键盘交互
    void kbUDEvent(bool opt);
    void kbTabEvent();
    void kbEnterEvent();
    void kbHomeAndEndEvent(bool opt);
    void kbPageEvent(bool opt);
    void autoChangePage(bool opt);
    //界面
    void initPanelGSettings();
    void InitNomalWidget();
    void InitErrorWidget();
    void clearPairedWidget();
    void getTransparency();
    void addNewDevItem(QString addr);

    bool whetherNeedInfoUser();
    TrayWidget(bool intel, BluetoothDbus *dBus, QWidget *parent = nullptr);
    static TrayWidget* _MSelf;
    TrayIcon *trayicon;

    BluetoothDbus *_SessionDBus;
    Config _config;

    KSwitchButton             *_MBtn = nullptr;
    QStackedWidget *_MStackedWidget = nullptr;
    QLabel                  *_MLoad = nullptr;
    QLabel                 *_MLabel = nullptr;
    QVBoxLayout       *_MMainLayout = nullptr;
    BluetoothSettingLabel       *_MSettingBtn = nullptr;
    QGSettings       *StyleSettings = nullptr;
    QGSettings *transparency_gsettings = nullptr;
    QGSettings *m_panelGSettings = nullptr;

    QWidget           *_NomalWidget = nullptr;
    QWidget           *_ErrorWidget = nullptr;
    QWidget          *_PairedWidget = nullptr;
    ErrorMessageWidget *errMsgDialog = nullptr;

    QVBoxLayout      *_PairedLayout = nullptr;
    QScrollArea        *_ScrollArea = nullptr;
    KyHLine               *lineFrame = nullptr;
    KyHLine               *lineFrame2 = nullptr;
    QMap<QString, QMap<QString, QVariant>> pairedDevices;
    int iconFlag = 0;
    int manualMove = 1;
    double tran =1;
    bool _userAction = true;
    bool manualSwitch = false;
    bool            _Intel;
    bool            MBtnFocused = false;
    int             selfHeight = 0;
    int pressCnt = 0;
    int scrollCnt = 0;
    int listCnt = 0;
    QDevItem *selectedDev = nullptr;
    int scrollTo = 0;
    int m_panelPosition = 0;
    int m_panelSize = 46;

signals:
    void trayIconStatusChanged(bool);
    void setManagerUnblocked(bool);

protected:
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *o, QEvent *e);
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);

    void moveEvent(QMoveEvent *event);
};

#endif // TRAYWIDGET_H

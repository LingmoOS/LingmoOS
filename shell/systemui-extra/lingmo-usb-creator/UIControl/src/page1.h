/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef PAGE1_H
#define PAGE1_H

#define NOUDISK "noUDisk"
#define UDISK_NAME_MAX_LENGTH 27//U盘名称最大长度
#define COMBOBOXW 370 //下拉框宽度
#define COMBOBOXH 30//下拉框高度
#define ITEMHEIGHT 30//下拉框标签高度
#define COMBOBOXRADIUS 6//下拉框窗口圆角
#define DARKTHEME "dark"
#define LIGHTTHEME "light"
#define LINGMO_STYLE "org.lingmo.style"
#include <QEvent>
#include <QDebug>
#include <QRect>
#include <QDropEvent>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QComboBox>
#include <QWidget>
#include <QLabel>
#include <QProcess>
#include <QComboBox>
#include <QMessageBox>
#include <QLineEdit>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QPushButton>
#include <QTimer>
#include <QCheckBox>
#include <QBoxLayout> //布局
#include <QFileDialog>//打开文件
#include <QStorageInfo>//硬盘信息
#include <QFileSystemWatcher>
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QCoreApplication>
#include <QTextCodec>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QApplication>
#include <QTableWidget>
#include <QPainter>
#include "topborder.h"
#include "line.h"
#include "include/GSettingSubject.h"
#include<QFrame>

class AvailableDiskInfo
{
public:
    AvailableDiskInfo(QString path = "unknowpath",QString name = "unknowmodel",QString capacity = "unknowcapacity"):devicePath(path),displayName(name),diskCapicity(capacity){}
public:
    QString devicePath;   //设备路径ex:/dev/sdb
    QString displayName;
    QString diskCapicity;
//    TODO:分区数获取，按照分区全部卸载。（目前只卸载了第一个分区然后就写入
    qint8 partsNums = 0;    //为0时视作没有分区，直接向块设备写入
};

class Page1 : public QWidget
{
    Q_OBJECT
public:
    explicit Page1();
    QString isoPath;    //iso文件的路径
     QFrame *fram;
    QList<AvailableDiskInfo*> diskInfos; // U盘信息
    bool ifStartBtnChange(); //开始制作按钮是否可以点击
    void setThemeStyleLight(); //设置浅色主题
    void setThemeStyleDark(); //设置深色主题
    void settingStyle();
    void dealSelectedFile(QString);
    QString getDevPath();
    void dealSetFontSize(int size);
    void fontSizeInit();
    QLabel *urlIso = nullptr;//显示镜像路径
    QLabel *urlIso1 = nullptr;
    QComboBox *comboUdisk = nullptr;//U盘列表
    QStringList m_udiskList; // U盘名称列表，同comboUdisk索引对应
    QString m_curUdiskName;  // 当前选中的U盘名称
    bool m_canChange;
    topborder * top;
   // QLineEdit * edit;
    line * edit;
    QString usb1;
    QString usb2;
    QPushButton *creatStart = nullptr;//开始制作
    QPushButton *findIso = nullptr;//浏览文件按钮
    GsettingSubject *subject;
    QLabel *tabUdisk = nullptr;//选择U盘标签
//    QLabel *tabUdisk1= nullptr;
    QPushButton *tabUdisk2= nullptr;
    
     QWidget *topwidget;
     QWidget * topwidget1;
    void dropEvent(QDropEvent*);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
signals:
    void makeStart(QString sourcePath,QString targetPath); //make start
    void starmake(QString sourcepath,QString usbpath);
    void diskLabelRefresh(QString); //  已选中U盘刷新
    void isoIllegal();
    void setIsoPath(const QString& iso_path);
public slots:
    //void refreshDiskList(QString path);
    void dealAuthDialogClose();  //处理授权框关闭
    void dealComboBoxChangeButton(); //combobox通知page1检查开始按钮是否可以亮起
    void dealIsoPath(const QString& iso_path);

private:
    void creatStartSlots();    //开始制作
    void initControlQss();//初始化控件及其样式
    void getStorageInfo();//获取磁盘信息
    void dealDialogCancel();     // 处理授权框关闭及取消
    void udiskPlugWatcherInit(); //U盘插拔监控初始化
    bool isCapicityAvailable(QString); //容量过滤
    void getUdiskPathAndCap();    //获取U盘路径和容量
    void getUdiskName();    //获取U盘第一个分区的命名
    bool checkISO(const QString fileName); //ISO合法性检验
    static void wait(uint sec);//获取挂载点延迟
    QJsonArray QStringToJsonArray(const QString jsonString);

    QString mountpoint;    //挂载点
    float USize;          //U盘大小
    float isoSize;        //iso大小
    QString m_userNamePath;   //当前用户名路径
    QStringList m_userNamelist; //当前用户名列表
    QString m_userName; //当前用户名
    QString themeStatus = LIGHTTHEME; //主题指示器
    QTimer *diskRefreshDelay = nullptr; //U盘插入后等待系统挂载的时间
    QLabel *tabIso = nullptr;//选择镜像标签
    QLabel *warnningText = nullptr;//警告标语tabUdisk
    QFileSystemWatcher *udiskplugwatcher = nullptr; //U盘插拔监控器
    QPushButton *dialogNo = nullptr;
    QPushButton *dialogYes = nullptr;

    bool paintOnce=false;//只绘制一次
    bool leaveThis=true;
protected:
     bool eventFilter(QObject *watched, QEvent *event);

};


#endif // PAGE1_H

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
#ifndef PAGE2_H
#define PAGE2_H

#define DARKTHEME "dark"
#define LIGHTTHEME "light"

#include <QWidget>
#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QMovie>
#include <QProcess>
#include <QBoxLayout>
#include <QString>
#include <QFileInfo>
#include <QStorageInfo>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>
#include <QDBusConnection>
#include <QLabel>
#include "include/menumodule.h"
#include "page1.h"
#include "QRoundProgressBar.h"
#include "kprogresscircle.h"

using namespace kdk;

class Page2 : public QWidget
{
    Q_OBJECT

public:
    explicit Page2(QWidget *parent = nullptr);

    QString uDiskPath = ""; //U盘路径
    QString usbpath;   //点击开始后的U盘路径
    QString isopath;    //点击开始的镜像文件路径
    Page1 * page1;       //选择界面
    QTimer *diskRefreshDelay;   //u盘定时刷新
//    QRoundProgressBar *round;      //计时界面
    KProgressCircle *round1;

    QLabel *m_progress;            //进度条
    QPushButton *returnPushButton=nullptr; // 回page1的按钮
    QPushButton *cancelbtn=nullptr;       //取消按钮
    menuModule *menu;       //菜单栏
    QString mountpoint;      //挂载点
    qint64 getFileSize(QString filePath); //获取文件大小，返回单位mb
    void setThemeStyleDark();
    void setThemeStyleLight();
    QJsonArray QStringToJsonArray(const QString jsonString);
    void playLoadingGif();

signals:
    void makeStart(QString sourcePath,QString targetPath); //make start
    void diskLabelRefresh(); //  已选中U盘刷新
    void makeFinish();
    void returnMain();
    void mainclose();

public slots:
    void makestar(QString path,QString sourcepath);
    void startMaking();
    void finishEvent();
    void allclose();
    void setIconchange();
    void dealWorkingProgress(int);
    void dealMakeFinish(QString);

private:
    enum typeMovieStatus {
        loading = 0,
        finish = 1,
        failed = 2
    } movieStatus;   //枚举类型的动画进度


    QString themeStatus = LIGHTTHEME;
    QMovie *movieLoading_l=nullptr; //过程动画，黑白两套
    QMovie *movieFinish_l=nullptr;
    QMovie *movieLoading_d=nullptr;
    QMovie *movieFinish_d=nullptr;
    QPixmap errLabel;
    QLabel *lableMovie=nullptr;
    QLabel *lableText=nullptr;
    QLabel *lableNum=nullptr;
    QProcess *command_dd;
    QProcess *md5;
    bool ismakesuccess =false;
    int frameCount=0;
    qint64 sourceFileSize = 0;
    QTimer *timer;
    QTimer *checkMd5FinishTimer = nullptr;
    bool m_isMd5Finish=false;
    QString m_isFinishstatus;
    QString MD5str;
    QString mountstr;
    QString m_userName;

    void playFinishGif();
    void playErrorGif();
    bool isMakingSuccess(); //进度到100%之后检查制作是否成功
    void movieRefresh(); //切换主题时重新加载动画
   // bool mountDevice(QString);
    void getmountpoint();
    void checkmd5();
    bool selectpath(QString path);

private slots:
    void readStandardOutput(int,QProcess::ExitStatus);
    void checkmd5finish();
};

#endif // PAGE2_H

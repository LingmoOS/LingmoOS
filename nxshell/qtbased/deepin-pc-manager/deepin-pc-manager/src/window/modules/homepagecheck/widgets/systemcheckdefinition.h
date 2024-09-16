// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMCHECKDEFINITION_H
#define SYSTEMCHECKDEFINITION_H

#include <QList>
#include <QObject>
#include <QString>

#define MB_COUNT (1 << 20)
#define GB_COUNT (1 << 30)

#define SYS_VER_CHECK_TIMEOUT 40 * 1000

#define FIXED_ROW(row, height)                        \
    {                                                 \
        mainLayout->setRowStretch(row, height);       \
        mainLayout->setRowMinimumHeight(row, height); \
    }

#define FIXED_COL(column, weight)                          \
    {                                                      \
        mainLayout->setColumnStretch(column, weight);      \
        mainLayout->setColumnMinimumWidth(column, weight); \
    }

#define FIXED_FONT_PIXEL_SIZE(widget, size)  \
    {                                        \
        QFont widget##Font = widget->font(); \
        widget##Font.setPixelSize(size);     \
        widget->setFont(widget##Font);       \
    }

#define CHECK_MAX_POINT 100
#define RICH_TEXT_COLOR_FORMAT "<font color=%1>%2</font>"

// 自动化标签
#define ACCNAMESTR(content) #content
#define SET_ACCESS_NAME_T(control, className, name) \
    control->setAccessibleName(ACCNAMESTR(className##_##name));

enum SysCheckItemID { SSH, Disk, Trash, SystemUpdate, DevMode, AutoStartApp, MaxLimit };

enum IgnoreStatus { WithNoIgnore, Ignored, IgnoreCanceled };

enum CheckProgressStatus { Success, TimeOut, Masked };

// 错误数据模型角色
enum CheckResultModelRole {
    ID = Qt::UserRole + 1, // 检查项ID
    CheckProgressFlag, // 检查项是完成还是超时还是被屏蔽（异常状态为系统版本检查超时，
                       // 病毒查杀冲突， SSH被隐藏）
    IsIssueHappen, // 检查项是否存在问题
    IsIgnored,     //  检查项是否被设置为忽略状态
    Details // 检查项问题详细数据(不要记录复杂数据，仅检查项UI需要显示的数据，如 int
            // bool等，具体数据放在本model中处理)
};

class SystemCheckDocument : public QObject
{
    Q_OBJECT
};

const QString k1stPageLogoPath = "dcc_syscheck_main_logo";
const QString kprogressingLogoPath = "dcc_syscheck_progressing";
const QString kAbnormalStatusIconPath = "dcc_syscheck_item_abnormal";
const QString kNormalStatusIconPath = "dcc_syscheck_item_normal";

const QString kAbnormalResultLogoPath = "dcc_syscheck_result_abnormal";
const QString kRepairingResultLogoPath = "dcc_syscheck_result_repair";
const QString k100PointResultLogoPath = "dcc_syscheck_result_100point";

const QString kIgnoreBtnIconPath = "dcc_syscheck_item_ignore";
const QString kDisIgnoreBtnIconPath = "dcc_syscheck_item_noignore";

const QString kProgressCancelIconPath = "dcc_syscheck_progress_cancel";

const QString k90_100Color = "#00C134";
const QString k75_90Color = "#8CD400";
const QString k60_75Color = "#FF5C00";
const QString k0_60Color = "#D80000";
const QString kOutOfDate = "#FF5C00";

#define kPointText SystemCheckDocument::tr("points")
#define kFixTip SystemCheckDocument::tr("Fixing")
#define kProcessingText SystemCheckDocument::tr("Checking")
// 首页上的彩色拼接字符串
#define kyouComputer SystemCheckDocument::tr("Your computer is ")
#define katRist SystemCheckDocument::tr("at risk")
#define katHighRist SystemCheckDocument::tr("at high risk")

#define k90_100GreetingStatus SystemCheckDocument::tr("Your computer is healthy")
#define k75_90GreetingStatus SystemCheckDocument::tr("Your computer is in good condition")
#define k60_75GreetingStatus SystemCheckDocument::tr("Your computer is at risk")
#define k0_60GreetingStatus SystemCheckDocument::tr("Your computer is at high risk")
#define k5DaysGreetingStatus \
    SystemCheckDocument::tr("Your computer has not been checked for a long time")
#define kOutOfDateGreetingStatus SystemCheckDocument::tr("Your computer has not been checked yet")

#define k90_100GreetingInfo SystemCheckDocument::tr("Great! Keep staying healthy please.")
#define k75_90GreetingInfo SystemCheckDocument::tr("Start a check and perform some improvements.")
#define k60_75GreetingInfo SystemCheckDocument::tr("Please check your system now.")
#define k0_60GreetingInfo \
    SystemCheckDocument::tr("It is strongly recommended to check and fix your system now.")
#define k5DaysGreetingInfo SystemCheckDocument::tr("Have a new check right now.")
#define kOutOfDateGreetingInfo SystemCheckDocument::tr("Try a full check.")

#define kAbnormalResultTitle SystemCheckDocument::tr("%1 issue(s) found")
#define kAbnormalResultTip \
    SystemCheckDocument::tr("Fix issues right now to keep your computer healthy.")
#define kNormalResultTitle SystemCheckDocument::tr("Your computer is healthy")
#define kNormalResultTip SystemCheckDocument::tr("Great! Keep staying healthy please.")

#define kFixingTitle SystemCheckDocument::tr("Fixing issues")
#define kFixingTip SystemCheckDocument::tr("It will be completed soon.")
#define kFIxingText SystemCheckDocument::tr("fixing")

// 以下列表，元素顺序均与6.0需求文档与UI对应
// 注意 SSH检查略过，能窗口中略过检查和隐藏，不要在这里删除代码
// configuration start
const QList<QString> kSystemCheckTypeIconList = {
    "dcc_syscheck_item_antivirus", "dcc_syscheck_item_avupdate", "dcc_syscheck_item_ssh",
    "dcc_syscheck_item_disk",      "dcc_syscheck_item_garbage",  "dcc_syscheck_item_sysupdate",
    "dcc_syscheck_item_dev",       "dcc_syscheck_item_autostart"
};

#define kTypename1 SystemCheckDocument::tr("Viruses and Trojans")
#define kTypename2 SystemCheckDocument::tr("Database version")
#define kTypename3 SystemCheckDocument::tr("Remote access (SSH)")
#define kTypename4 SystemCheckDocument::tr("Disk checking")
#define kTypename5 SystemCheckDocument::tr("System cleanup")
#define kTypename6 SystemCheckDocument::tr("System version")
#define kTypename7 SystemCheckDocument::tr("Developer mode")
#define kTypename8 SystemCheckDocument::tr("Startup programs")

#define kStatusNormal1 SystemCheckDocument::tr("No threats found")
#define kStatusNormal2 SystemCheckDocument::tr("Up to date")
#define kStatusNormal3 SystemCheckDocument::tr("Closed")
#define kStatusNormal4 SystemCheckDocument::tr("No errors found")
#define kStatusNormal5 SystemCheckDocument::tr("Clean")
#define kStatusNormal6 SystemCheckDocument::tr("Up to date")
#define kStatusNormal7 SystemCheckDocument::tr("No root access")
#define kStatusNormal8 SystemCheckDocument::tr("%1 apps")

#define kStatusAbnormal1 SystemCheckDocument::tr("Found threats")
#define kStatusAbnormal2 SystemCheckDocument::tr("Out of date")
#define kStatusAbnormal3 SystemCheckDocument::tr("Open")
#define kStatusAbnormal4 SystemCheckDocument::tr("Found errors")
#define kStatusAbnormal5 SystemCheckDocument::tr("There are junk files")
#define kStatusAbnormal6 SystemCheckDocument::tr("Out of date")
#define kStatusAbnormal7 SystemCheckDocument::tr("Root access allowed, which is risky")
#define kStatusAbnormal8 SystemCheckDocument::tr("%1 apps")

#define kJumpLabel1 SystemCheckDocument::tr("Fix now")
#define kJumpLabel2 SystemCheckDocument::tr("Update now")
#define kJumpLabel3 SystemCheckDocument::tr("Close it")
#define kJumpLabel4 SystemCheckDocument::tr("Go to check")
#define kJumpLabel5 SystemCheckDocument::tr("Clean up")
#define kJumpLabel6 SystemCheckDocument::tr("Check for updates")
#define kJumpLabel7 ""
#define kJumpLabel8 SystemCheckDocument::tr("Check up")

#define kStageinfo1 SystemCheckDocument::tr("Scanning viruses and Trojans")
#define kStageinfo2 SystemCheckDocument::tr("Checking the version of virus database")
#define kStageinfo3 SystemCheckDocument::tr("Checking the state of remote access (SSH)")
#define kStageinfo4 SystemCheckDocument::tr("Checking disks")
#define kStageinfo5 SystemCheckDocument::tr("Checking system junk")
#define kStageinfo6 SystemCheckDocument::tr("Checking if your system is up to date")
#define kStageinfo7 SystemCheckDocument::tr("Checking if the developer mode is enabled")
#define kStageinfo8 SystemCheckDocument::tr("Checking startup programs")

// 检查项异常扣分规则
const QList<int> kIssuePointInfoList{ 30,  // av
                                      10,  // av version
                                      5,   // ssh
                                      15,  // disk
                                      5,   // trash
                                      5,   // system update
                                      5,   // dev mode
                                      5 }; // autostart
// 垃圾清理总量大于1G时，额外扣分
const int kTrashSizeTooLargeAdjustPoint = 5;

#define kResultWidLeftButtonRecheck SystemCheckDocument::tr("Check Again")
#define kResultWidRightButtonChecked SystemCheckDocument::tr("Fix All")
#define kResultWidRightButtonFixed SystemCheckDocument::tr("Done", "button")

#define kIgnoreButtonToolTip SystemCheckDocument::tr("Dismiss")
#define kDisIgnoreButtonToolTip SystemCheckDocument::tr("No dismission")

// 日志相关
#define kAutoStartDismissed SystemCheckDocument::tr("Startup programs checking dismissed")
#define kAutoStartResumed SystemCheckDocument::tr("Startup programs checking resumed")
#define kDevModeDismissed SystemCheckDocument::tr("Developer mode checking dismissed")
#define kAVFixed SystemCheckDocument::tr("Viruses and Trojans processed")
#define kAVVersionFixed SystemCheckDocument::tr("Virus database updated")
#define kTrashFixed SystemCheckDocument::tr("Junk files cleaned up")
#define KDiskChecked SystemCheckDocument::tr("Disk errors checked")
#define kErrorInfomation SystemCheckDocument::tr("%1 issues found in %2")

// configuration end

class SystemCheckHelper
{
public:
    static QString getColoredText(int point, const QString &input)
    {
        if (point >= 90) {
            return QString(RICH_TEXT_COLOR_FORMAT).arg(k90_100Color).arg(input);
        }

        if (point >= 75) {
            return QString(RICH_TEXT_COLOR_FORMAT).arg(k75_90Color).arg(input);
        }

        if (point >= 60) {
            return QString(RICH_TEXT_COLOR_FORMAT).arg(k60_75Color).arg(input);
        }

        if (point >= 0) {
            return QString(RICH_TEXT_COLOR_FORMAT).arg(k0_60Color).arg(input);
        }

        return QString(RICH_TEXT_COLOR_FORMAT).arg(kOutOfDate).arg(input);
    }
};

#endif // SYSTEMCHECKDEFINITION_H

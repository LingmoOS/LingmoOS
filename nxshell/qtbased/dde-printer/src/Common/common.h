// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QLoggingCategory>
#include <QDebug>
#include <cups/adminutil.h>

#define APPNAME "dde-printer"

Q_DECLARE_LOGGING_CATEGORY(COMMONMOUDLE)

typedef enum time_record_e {
    ADD_TIME = 0,
    START_TIME,
    FINISH_TIME,
    RESET_TIME,
    MAX_TIME
} time_record_t;
//将输入字符串转为小写，分隔出字母和数字
QString normalize(const QString &strin);

//解析1284 IE
QMap<QString, QString> parseDeviceID(const QString &strId);

//替换厂商名字，统一不同写法
//如果传入len，len返回被替换名字的长度
QString replaceMakeName(QString &make_and_model, int *len);

//移除model中的make信息
void removeMakeInModel(const QString &strMake, QString &strModel);

//解析make-and-model字段中的厂商和型号信息
void ppdMakeModelSplit(const QString &strMakeAndModel, QString &strMake, QString &strModel);

int shellCmd(const QString &cmd, QString &out, QString &strErr, int timeout = 30000);

//获取ipp请求的值
QVariant ipp_attribute_value(ipp_attribute_t *attr, int i);

QString getHostFromUri(const QString &strUri);

//通过空格分割字符串，在""内的空格不算
QStringList splitStdoutString(const QString &str);

//获取打印机PPD文件路径
QString getPrinterPPD(const char *name);

//获取打印机uri
QString getPrinterUri(const char *name);

bool isPackageExists(const QString &package);

QString reslovedHost(const QString &strHost);

QString getPrinterNameFromUri(const QString &uri);

QString toNormalName(const QString &name);

bool isIpv4Address(const QString &str);

void loadEventlib();

bool isEventSdkInit();

QStringList getCurrentTime(time_record_t type);

typedef void (*pfWriteEventLog)(const std::string &);
pfWriteEventLog getWriteEventLog();
void unloadEventLib();

int managerCanonPrinter(const QString &action, const QStringList &args);

bool isPackageInstalled(const QString &packageName);
#endif // COMMON_H

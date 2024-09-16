// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDESTINATION_H
#define DDESTINATION_H
#include "cupsconnection.h"

#include <QStringList>
#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(UTIL)

using namespace std;

enum DESTTYPE {
    PRINTER,
    CLASS
};

class DDestination
{

public:
    DDestination();
    virtual ~DDestination() {}

public:
    QString getName();
    void setName(const QString &strName);

    bool isShared();
    void setShared(bool shared);

    bool isEnabled();
    void setEnabled(bool enabled);

    //BaseInfo
    QStringList getPrinterBaseInfo();
    DESTTYPE getType() const;

    QString printerInfo();
    void setPrinterInfo(const QString &info);

    QString printerLocation();
    void setPrinterLocation(const QString &location);


    //Due to ppd file may been modify out the program,it need to call it again rather than save the status of it.
    virtual bool initPrinterPPD();
    virtual bool isPpdFileBroken();

    /**
    * @projectName   Printer
    * @brief         复制信息用于创建新的打印机
    * @author        liurui
    * @date          2019-11-07
    */
    void getPrinterInfo(QString &info, QString &location, QString &deviceURI, QString &ppdfile) const;

private:
    // 初始化打印机的属性，避免后续重复查询
    void initPrinterAttr();

protected:
    DESTTYPE m_type;
    QString m_strName;

    bool m_isShared;
    bool m_isEnabled;
    QString m_printerLocation;
    QString m_printerStatus;
    QString m_printerInfo; //打印机描述
    QString m_printerModel;
    QString m_printerURI;
    QString m_ppdFile;
};

#endif // DDESTINATION_H
